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

        // Managing the running of the analysis system
        private volatile bool thread_running;

        FpsTracker processing_fps = new FpsTracker();

        volatile bool detectionSucceeding = false;

        volatile bool reset = false;

        // For tracking
        CLMParameters clm_params;
        CLM clm_model;
        FaceAnalyserManaged face_analyser;

        // For selecting webcams
        CameraSelection cam_sec;
                
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
       
        // The main function call for processing images, video files or webcam feed
        private void ProcessingLoop(String[] filenames, int cam_id = -1, int width = -1, int height = -1, bool multi_face = false)
        {

            thread_running = true;

            mirror_image = false;

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0,0,0,0,200), (Action)(() =>
            {

                rapportPlot.AssocColor(0, Colors.Blue);
                valencePlot.AssocColor(0, Colors.Green);
                valencePlot.AssocColor(1, Colors.Red);
                attentionPlot.AssocColor(0, Colors.Red);
                attentionPlot.AssocColor(1, Colors.Brown);
                attentionPlot.AssocColor(2, Colors.BlueViolet);

                attentionPlot.AssocThickness(0, 3);
                attentionPlot.AssocName(0, "Attention");
                attentionPlot.AssocName(1, "Head attention");
                attentionPlot.AssocName(2, "Gaze attention");
                attentionPlot.AssocThickness(1, 1);
                attentionPlot.AssocThickness(2, 1);

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
                        // Start the actual processing                        
                        VideoLoop();                        
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
                            // Start the actual processing                        
                            VideoLoop();
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
                    // Start the actual processing
                    VideoLoop();
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
                
                double time_stamp = (DateTime.Now - (DateTime)startTime).TotalMilliseconds;
                // The face analysis step (only done if recording AUs, HOGs or video)
                face_analyser.AddNextFrame(frame, clm_model, time_stamp, fx, fy, cx, cy, true, false, false);

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

                    Dictionary<int, double> rapportDict = new Dictionary<int, double>();
                    rapportDict[0] = (face_analyser.GetRapport() - 1.0)/ 6.5;
                    rapportPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = rapportDict, Confidence = confidence });

                    Dictionary<int, double> attentionDict = new Dictionary<int, double>();
                    attentionDict[0] = (face_analyser.GetAttention() - 1.0) / 6.5;
                    attentionDict[1] = face_analyser.GetHeadAttention();
                    attentionDict[2] = face_analyser.GetEyeAttention();
                    attentionPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = attentionDict, Confidence = confidence });

                    Dictionary<int, double> valenceDict = new Dictionary<int, double>();
                    valenceDict[0] = (face_analyser.GetValence() - 1.0) / 6.5;
                    valenceDict[1] = face_analyser.GetArousal();
                    valencePlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = valenceDict, Confidence = confidence });

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

                }));

                if (reset)
                {
                    clm_model.Reset();
                    face_analyser.Reset();
                    reset = false;
                }

                frame_id++;


            }

            latest_img = null;
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

        private void VisualisationCheckBox_Click(object sender, RoutedEventArgs e)
        {
            VideoBorder.Visibility = System.Windows.Visibility.Visible;
            MainGrid.ColumnDefinitions[0].Width = new GridLength(2.1, GridUnitType.Star);              
        }

        private void SetupImageMode()
        {

            // Actually update the GUI accordingly
            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 2000), (Action)(() =>
            {                
                VisualisationCheckBox_Click(null, null);
            }));

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
