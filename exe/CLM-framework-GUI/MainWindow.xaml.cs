using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;

using OpenCVWrappers;
using CLM_Interop;
using CLM_Interop.CLMTracker;
using FaceAnalyser_Interop;
using Camera_Interop;
using System.Windows.Threading;
using System.IO;

namespace CLM_framework_GUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        // Timing for measuring FPS
        #region High-Resolution Timing
        static DateTime startTime;
        static Stopwatch sw = new Stopwatch();

        static MainWindow()
        {
            startTime = DateTime.Now;
            sw.Start();
        }

        public static DateTime CurrentTime
        {
            get { return startTime + sw.Elapsed; }
        }
        #endregion

        Thread processing_thread;

        // Some members for displaying the results
        private Capture capture;
        private WriteableBitmap latest_img;
        private WriteableBitmap latest_aligned_face;
        private WriteableBitmap latest_HOG_descriptor;

        // Managing the running of the analysis system
        private volatile bool thread_running;
        private volatile bool thread_paused = false;
        // Allows for going forward in time step by step
        // Useful for visualising things
        private volatile int skip_frames = 0;

        FpsTracker processing_fps = new FpsTracker();

        volatile bool detectionSucceeding = false;

        volatile bool reset = false;

        // For tracking
        CLMParameters clm_params;
        CLM clm_model;
        FaceAnalyserManaged face_analyser;

        // For selecting webcams
        CameraSelection cam_sec;
        bool using_webcam;

        // Recording parameters (default values)
        bool record_aus = false; // Recording Action Units
        bool record_pose = false; // head location and orientation
        bool record_params = false; // rigid and non-rigid shape parameters
        bool record_2D_landmarks = false; // 2D landmark location
        bool record_3D_landmarks = false; // 3D landmark locations in world coordinates
        bool record_HOG = false; // HOG features extracted from face images
        bool record_gaze = false; // Gaze recording
        bool record_aligned = false; // aligned face images
        bool record_tracked_vid = false;

        // Visualisation options
        bool show_tracked_video = true;
        bool show_appearance = true;
        bool show_geometry = true;
        bool show_aus = true;
        
        // TODO classifiers converted to regressors

        // TODO indication that track is done        

        // The recording managers
        StreamWriter output_head_pose_file;
        StreamWriter output_clm_params_file;
        StreamWriter output_2D_landmarks_file;
        StreamWriter output_3D_landmarks_file;
        StreamWriter output_au_class;
        StreamWriter output_au_reg;
        StreamWriter output_gaze;

        // Where the recording is done (by default in a record directory, from where the application executed)
        String record_root = "./record";

        // For AU visualisation and output        
        List<String> au_class_names;
        List<String> au_reg_names;

        // For AU prediction
        bool dynamic_AU_shift = true;
        bool dynamic_AU_scale = false;
        bool use_dynamic_models = true;

        private volatile bool mirror_image = false;

        public MainWindow()
        {
            InitializeComponent();

            // Set the icon
            Uri iconUri = new Uri("logo1.ico", UriKind.RelativeOrAbsolute);
            this.Icon = BitmapFrame.Create(iconUri);

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 2000), (Action)(() =>
            {
                RecordAUCheckBox.IsChecked = record_aus;
                RecordAlignedCheckBox.IsChecked = record_aligned;
                RecordTrackedVidCheckBox.IsChecked = record_tracked_vid;
                RecordHOGCheckBox.IsChecked = record_HOG;
                RecordGazeCheckBox.IsChecked = record_gaze;
                RecordLandmarks2DCheckBox.IsChecked = record_2D_landmarks;
                RecordLandmarks3DCheckBox.IsChecked = record_3D_landmarks;
                RecordParamsCheckBox.IsChecked = record_params;
                RecordPoseCheckBox.IsChecked = record_pose;

                UseDynamicModelsCheckBox.IsChecked = use_dynamic_models;
                UseDynamicScalingCheckBox.IsChecked = dynamic_AU_scale;
                UseDynamicShiftingCheckBox.IsChecked = dynamic_AU_shift;
            }));
            
            String root = AppDomain.CurrentDomain.BaseDirectory;

            clm_params = new CLMParameters(root);
            clm_model = new CLM(clm_params);
            face_analyser = new FaceAnalyserManaged(root, use_dynamic_models);

        }

        private bool ProcessFrame(CLM clm_model, CLMParameters clm_params, RawImage frame, RawImage grayscale_frame, double fx, double fy, double cx, double cy)
        {
            detectionSucceeding = clm_model.DetectLandmarksInVideo(grayscale_frame, clm_params);
            return detectionSucceeding;

        }

        private List<List<Tuple<double, double>>> ProcessImage(CLM clm_model, CLMParameters clm_params, RawImage frame, RawImage grayscale_frame)
        {
            List<List<Tuple<double,double>>> landmark_detections = clm_model.DetectMultiFaceLandmarksInImage(grayscale_frame, clm_params);
            return landmark_detections;

        }

        private void SetupRecording(String root, String filename, int width, int height)
        {
            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
            {
                RecordingMenu.IsEnabled = false;
                UseDynamicModelsCheckBox.IsEnabled = false;
            }));

            if (!System.IO.Directory.Exists(root))
            {
                System.IO.Directory.CreateDirectory(root);
            }

            if (record_pose)
            {
                String filename_poses = root + "/" + filename + ".pose";
                output_head_pose_file = new StreamWriter(filename_poses);
                output_head_pose_file.WriteLine("frame,success,confidence,pose_X(mm),pose_Y(mm),pose_Z(mm),pitch(rad),yaw(rad),roll(rad)");
            }

            if (record_params)
            {
                String filename_params = root + "/" + filename + ".params";
                output_clm_params_file = new StreamWriter(filename_params);

                output_clm_params_file.Write("frame,success,confidence,scale,rot_x,rot_y,rot_z,tx,ty");
                for (int i = 0; i < clm_model.GetNumModes(); ++i)
                {
                    output_clm_params_file.Write(",p" + i);
                }
                output_clm_params_file.WriteLine();
            }

            if (record_2D_landmarks)
            {
                String filename_2d_landmarks = root + "/" + filename + ".landmarks_2d";
                output_2D_landmarks_file = new StreamWriter(filename_2d_landmarks);

                output_2D_landmarks_file.Write("frame,success,confidence");
                for (int i = 0; i < clm_model.GetNumPoints(); ++i)
                {
                    output_2D_landmarks_file.Write(",x" + i);
                }
                for (int i = 0; i < clm_model.GetNumPoints(); ++i)
                {
                    output_2D_landmarks_file.Write(",y" + i);
                }
                output_2D_landmarks_file.WriteLine();
            }

            if (record_aus)
            {
                String filename_au_class = root + "/" + filename + ".au_class";
                output_au_class = new StreamWriter(filename_au_class);
                
                output_au_class.Write("frame,success,confidence");
                au_class_names = face_analyser.GetClassActionUnitsNames();
                au_class_names.Sort();
                foreach (var name in au_class_names)
                {
                    output_au_class.Write("," + name);
                }
                output_au_class.WriteLine();

                String filename_au_reg = root + "/" + filename + ".au_reg";
                output_au_reg = new StreamWriter(filename_au_reg);
                
                output_au_reg.Write("frame,success,confidence");
                au_reg_names = face_analyser.GetRegActionUnitsNames();
                au_reg_names.Sort();
                foreach (var name in au_reg_names)
                {
                    output_au_reg.Write("," + name);
                }
                output_au_reg.WriteLine();
            
            }

            if(record_gaze)
            {
                String filename_gaze = root + "/" + filename + ".gaze";
                output_gaze = new StreamWriter(filename_gaze);

                output_gaze.Write("frame, success, confidence, x_0, y_0, z_0, x_1, y_1, z_1, x_h0, y_h0, z_h0, x_h1, y_h1, z_h1");                
            }

            if (record_3D_landmarks)
            {
                String filename_3d_landmarks = root + "/" + filename + ".landmarks_3d";
                output_3D_landmarks_file = new StreamWriter(filename_3d_landmarks);

                output_3D_landmarks_file.Write("frame,success,confidence");
                for (int i = 0; i < clm_model.GetNumPoints(); ++i)
                {
                    output_3D_landmarks_file.Write(",X" + i);
                }
                for (int i = 0; i < clm_model.GetNumPoints(); ++i)
                {
                    output_3D_landmarks_file.Write(",Y" + i);
                }
                for (int i = 0; i < clm_model.GetNumPoints(); ++i)
                {
                    output_3D_landmarks_file.Write(",Z" + i);
                }
                output_3D_landmarks_file.WriteLine();
            }

            if (record_aligned)
            {
                String aligned_root = root + "/" + filename + "_aligned/";
                System.IO.Directory.CreateDirectory(aligned_root);
                face_analyser.SetupAlignedImageRecording(aligned_root);
            }

            if (record_tracked_vid)
            {
                String vid_loc = root + "/" + filename + ".avi";
                System.IO.Directory.CreateDirectory(root);
                face_analyser.SetupTrackingRecording(vid_loc, width, height, 30);
            }

            if (record_HOG)
            {
                String filename_HOG = root + "/" + filename + ".hog";
                face_analyser.SetupHOGRecording(filename_HOG);
            }

        }

        private void StopRecording()
        {
            if (record_pose && output_head_pose_file != null)
                output_head_pose_file.Close();

            if (record_params && output_clm_params_file != null)
                output_clm_params_file.Close();

            if (record_2D_landmarks && output_2D_landmarks_file != null)
                output_2D_landmarks_file.Close();

            if (record_3D_landmarks && output_3D_landmarks_file != null)
                output_3D_landmarks_file.Close();

            if (record_gaze && output_gaze != null)
                output_gaze.Close();             

            if (record_HOG)
                face_analyser.StopHOGRecording();

            if (record_tracked_vid)
                face_analyser.StopTrackingRecording();

            if(record_aus && output_au_class != null && output_au_reg != null)
            {
                output_au_class.Close();
                output_au_reg.Close();
            }

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
            {
                RecordingMenu.IsEnabled = true;
                UseDynamicModelsCheckBox.IsEnabled = true;

            }));

        }

        // Recording the relevant objects
        private void RecordFrame(CLM clm_model, bool success_b, int frame_ind, RawImage frame, RawImage grayscale_frame, double fx, double fy, double cx, double cy)
        {
            double confidence = (-clm_model.GetConfidence())/2.0 + 0.5;
            
            List<double> pose = new List<double>();
            clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy);

            int success = 0;
            if (success_b)
                success = 1;

            if (record_pose)
            {
                String pose_string = String.Format("{0},{1},{2:F3},{3:F3},{4:F3},{5:F3},{6:F3},{7:F3},{8:F3}", frame_ind, success, confidence, pose[0], pose[1], pose[2], pose[3], pose[4], pose[5]);
                output_head_pose_file.WriteLine(pose_string);
            }

            if (record_params)
            {
                output_clm_params_file.Write(String.Format("{0},{1},{2,0:F3}", frame_ind, success, confidence));
                
                List<double> all_params = clm_model.GetParams();

                for (int i = 0; i < all_params.Count; ++i)
                {
                    String param = String.Format("{0,0:F5}", all_params[i]);
                    output_clm_params_file.Write("," + param);
                }
                output_clm_params_file.WriteLine();
            }

            if (record_2D_landmarks)
            {
                List<Tuple<double,double>> landmarks_2d = clm_model.CalculateLandmarks();

                output_2D_landmarks_file.Write(String.Format("{0},{1},{2:F3}", frame_ind, success, confidence));

                for (int i = 0; i < landmarks_2d.Count; ++i)
                {
                    output_2D_landmarks_file.Write(",{0:F2}", landmarks_2d[i].Item1);
                }
                for (int i = 0; i < landmarks_2d.Count; ++i)
                {
                    output_2D_landmarks_file.Write(",{0:F2}", landmarks_2d[i].Item2);
                }
                output_2D_landmarks_file.WriteLine();
            }

            if (record_3D_landmarks)
            {
                List<System.Windows.Media.Media3D.Point3D> landmarks_3d = clm_model.Calculate3DLandmarks(fx, fy, cx, cy);

                output_3D_landmarks_file.Write(String.Format("{0},{1},{2:F3}", frame_ind, success, confidence));

                for (int i = 0; i < landmarks_3d.Count; ++i)
                {
                    output_3D_landmarks_file.Write(",{0:F2}", landmarks_3d[i].X);
                }
                for (int i = 0; i < landmarks_3d.Count; ++i)
                {
                    output_3D_landmarks_file.Write(",{0:F2}", landmarks_3d[i].Y);
                }
                for (int i = 0; i < landmarks_3d.Count; ++i)
                {
                    output_3D_landmarks_file.Write(",{0:F2}", landmarks_3d[i].Z);
                }
                output_3D_landmarks_file.WriteLine();
            }

            if (record_aus)
            {
                var au_classes = face_analyser.GetCurrentAUsClass();
                var au_regs = face_analyser.GetCurrentAUsReg();
                
                output_au_class.Write(String.Format("{0},{1},{2:F3}", frame_ind, success, confidence));

                foreach (var name_class in au_class_names)
                {
                    output_au_class.Write(",{0:F0}", au_classes[name_class]);
                }
                output_au_class.WriteLine();

                output_au_reg.Write(String.Format("{0},{1},{2:F3}", frame_ind, success, confidence));

                foreach (var name_reg in au_reg_names)
                {
                    output_au_reg.Write(",{0:F2}", au_regs[name_reg]);
                }
                output_au_reg.WriteLine();

            }

            if(record_gaze)
            {

                var gaze_cam = face_analyser.GetGazeCamera();
                var gaze_head = face_analyser.GetGazeHead();

                output_gaze.Write(String.Format("{0},{1},{2:F3}", frame_ind, success, confidence));

                output_gaze.Write(String.Format(",{0:F3},{1:F3},{2:F3},{3:F3},{4:F3},{5:F3}", gaze_cam.Item1.Item1, gaze_cam.Item1.Item2, gaze_cam.Item1.Item3,
                    gaze_cam.Item2.Item1, gaze_cam.Item2.Item2, gaze_cam.Item2.Item3));

                output_gaze.Write(String.Format(",{0:F3},{1:F3},{2:F3},{3:F3},{4:F3},{5:F3}", gaze_head.Item1.Item1, gaze_head.Item1.Item2, gaze_head.Item1.Item3,
                    gaze_head.Item2.Item1, gaze_head.Item2.Item2, gaze_head.Item2.Item3));

                output_gaze.WriteLine();

            }

            if (record_aligned)
            {
                face_analyser.RecordAlignedFrame(frame_ind);
            }

            if (record_HOG)
            {
                face_analyser.RecordHOGFrame();
            }

            if (record_tracked_vid)
            {
                face_analyser.RecordTrackedFace();
            }
        }

        // The main function call for processing images, video files or webcam feed
        private void ProcessingLoop(String[] filenames, int cam_id = -1, int width = -1, int height = -1, bool multi_face = false)
        {
            if (filenames == null)
                using_webcam = true;
            else
                using_webcam = false;

            thread_running = true;

            mirror_image = false;

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0,0,0,0,200), (Action)(() =>
            {
                ResetButton.IsEnabled = true;
                PauseButton.IsEnabled = true;
                StopButton.IsEnabled = true;
            }));

            // Create the video capture and call the VideoLoop
            if(filenames != null)
            {
                clm_params.optimiseForVideo();
                if (cam_id == -2)
                {
                    List<String> image_files_all = new List<string>();
                    foreach (string image_name in filenames)
                        image_files_all.Add(image_name);

                    // Loading an image sequence that represents a video                   
                    capture = new Capture(image_files_all);

                    if (capture.isOpened())
                    {
                        // Prepare recording if any based on the directory
                        String file_no_ext = System.IO.Path.GetDirectoryName(filenames[0]);
                        file_no_ext = System.IO.Path.GetFileName(file_no_ext);
                        SetupRecording(record_root, file_no_ext, capture.width, capture.height);

                        // Start the actual processing                        
                        VideoLoop();
                        
                        // Clear up the recording
                        StopRecording();

                    }
                    else
                    {
                        string messageBoxText = "Failed to open an image";
                        string caption = "Not valid file";
                        MessageBoxButton button = MessageBoxButton.OK;
                        MessageBoxImage icon = MessageBoxImage.Warning;

                        // Display message box
                        MessageBox.Show(messageBoxText, caption, button, icon);
                    }
                }
                else if (cam_id == -3)
                {
                    SetupImageMode();
                    clm_params.optimiseForImages();
                    // Loading an image file (or a number of them)
                    foreach (string filename in filenames)
                    {
                        if (!thread_running)
                        {
                            continue;
                        }

                        capture = new Capture(filename);

                        if (capture.isOpened())
                        {
                            // Start the actual processing                        
                            ProcessImage();

                        }
                        else
                        {
                            string messageBoxText = "File is not an image or the decoder is not supported.";
                            string caption = "Not valid file";
                            MessageBoxButton button = MessageBoxButton.OK;
                            MessageBoxImage icon = MessageBoxImage.Warning;

                            // Display message box
                            MessageBox.Show(messageBoxText, caption, button, icon);
                        }
                    }
                }
                else
                {
                    clm_params.optimiseForVideo();
                    // Loading a video file (or a number of them)
                    foreach (string filename in filenames)
                    {
                        if (!thread_running)
                        {
                            continue;
                        }

                        capture = new Capture(filename);

                        if (capture.isOpened())
                        {
                            // Prepare recording if any
                            String file_no_ext = System.IO.Path.GetFileNameWithoutExtension(filename);

                            SetupRecording(record_root, file_no_ext, capture.width, capture.height);

                            // Start the actual processing                        
                            VideoLoop();

                            // Clear up the recording
                            StopRecording();
                        }
                        else
                        {
                            string messageBoxText = "File is not a video or the codec is not supported.";
                            string caption = "Not valid file";
                            MessageBoxButton button = MessageBoxButton.OK;
                            MessageBoxImage icon = MessageBoxImage.Warning;

                            // Display message box
                            MessageBox.Show(messageBoxText, caption, button, icon);
                        }
                    }
                }
            }
            else
            {
                capture = new Capture(cam_id, width, height);
                mirror_image = true;

                if (capture.isOpened())
                {
                    // Prepare recording if any                    
                    String dir_out = DateTime.Now.ToString("yyyy-MMM-dd--HH-mm");

                    SetupRecording(record_root + "/" + dir_out, "webcam", width, height);

                    // Start the actual processing
                    VideoLoop();

                    StopRecording();
                }
                else
                {

                    string messageBoxText = "Failed to open a webcam";
                    string caption = "Webcam failure";
                    MessageBoxButton button = MessageBoxButton.OK;
                    MessageBoxImage icon = MessageBoxImage.Warning;

                    // Display message box
                    MessageBox.Show(messageBoxText, caption, button, icon);
                }
            }

            // TODO this should be up a level
            // Some GUI clean up
            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
            {
                Console.WriteLine("Cleaning up after processing is done");
                PauseButton.IsEnabled = false;
                StopButton.IsEnabled = false;
                ResetButton.IsEnabled = false;
                NextFiveFramesButton.IsEnabled = false;
                NextFrameButton.IsEnabled = false;
            }));

        }

        // Capturing and processing the video frame by frame
        private void ProcessImage()
        {
            Thread.CurrentThread.IsBackground = true;

            clm_model.Reset();
            face_analyser.Reset();


            //////////////////////////////////////////////
            // CAPTURE FRAME AND DETECT LANDMARKS FOLLOWED BY THE REQUIRED IMAGE PROCESSING
            //////////////////////////////////////////////
            RawImage frame = null;
            double progress = -1;

            frame = new RawImage(capture.GetNextFrame(mirror_image));
            progress = capture.GetProgress();

            if (frame.Width == 0)
            {
                // This indicates that we reached the end of the video file
                return;
            }
            
            var grayFrame = new RawImage(capture.GetCurrentFrameGray());

            if (grayFrame == null)
            {
                Console.WriteLine("Gray is empty");
                return;
            }

            List<List<Tuple<double, double>>> landmark_detections = ProcessImage(clm_model, clm_params, frame, grayFrame);

            List<Point> landmark_points = new List<Point>();

            for(int i = 0; i < landmark_detections.Count; ++i)
            {

                List<Tuple<double,double>> landmarks = landmark_detections[i];
                foreach (var p in landmarks)
                {
                    landmark_points.Add(new Point(p.Item1, p.Item2));
                }
            }

            // Visualisation
            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
            {
                if (show_tracked_video)
                {
                    if (latest_img == null)
                    {
                        latest_img = frame.CreateWriteableBitmap();
                    }

                    frame.UpdateWriteableBitmap(latest_img);

                    video.Source = latest_img;
                    video.Confidence = 1;
                    video.FPS = processing_fps.GetFPS();
                    video.Progress = progress;

                    video.OverlayLines = new List<Tuple<Point,Point>>();

                    video.OverlayPoints = landmark_points;
                }
               
            }));

            latest_img = null;
        }


        // Capturing and processing the video frame by frame
        private void VideoLoop()
        {
            Thread.CurrentThread.IsBackground = true;

            DateTime? startTime = CurrentTime;

            var lastFrameTime = CurrentTime;

            clm_model.Reset();
            face_analyser.Reset();

            // TODO add an ability to change these through a calibration procedure or setting menu
            double fx, fy, cx, cy;
            fx = 500.0;
            fy = 500.0;
            cx = cy = -1;
            
            int frame_id = 0;

            while (thread_running)
            {
                //////////////////////////////////////////////
                // CAPTURE FRAME AND DETECT LANDMARKS FOLLOWED BY THE REQUIRED IMAGE PROCESSING
                //////////////////////////////////////////////
                RawImage frame = null;
                double progress = -1;

                frame = new RawImage(capture.GetNextFrame(mirror_image));
                progress = capture.GetProgress();
                
                if(frame.Width == 0)
                {
                    // This indicates that we reached the end of the video file
                    break;
                }

                // TODO stop button actually clears the video
                
                lastFrameTime = CurrentTime;
                processing_fps.AddFrame();

                var grayFrame = new RawImage(capture.GetCurrentFrameGray());

                if (grayFrame == null)
                {
                    Console.WriteLine("Gray is empty");
                    continue;
                }

                // This is more ore less guess work, but seems to work well enough
                if (cx == -1)
                {
                    fx = fx * (grayFrame.Width / 640.0);
                    fy = fy * (grayFrame.Height / 480.0);

                    fx = (fx + fy) / 2.0;
                    fy = fx;

                    cx = grayFrame.Width / 2f;
                    cy = grayFrame.Height / 2f;
                }

                bool detectionSucceeding = ProcessFrame(clm_model, clm_params, frame, grayFrame, fx, fy, cx, cy);


                
                double confidence = (-clm_model.GetConfidence()) / 2.0 + 0.5;

                if (confidence < 0)
                    confidence = 0;
                else if (confidence > 1)
                    confidence = 1;

                List<double> pose = new List<double>();
                clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy);
                List<double> non_rigid_params = clm_model.GetNonRigidParams();

                // The face analysis step (only done if recording AUs, HOGs or video)
                if (record_aus || record_HOG || record_aligned || show_aus || show_appearance || record_tracked_vid || record_gaze)
                {
                    face_analyser.AddNextFrame(frame, clm_model, fx, fy, cx, cy, using_webcam, show_appearance, record_tracked_vid);
                }

                List<Tuple<Point, Point>> lines = null;
                List<Tuple<double, double>> landmarks = null;
                List<Tuple<Point, Point>> gaze_lines = null;

                if (detectionSucceeding)
                {
                    landmarks = clm_model.CalculateLandmarks();
                    lines = clm_model.CalculateBox((float)fx, (float)fy, (float)cx, (float)cy);
                    gaze_lines = face_analyser.CalculateGazeLines((float)fx, (float)fy, (float)cx, (float)cy);

                }

                // Visualisation
                Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0,0,0,0,200), (Action)(() =>
                {
                    if (show_aus)
                    {
                        var au_classes = face_analyser.GetCurrentAUsClass();
                        var au_regs = face_analyser.GetCurrentAUsReg();

                        auClassGraph.Update(au_classes);

                        var au_regs_scaled = new Dictionary<String, double>();
                        foreach (var au_reg in au_regs)
                        {
                            au_regs_scaled[au_reg.Key] = au_reg.Value / 5.0;
                            if (au_regs_scaled[au_reg.Key] < 0)
                                au_regs_scaled[au_reg.Key] = 0;
                            
                            if (au_regs_scaled[au_reg.Key] > 1)
                                au_regs_scaled[au_reg.Key] = 1;
                        }
                        auRegGraph.Update(au_regs_scaled);
                    }

                    if (show_geometry)
                    {
                        int yaw = (int)(pose[4] * 180 / Math.PI + 0.5);
                        int roll = (int)(pose[5] * 180 / Math.PI + 0.5);
                        int pitch = (int)(pose[3] * 180 / Math.PI + 0.5);

                        YawLabel.Content = yaw + "°";
                        RollLabel.Content = roll + "°";
                        PitchLabel.Content = pitch + "°";

                        XPoseLabel.Content = (int)pose[0] + " mm";
                        YPoseLabel.Content = (int)pose[1] + " mm";
                        ZPoseLabel.Content = (int)pose[2] + " mm";

                        nonRigidGraph.Update(non_rigid_params);

                        // Update eye gaze
                        var gaze_both = face_analyser.GetGazeCamera();
                        double x = (gaze_both.Item1.Item1 + gaze_both.Item2.Item1) / 2.0;
                        double y = (gaze_both.Item1.Item2 + gaze_both.Item2.Item2) / 2.0;

                        // Tweak it to a more presentable value
                        x = (int)(x * 35);
                        y = (int)(y * 70);

                        if (x < -10)
                            x = -10;
                        if (x > 10)
                            x = 10;
                        if (y < -10)
                            y = -10;
                        if (y > 10)
                            y = 10;

                        GazeXLabel.Content = x/10.0;
                        GazeYLabel.Content = y/10.0;


                    }

                    if (show_tracked_video)
                    {
                        if (latest_img == null)
                        {
                            latest_img = frame.CreateWriteableBitmap();
                        }

                        frame.UpdateWriteableBitmap(latest_img);

                        video.Source = latest_img;
                        video.Confidence = confidence;
                        video.FPS = processing_fps.GetFPS();
                        video.Progress = progress;

                        if (!detectionSucceeding)
                        {
                            video.OverlayLines.Clear();
                            video.OverlayPoints.Clear();
                            video.GazeLines.Clear();
                        }
                        else
                        {
                            video.OverlayLines = lines;

                            List<Point> landmark_points = new List<Point>();
                            foreach (var p in landmarks)
                            {
                                landmark_points.Add(new Point(p.Item1, p.Item2));
                            }

                            video.OverlayPoints = landmark_points;

                            video.GazeLines = gaze_lines;
                        }
                    }

                    if (show_appearance)
                    {
                        RawImage aligned_face = face_analyser.GetLatestAlignedFace();
                        RawImage hog_face = face_analyser.GetLatestHOGDescriptorVisualisation();

                        if (latest_aligned_face == null)
                        {
                            latest_aligned_face = aligned_face.CreateWriteableBitmap();
                            latest_HOG_descriptor = hog_face.CreateWriteableBitmap();
                        }

                        aligned_face.UpdateWriteableBitmap(latest_aligned_face);
                        hog_face.UpdateWriteableBitmap(latest_HOG_descriptor);

                        AlignedFace.Source = latest_aligned_face;
                        AlignedHOG.Source = latest_HOG_descriptor;
                    }
                }));

                // Recording the tracked model
                RecordFrame(clm_model, detectionSucceeding, frame_id, frame, grayFrame, fx, fy, cx, cy);

                if (reset)
                {
                    clm_model.Reset();
                    face_analyser.Reset();
                    reset = false;
                }

                while (thread_running & thread_paused && skip_frames == 0)
                {
                    Thread.Sleep(10);
                }

                frame_id++;

                if (skip_frames > 0)
                    skip_frames--;

            }

            latest_img = null;
            skip_frames = 0;

            // Unpause if it's paused
            if (thread_paused)
            {
                Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0,0,0,0,200), (Action)(() =>
                {
                    PauseButton_Click(null, null);
                }));
            }
        }

        private void StopTracking()
        {
            // First complete the running of the thread
            if (processing_thread != null)
            {
                // Tell the other thread to finish
                thread_running = false;
                processing_thread.Join();
            }
        }

        private void imageFileOpenClick(object sender, RoutedEventArgs e)
        {
            new Thread(() => imageOpen()).Start();
        }

        private void imageOpen()
        {
            StopTracking();

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 2, 0), (Action)(() =>
            {
                var d = new OpenFileDialog();
                d.Multiselect = true;
                d.Filter = "Image files|*.jpg;*.jpeg;*.bmp;*.png;*.gif";

                if (d.ShowDialog(this) == true)
                {

                    string[] image_files = d.FileNames;

                    processing_thread = new Thread(() => ProcessingLoop(image_files, -3));
                    processing_thread.Start();

                }
            }));
        }

        private void videoFileOpenClick(object sender, RoutedEventArgs e)
        {
            new Thread(() => openVideoFile()).Start();
        }

        private void openVideoFile()
        {
            StopTracking();

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 2, 0), (Action)(() =>
            {
                var d = new OpenFileDialog();
                d.Multiselect = true;
                d.Filter = "Video files|*.avi;*.wmv;*.mov;*.mpg;*.mpeg;*.mp4";

                if (d.ShowDialog(this) == true)
                {

                    string[] video_files = d.FileNames;

                    processing_thread = new Thread(() => ProcessingLoop(video_files));
                    processing_thread.Start();

                }
            }));
        }

        private void imageSequenceFileOpenClick(object sender, RoutedEventArgs e)
        {
            new Thread(() => imageSequenceOpen()).Start();
        }

        private void imageSequenceOpen()
        {
            StopTracking();

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 2, 0), (Action)(() =>
            {
                var d = new OpenFileDialog();
                d.Multiselect = true;
                d.Filter = "Image files|*.jpg;*.jpeg;*.bmp;*.png;*.gif";

                if (d.ShowDialog(this) == true)
                {

                    string[] image_files = d.FileNames;

                    processing_thread = new Thread(() => ProcessingLoop(image_files, -2));
                    processing_thread.Start();

                }
            }));
        }

        private void openWebcamClick(object sender, RoutedEventArgs e)
        {
            new Thread(() => openWebcam()).Start();
        }

        private void openWebcam()
        {
            StopTracking();

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 2, 0), (Action)(() =>
            {
                // First close the cameras that might be open to avoid clashing with webcam opening
                if (capture != null)
                {
                    capture.Dispose();
                }

                if (cam_sec == null)
                {
                    cam_sec = new CameraSelection();
                }
                else
                {
                    cam_sec = new CameraSelection(cam_sec.cams);
                    cam_sec.Visibility = System.Windows.Visibility.Visible;
                }

                // Set the icon
                Uri iconUri = new Uri("logo1.ico", UriKind.RelativeOrAbsolute);
                cam_sec.Icon = BitmapFrame.Create(iconUri);

                if (!cam_sec.no_cameras_found)
                    cam_sec.ShowDialog();

                if (cam_sec.camera_selected)
                {
                    int cam_id = cam_sec.selected_camera.Item1;
                    int width = cam_sec.selected_camera.Item2;
                    int height = cam_sec.selected_camera.Item3;

                    processing_thread = new Thread(() => ProcessingLoop(null, cam_id, width, height));
                    processing_thread.Start();

                }
            }));
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (processing_thread != null)
            {
                // Stop capture and tracking
                thread_running = false;
                processing_thread.Join();

                capture.Dispose();
            }
            face_analyser.Dispose();            
        }

        // Stopping the tracking
        private void StopButton_Click(object sender, RoutedEventArgs e)
        {
            if (processing_thread != null)
            {
                // Stop capture and tracking
                thread_paused = false;
                thread_running = false;
                processing_thread.Join();

                PauseButton.IsEnabled = false;
                NextFrameButton.IsEnabled = false;
                NextFiveFramesButton.IsEnabled = false;
                StopButton.IsEnabled = false;
                ResetButton.IsEnabled = false;
                RecordingMenu.IsEnabled = true;

                UseDynamicModelsCheckBox.IsEnabled = true;
            }
        }

        // Stopping the tracking
        private void ResetButton_Click(object sender, RoutedEventArgs e)
        {
            if (processing_thread != null)
            {
                // Stop capture and tracking
                reset = true;
            }
        }

        // Stopping the tracking
        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            if (processing_thread != null)
            {
                // Stop capture and tracking                
                thread_paused = !thread_paused;
                
                ResetButton.IsEnabled = !thread_paused;

                NextFrameButton.IsEnabled = thread_paused;
                NextFiveFramesButton.IsEnabled = thread_paused;

                if (thread_paused)
                {
                    PauseButton.Content = "Resume";
                }
                else
                {
                    PauseButton.Content = "Pause";
                }
            }
        }

        private void SkipButton_Click(object sender, RoutedEventArgs e)
        {
            if(sender.Equals(NextFrameButton))
            {
                skip_frames += 1;
            }
            else if(sender.Equals(NextFiveFramesButton))
            {
                skip_frames += 5;
            }
        }

        private void VisualisationCheckBox_Click(object sender, RoutedEventArgs e)
        {
            show_tracked_video = ShowVideoCheckBox.IsChecked;
            show_appearance = ShowAppearanceFeaturesCheckBox.IsChecked;
            show_geometry = ShowGeometryFeaturesCheckBox.IsChecked;
            show_aus = ShowAUsCheckBox.IsChecked;

            // Collapsing or restoring the windows here
            if (!show_tracked_video)
            {
                VideoBorder.Visibility = System.Windows.Visibility.Collapsed;
                MainGrid.ColumnDefinitions[0].Width = new GridLength(0, GridUnitType.Star);
            }
            else
            {
                VideoBorder.Visibility = System.Windows.Visibility.Visible;
                MainGrid.ColumnDefinitions[0].Width = new GridLength(2.1, GridUnitType.Star);
            }

            if (!show_appearance)
            {
                AppearanceBorder.Visibility = System.Windows.Visibility.Collapsed;
                MainGrid.ColumnDefinitions[1].Width = new GridLength(0, GridUnitType.Star);
            }
            else
            {
                AppearanceBorder.Visibility = System.Windows.Visibility.Visible;
                MainGrid.ColumnDefinitions[1].Width = new GridLength(0.8, GridUnitType.Star);
            }

            // Collapsing or restoring the windows here
            if (!show_geometry)
            {
                GeometryBorder.Visibility = System.Windows.Visibility.Collapsed;
                MainGrid.ColumnDefinitions[2].Width = new GridLength(0, GridUnitType.Star);
            }
            else
            {
                GeometryBorder.Visibility = System.Windows.Visibility.Visible;
                MainGrid.ColumnDefinitions[2].Width = new GridLength(1.0, GridUnitType.Star);
            }

            // Collapsing or restoring the windows here
            if (!show_aus)
            {
                ActionUnitBorder.Visibility = System.Windows.Visibility.Collapsed;
                MainGrid.ColumnDefinitions[3].Width = new GridLength(0, GridUnitType.Star);
            }
            else
            {
                ActionUnitBorder.Visibility = System.Windows.Visibility.Visible;
                MainGrid.ColumnDefinitions[3].Width = new GridLength(1.6, GridUnitType.Star);
            }
        
        }

        private void SetupImageMode()
        {
            // Turn off recording
            record_aus = false;
            record_aligned = false;
            record_HOG = false;
            record_gaze = false;
            record_tracked_vid = false;
            record_2D_landmarks = false;
            record_3D_landmarks = false;
            record_params = false;
            record_pose = false;

            // Turn off unneeded visualisations
            show_tracked_video = true;
            show_appearance = false;
            show_geometry = false;
            show_aus = false;

            // Actually update the GUI accordingly
            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 2000), (Action)(() =>
            {
                RecordAUCheckBox.IsChecked = record_aus;
                RecordAlignedCheckBox.IsChecked = record_aligned;
                RecordTrackedVidCheckBox.IsChecked = record_tracked_vid;
                RecordHOGCheckBox.IsChecked = record_HOG;
                RecordGazeCheckBox.IsChecked = record_gaze;
                RecordLandmarks2DCheckBox.IsChecked = record_2D_landmarks;
                RecordLandmarks3DCheckBox.IsChecked = record_3D_landmarks;
                RecordParamsCheckBox.IsChecked = record_params;
                RecordPoseCheckBox.IsChecked = record_pose;

                ShowVideoCheckBox.IsChecked = true;
                ShowAppearanceFeaturesCheckBox.IsChecked = false;
                ShowGeometryFeaturesCheckBox.IsChecked = false;
                ShowAUsCheckBox.IsChecked = false;

                VisualisationCheckBox_Click(null, null);
            }));

            // TODO change what next and back buttons do?
        }

        private void recordCheckBox_click(object sender, RoutedEventArgs e)
        {
            record_aus = RecordAUCheckBox.IsChecked;
            record_aligned = RecordAlignedCheckBox.IsChecked;
            record_HOG = RecordHOGCheckBox.IsChecked;
            record_gaze = RecordGazeCheckBox.IsChecked;
            record_tracked_vid = RecordTrackedVidCheckBox.IsChecked;
            record_2D_landmarks = RecordLandmarks2DCheckBox.IsChecked;
            record_3D_landmarks = RecordLandmarks3DCheckBox.IsChecked;
            record_params = RecordParamsCheckBox.IsChecked;
            record_pose = RecordPoseCheckBox.IsChecked;
        }

        private void UseDynamicModelsCheckBox_Click(object sender, RoutedEventArgs e)
        {
            dynamic_AU_shift = UseDynamicShiftingCheckBox.IsChecked;
            dynamic_AU_scale = UseDynamicScalingCheckBox.IsChecked;

            if(use_dynamic_models != UseDynamicModelsCheckBox.IsChecked)
            {
                // Change the face analyser, this should be safe as the model is only allowed to change when not running
                String root = AppDomain.CurrentDomain.BaseDirectory;
                face_analyser = new FaceAnalyserManaged(root, UseDynamicModelsCheckBox.IsChecked);                
            }
            use_dynamic_models = UseDynamicModelsCheckBox.IsChecked;
        }

    }
}
