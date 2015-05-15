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

        private volatile bool thread_running;
        private volatile bool thread_paused = false;

        FpsTracker processing_fps = new FpsTracker();

        // TODO rem?
        volatile bool detectionSucceeding = false;
        volatile bool reset = false;

        // For tracking
        CLMParameters clm_params;
        CLM clm_model;
        FaceAnalyserManaged face_analyser;

        // For selecting webcams
        CameraSelection cam_sec;
        
        // Recording parameters (default values)
        bool record_aus = true; // Recording Action Units
        bool record_pose = true; // head location and orientation
        bool record_params = true; // rigid and non-rigid shape parameters
        bool record_2D_landmarks = true; // 2D landmark location
        bool record_3D_landmarks = true; // 3D landmark locations in world coordinates
        bool record_HOG = false; // HOG features extracted from face images
        bool record_aligned = false; // aligned face images
        bool record_tracked_vid = false;

        // TODO if image don't record some of these (unless treated as video?)
        // Separate entries for video/image in opening file menu
        // If image just do landmarks and do multiple faces?

        // The recording managers
        StreamWriter output_head_pose_file;
        StreamWriter output_clm_params_file;
        StreamWriter output_2D_landmarks_file;
        StreamWriter output_3D_landmarks_file;
        StreamWriter output_au_class;
        StreamWriter output_au_reg;

        // Where the recording is done (by default in a record directory, from where the application executed)
        String record_root = "./record";

        // For AU visualisation and output        
        List<String> au_class_names;
        List<String> au_reg_names;

        // For AU prediction
        bool dynamic_AU_shift = true;
        bool dynamic_AU_scale = false;

        // TODO adding resets to the face analyser

        // TODO add a reset button?

        // todo add four classifiers

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
                RecordLandmarks2DCheckBox.IsChecked = record_2D_landmarks;
                RecordLandmarks3DCheckBox.IsChecked = record_3D_landmarks;
                RecordParamsCheckBox.IsChecked = record_params;
                RecordPoseCheckBox.IsChecked = record_pose;
                UseDynamicScalingCheckBox.IsChecked = dynamic_AU_scale;
                UseDynamicShiftingCheckBox.IsChecked = dynamic_AU_shift;
            }));

            clm_params = new CLMParameters();
            clm_model = new CLM();
            face_analyser = new FaceAnalyserManaged();

        }

        private bool ProcessFrame(CLM clm_model, CLMParameters clm_params, RawImage frame, RawImage grayscale_frame, double fx, double fy, double cx, double cy)
        {
            detectionSucceeding = clm_model.DetectLandmarksInVideo(grayscale_frame, clm_params);
            return detectionSucceeding;

        }

        private void SetupRecording(String root, String filename, int width, int height)
        {
            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
            {
                RecordingMenu.IsEnabled = false;
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
            }));

        }

        // Recording the relevant objects
        private void RecordFrame(CLM clm_model, CLMParameters clm_params, bool success_b, int frame_ind, RawImage frame, RawImage grayscale_frame, double fx, double fy, double cx, double cy)
        {
            double confidence = (-clm_model.GetConfidence())/2.0 + 0.5;
            
            List<double> pose = new List<double>();
            clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy, clm_params);

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
        private void ProcessingLoop(String[] filenames, int cam_id = -1, int width = -1, int height = -1)
        {
            thread_running = true;

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0,0,0,0,200), (Action)(() =>
            {
                PauseButton.IsEnabled = true;
                StopButton.IsEnabled = true;
            }));

            // Create the video capture and call the VideoLoop
            if(filenames != null)
            {
                foreach (string filename in filenames)
                {
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
            else
            {
                capture = new Capture(cam_id, width, height);

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

            // Some GUI clean up
            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
            {
                PauseButton.IsEnabled = false;
                StopButton.IsEnabled = false;
            }));

        }

        // Capturing and processing the video frame by frame
        private void VideoLoop()
        {
            Thread.CurrentThread.IsBackground = true;

            DateTime? startTime = CurrentTime;

            var lastFrameTime = CurrentTime;

            clm_model.Reset();
            face_analyser.Reset();

            // TODO add an ability to change these
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
                try
                {
                    frame = new RawImage(capture.GetNextFrame());
                }
                catch (CLM_Interop.CaptureFailedException)
                {
                    // This indicates that we reached the end of the video file
                    break;
                }

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

                List<Tuple<Point, Point>> lines = null;
                List<Tuple<double, double>> landmarks = null;
                
                if (detectionSucceeding)
                {
                    landmarks = clm_model.CalculateLandmarks();
                    lines = clm_model.CalculateBox((float)fx, (float)fy, (float)cx, (float)cy);
                }
                
                double confidence = (-clm_model.GetConfidence()) / 2.0 + 0.5;

                if (confidence < 0)
                    confidence = 0;
                else if (confidence > 1)
                    confidence = 1;

                List<double> pose = new List<double>();
                clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy, clm_params);
                List<double> non_rigid_params = clm_model.GetNonRigidParams();

                // The face analysis step
                face_analyser.AddNextFrame(frame, clm_model, dynamic_AU_shift, dynamic_AU_scale);
                RawImage aligned_face = face_analyser.GetLatestAlignedFace();
                RawImage hog_face = face_analyser.GetLatestHOGDescriptorVisualisation();

                var au_classes = face_analyser.GetCurrentAUsClass();
                var au_regs = face_analyser.GetCurrentAUsReg();

                // Visualisation
                Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0,0,0,0,200), (Action)(() =>
                {
                    auClassGraph.Update(au_classes);

                    var au_regs_scaled = new Dictionary<String, double>();
                    foreach (var au_reg in au_regs)
                        au_regs_scaled[au_reg.Key] = au_reg.Value / 5.0;

                    auRegGraph.Update(au_regs_scaled);                    

                    if (latest_img == null)
                    {
                        latest_img = frame.CreateWriteableBitmap();
                        latest_aligned_face = aligned_face.CreateWriteableBitmap();
                        latest_HOG_descriptor = hog_face.CreateWriteableBitmap();
                    }

                    int yaw = (int)(pose[4] * 180 / Math.PI + 0.5);
                    int roll = (int)(pose[5] * 180 / Math.PI + 0.5);
                    int pitch = (int)(pose[3] * 180 / Math.PI + 0.5);

                    YawLabel.Content = yaw + "°";
                    RollLabel.Content = roll + "°";
                    PitchLabel.Content = pitch + "°";

                    XPoseLabel.Content = (int)pose[0] + " mm";
                    YPoseLabel.Content = (int)pose[1] + " mm";
                    ZPoseLabel.Content = (int)pose[2] + " mm";

                    frame.UpdateWriteableBitmap(latest_img);

                    video.Source = latest_img;
                    video.Confidence = confidence;
                    video.FPS = processing_fps.GetFPS();


                    if (!detectionSucceeding)
                    {
                        video.OverlayLines.Clear();
                        video.OverlayPoints.Clear();
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
                    }

                    nonRigidGraph.Update(non_rigid_params);

                    // Update face analysis frames
                    frame.UpdateWriteableBitmap(latest_img);

                    aligned_face.UpdateWriteableBitmap(latest_aligned_face);
                    hog_face.UpdateWriteableBitmap(latest_HOG_descriptor);
                    
                    AlignedFace.Source = latest_aligned_face;
                    AlignedHOG.Source = latest_HOG_descriptor;

                }));

                // Recording the tracked model
                RecordFrame(clm_model, clm_params, detectionSucceeding, frame_id, frame, grayFrame, fx, fy, cx, cy);

                if (reset)
                {
                    clm_model.Reset();
                    reset = false;
                }

                while (thread_running & thread_paused)
                {
                    Thread.Sleep(10);
                }

                frame_id++;

            }
            latest_img = null;
        }

        private void fileOpenClick(object sender, RoutedEventArgs e)
        {

            var d = new OpenFileDialog();
            d.Multiselect = true;

            if (d.ShowDialog(this) == true)
            {
                // First complete the running of the thread
                if (processing_thread != null)
                {
                    // Tell the other thread to finish
                    thread_running = false;
                    processing_thread.Join();
                }

                string[] video_files = d.FileNames;

                processing_thread = new Thread(() => ProcessingLoop(video_files));
                processing_thread.Start();

            }
        }

        private void openWebcamClick(object sender, RoutedEventArgs e)
        {
            // First complete the running of the thread
            if (processing_thread != null)
            {
                // Let the other thread finish first
                thread_running = false;

                processing_thread.Join();
            }

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
            
            if(!cam_sec.no_cameras_found)
                cam_sec.ShowDialog();

            if (cam_sec.camera_selected)
            {
                int cam_id = cam_sec.selected_camera.Item1;
                int width = cam_sec.selected_camera.Item2;
                int height = cam_sec.selected_camera.Item3;

                processing_thread = new Thread(() => ProcessingLoop(null, cam_id, width, height));
                processing_thread.Start();

            }
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
                StopButton.IsEnabled = false;
                RecordingMenu.IsEnabled = true;
            }
        }

        // Stopping the tracking
        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            if (processing_thread != null)
            {
                // Stop capture and tracking                
                thread_paused = !thread_paused;

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

        // Stopping the tracking
        private void recordCheckBox_click(object sender, RoutedEventArgs e)
        {
            record_aus = RecordAUCheckBox.IsChecked;
            record_aligned = RecordAlignedCheckBox.IsChecked;
            record_HOG = RecordHOGCheckBox.IsChecked;
            record_tracked_vid = RecordTrackedVidCheckBox.IsChecked;
            record_2D_landmarks = RecordLandmarks2DCheckBox.IsChecked;
            record_3D_landmarks = RecordLandmarks3DCheckBox.IsChecked;
            record_params = RecordParamsCheckBox.IsChecked;
            record_pose = RecordPoseCheckBox.IsChecked;

            dynamic_AU_shift = UseDynamicShiftingCheckBox.IsChecked;
            dynamic_AU_scale = UseDynamicScalingCheckBox.IsChecked;

        }

        private void UseDynamicModelsCheckBox_Click(object sender, RoutedEventArgs e)
        {

        }

    }
}
