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
using CLM_framework_GUI;
using System.Collections.Concurrent;

namespace Ophthalm_experiments
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

        CLM_framework_GUI.FpsTracker processing_fps = new CLM_framework_GUI.FpsTracker();

        Thread processing_thread;

        string patient_id;
        bool record_video;
        bool record_head_pose;

        // Capturing and displaying the images
        CLM_framework_GUI.OverlayImage webcam_img;

        // Some members for displaying the results
        private Capture capture;
        private WriteableBitmap latest_img;

        // For tracking
        double fx = 500, fy = 500, cx = 0, cy = 0;
        bool reset = false;

        // For recording
        string record_root = "./recorded/patient ";
        string output_root;
        private Object recording_lock = new Object();
        int trial_id = 0;
        bool recording = false;
        int img_width;
        int img_height;

        double seconds_to_record = 10;

        System.IO.StreamWriter recording_success_file = null;

        ConcurrentQueue<Tuple<RawImage, bool, List<double>>> recording_objects;

        public void StartExperiment()
        {
            // Inquire more from the user

            // Get the entry dialogue now for the patient ID
            trial_id = 0;
            TextEntryWindow patient_id_window = new TextEntryWindow();
            patient_id_window.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            
            if (patient_id_window.ShowDialog() == true)
            {

                patient_id = patient_id_window.ResponseText;

                output_root = record_root + patient_id + "/";

                if (System.IO.Directory.Exists(output_root))
                {
                    string messageBoxText = "The recording for patient already exists, are you sure you want to continue?";
                    string caption = "Directory exists!";
                    MessageBoxButton button = MessageBoxButton.YesNo;
                    MessageBoxImage icon = MessageBoxImage.Warning;
                    MessageBoxResult result = MessageBox.Show(messageBoxText, caption, button, icon);
                    if (result == MessageBoxResult.No)
                    {
                        this.Close();
                    }

                    // Else find the latest trial from which to continu
                    int trial_id_not_found = 0;
                    while (System.IO.File.Exists(output_root + '/' + "trial_" + trial_id_not_found + ".avi"))
                    {
                        trial_id_not_found++;
                    }
                    trial_id = trial_id_not_found;
                }

                System.IO.Directory.CreateDirectory(output_root);

                record_video = patient_id_window.RecordVideo;
                record_head_pose = patient_id_window.RecordHeadPose;
                RecordingButton.Content = "Record trial: " + trial_id;

            }
            else
            {
                this.Close();
            }

        }

        public MainWindow()
        {
            InitializeComponent();
            
            // First make the user chooose a webcam
            CLM_framework_GUI.CameraSelection cam_select = new CLM_framework_GUI.CameraSelection();
            cam_select.ShowDialog();

            if (cam_select.camera_selected)
            {

                // Create the capture device
                int cam_id = cam_select.selected_camera.Item1;
                img_width = cam_select.selected_camera.Item2;
                img_height = cam_select.selected_camera.Item3;

                capture = new Capture(cam_id, img_width, img_height);

                // Set appropriate fx and cx values
                fx = fx * (img_width / 640.0);
                fy = fy * (img_height / 480.0);

                fx = (fx + fy) / 2.0;
                fy = fx;

                cx = img_width / 2.0;
                cy = img_height / 2.0;

                if (capture.isOpened())
                {
                    processing_thread = new Thread(VideoLoop);
                    processing_thread.Start();
                }
                else
                {

                    string messageBoxText = "Failed to open a webcam";
                    string caption = "Webcam failure";
                    MessageBoxButton button = MessageBoxButton.OK;
                    MessageBoxImage icon = MessageBoxImage.Warning;

                    // Display message box
                    MessageBox.Show(messageBoxText, caption, button, icon);
                    this.Close();
                }

                // Create an overlay image for display purposes
                webcam_img = new CLM_framework_GUI.OverlayImage();

                webcam_img.SetValue(Grid.ColumnProperty, 0);
                webcam_img.SetValue(Grid.RowProperty, 1);
                MainGrid.Children.Add(webcam_img);

                StartExperiment();
                
            }
            else
            {
                cam_select.Close();
                this.Close();
            }

        }

        private bool ProcessFrame(CLM clm_model, CLMParameters clm_params, RawImage frame, RawImage grayscale_frame, double fx, double fy, double cx, double cy)
        {
            bool detection_succeeding = clm_model.DetectLandmarksInVideo(grayscale_frame, clm_params);
            return detection_succeeding;

        }

        // Capturing and processing the video frame by frame
        private void RecordingLoop()
        {
            // Set up the recording objects first
            Thread.CurrentThread.IsBackground = true;

            System.IO.StreamWriter output_head_pose_file = null;

            if(record_head_pose)
            {
                String filename_poses = output_root + "/trial_" + trial_id + ".poses.txt";
                output_head_pose_file = new System.IO.StreamWriter(filename_poses);
                output_head_pose_file.WriteLine("time(ms), success, pose_X(mm), pose_Y(mm), pose_Z(mm), pitch(deg), yaw(deg), roll(deg)");
            }
            
            VideoWriter video_writer = null;
            
            if(record_video)
            {                
                double fps = processing_fps.GetFPS();
                String filename_video = output_root + "/trial_" + trial_id + ".avi";
                video_writer = new VideoWriter(filename_video, img_width, img_height, fps, true);
            }

            // The timiing should be when the item is captured, but oh well
            Stopwatch stopWatch = new Stopwatch();
            stopWatch.Start();                    
                    
            while (recording)
            {
                Tuple<RawImage, bool, List<double>> recording_object;
                if (recording_objects.TryDequeue(out recording_object))
                {

                    if (record_video)
                    {
                        video_writer.Write(recording_object.Item1);
                    }

                    if (record_head_pose)
                    {
                        String output_pose_line = stopWatch.ElapsedMilliseconds.ToString();
                        if (recording_object.Item2)
                            output_pose_line += ", 1";
                        else
                            output_pose_line += ", 0";

                        for (int i = 0; i < recording_object.Item3.Count; ++i)
                        {
                            double num = recording_object.Item3[i];
                            if (i > 2)
                            {
                                output_pose_line += ", " + num * 180 / Math.PI;
                            }
                            else
                            {
                                output_pose_line += ", " + num;
                            }
                        }
                        output_head_pose_file.WriteLine(output_pose_line);
                    }
                }
                
                Thread.Sleep(10);
            }

            // Clean up the recording
            if (record_head_pose)
            {
                output_head_pose_file.Close();
            }            
        }

        // Capturing and processing the video frame by frame
        private void VideoLoop()
        {
            Thread.CurrentThread.IsBackground = true;

            CLMParameters clm_params = new CLMParameters();
            CLM clm_model = new CLM();

            DateTime? startTime = CurrentTime;

            var lastFrameTime = CurrentTime;

            while (true)
            {

                //////////////////////////////////////////////
                // CAPTURE FRAME AND DETECT LANDMARKS FOLLOWED BY THE REQUIRED IMAGE PROCESSING
                //////////////////////////////////////////////

                RawImage frame = null;
                try
                {
                    frame = capture.GetNextFrame();
                }
                catch (CLM_Interop.CaptureFailedException)
                {
                    break;
                }

                lastFrameTime = CurrentTime;
                processing_fps.AddFrame();

                var grayFrame = capture.GetCurrentFrameGray();

                if (grayFrame == null)
                    continue;

                bool detectionSucceeding = ProcessFrame(clm_model, clm_params, frame, grayFrame, fx, fy, cx, cy);

                lock (recording_lock)
                {

                    if (recording)
                    {
                        // Add objects to recording queues
                        List<double> pose = new List<double>();
                        clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy, clm_params);
                        RawImage image = new RawImage(frame);
                        recording_objects.Enqueue(new Tuple<RawImage, bool, List<double>>(image, detectionSucceeding, pose));
 
                    }
                }

                List<Tuple<Point, Point>> lines = null;
                List<Point> landmarks = null;
                if (detectionSucceeding)
                {
                    landmarks = clm_model.CalculateLandmarks();
                    lines = clm_model.CalculateBox((float)fx, (float)fy, (float)cx, (float)cy);
                }

                if (reset)
                {
                    clm_model.Reset();
                    reset = false;
                }

                // The actual frame processing
                try
                {
                    Dispatcher.Invoke(() =>
                    {
                        if (latest_img == null)
                            latest_img = frame.CreateWriteableBitmap();

                        fpsLabel.Content = "FPS: " + processing_fps.GetFPS().ToString("0");
                        List<double> pose = new List<double>();
                        clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy, clm_params);

                        //headOrientationLabel.Content = "Head orientation (degrees) - Roll: " + (int)(pose[5] * 180 / Math.PI + 0.5) + " Pitch: " + (int)(pose[3] * 180 / Math.PI + 0.5) + " Yaw: " + (int)(pose[4] * 180 / Math.PI + 0.5);
                        YawLabel.Content = (int)(pose[4] * 180 / Math.PI + 0.5) + "°";
                        RollLabel.Content = (int)(pose[5] * 180 / Math.PI + 0.5) + "°";
                        PitchLabel.Content = (int)(pose[3] * 180 / Math.PI + 0.5) + "°";

                        XPoseLabel.Content = (int)pose[0] + " mm";
                        YPoseLabel.Content = (int)pose[1] + " mm";
                        ZPoseLabel.Content = (int)pose[2] + " mm";
                        //headPoseLabel.Content = "Head pose (mm.) - X: " + (int)pose[0] + " Y: " + (int)pose[1] + " Z: " + (int)pose[2];

                        frame.UpdateWriteableBitmap(latest_img);
                        webcam_img.Source = latest_img;

                        if (!detectionSucceeding)
                        {
                            webcam_img.OverlayLines.Clear();
                            webcam_img.OverlayPoints.Clear();
                        }
                        else
                        {
                            webcam_img.OverlayLines = lines;
                            webcam_img.OverlayPoints = landmarks;
                            webcam_img.Confidence = 1;
                        }
                    });
                }
                catch (TaskCanceledException)
                {
                    // Quitting
                    break;
                }
            }
            System.Console.Out.WriteLine("Thread finished");
        }

        private void startRecordingButton_Click(object sender, RoutedEventArgs e)
        {
            lock (recording_lock)
            {
                RecordingButton.IsEnabled = false;
                CompleteButton.IsEnabled = false;
                recording_objects = new ConcurrentQueue<Tuple<RawImage, bool, List<double>>>();

                recording = true;

                new Thread(() =>
                {
                    Thread.CurrentThread.IsBackground = true;
                    
                    // Start the recording thread
                    Thread rec_thread = new Thread(RecordingLoop);
                    rec_thread.Start();

                    double d = seconds_to_record * 1000;

                    Stopwatch stopWatch = new Stopwatch();
                    stopWatch.Start();                    
                    
                    while(d > 1000)
                    {
                    
                        Dispatcher.Invoke(() =>
                        {
                            RecordingButton.Content = ((int)(d / 1000)).ToString() + " seconds remaining";
                        }); 
                        
                        System.Threading.Thread.Sleep(1000);

                        d = seconds_to_record * 1000 - stopWatch.ElapsedMilliseconds;
                    }

                    if (d > 0)
                    {
                        System.Threading.Thread.Sleep((int)(d));
                    }

                    recording = false;

                    Dispatcher.Invoke(() =>
                    {
                        RecordingButton.Content = "0 seconds remaining";
                    }); 

                    Dispatcher.Invoke(() =>
                    {
                        lock (recording_lock)
                        {

                            // Wait for the recording thread to finish before enabling
                            rec_thread.Join();

                            string messageBoxText = "Was the tracking successful?";
                            string caption = "Success of tracking";
                            MessageBoxButton button = MessageBoxButton.YesNo;
                            MessageBoxImage icon = MessageBoxImage.Question;
                            MessageBoxResult result = MessageBox.Show(messageBoxText, caption, button, icon);

                            if (recording_success_file == null)
                            {
                                recording_success_file = new System.IO.StreamWriter(output_root + "/recording_success.txt", true);
                            }

                            if (result == MessageBoxResult.Yes)
                            {
                                recording_success_file.WriteLine('1');
                            }
                            else
                            {
                                recording_success_file.WriteLine('0');
                            }
                            recording_success_file.Flush();

                            trial_id++;
                            RecordingButton.Content = "Record trial: " + trial_id;
                            RecordingButton.IsEnabled = true;
                            CompleteButton.IsEnabled = true;
                        }

                    });
                }).Start();
            }
        }

        private void ResetButton_Click(object sender, RoutedEventArgs e)
        {
            reset = true;
        }

        private void CompleteButton_Click(object sender, RoutedEventArgs e)
        {
            StartExperiment();
        }

    }
}