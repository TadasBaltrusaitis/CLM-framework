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

        // TODO populate
        private WriteableBitmap latest_aligned_face;
        private WriteableBitmap latest_HOG_descriptor;

        private volatile bool thread_running;

        // TODO rem?
        double fpsLimit = 0;
        FpsTracker processing_fps = new FpsTracker();

        // Keep track of video file (TODO rem)
        string videoFile = null;
        // TODO rem?
        volatile bool detectionSucceeding = false;
        volatile bool reset = false;

        // For tracking
        CLMParameters clm_params;
        CLM clm_model;

        // For updating the GUI
        // TODO
        private Object update_lock = new Object();

        public MainWindow()
        {
            InitializeComponent();

            clm_params = new CLMParameters();
            clm_model = new CLM();
        }

        private bool ProcessFrame(CLM clm_model, CLMParameters clm_params, RawImage frame, RawImage grayscale_frame, double fx, double fy, double cx, double cy)
        {
            detectionSucceeding = clm_model.DetectLandmarksInVideo(grayscale_frame, clm_params);
            return detectionSucceeding;

        }

        // Capturing and processing the video frame by frame
        private void VideoLoop()
        {
            Thread.CurrentThread.IsBackground = true;

            // TODO set these properly
            double fx = 500, fy = 500, cx = 0, cy = 0;

            DateTime? startTime = CurrentTime;

            var lastFrameTime = CurrentTime;

            clm_model.Reset();
            
            while (thread_running)
            {
                //////////////////////////////////////////////
                // CAPTURE FRAME AND DETECT LANDMARKS FOLLOWED BY THE REQUIRED IMAGE PROCESSING
                //////////////////////////////////////////////
                if (fpsLimit > 0)
                {
                    while (CurrentTime < lastFrameTime + TimeSpan.FromSeconds(1 / fpsLimit))
                        Thread.Sleep(1);
                }

                RawImage frame = null;
                try
                {
                    frame = new RawImage(capture.GetNextFrame());
                }
                catch (CLM_Interop.CaptureFailedException)
                {
                    Console.WriteLine("Capture failed");
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

                if (cx == 0 && cy == 0)
                {
                    cx = grayFrame.Width / 2f;
                    cy = grayFrame.Height / 2f;
                }

                bool detectionSucceeding = ProcessFrame(clm_model, clm_params, frame, grayFrame, fx, fy, cx, cy);

                List<Tuple<Point, Point>> lines = null;
                List<Point> landmarks = null;
                
                if (detectionSucceeding)
                {
                    landmarks = clm_model.CalculateLandmarks();
                    lines = clm_model.CalculateBox((float)fx, (float)fy, (float)cx, (float)cy);
                }
                
                // TODO rem?
                var landmarks_3D = clm_model.Calculate3DLandmarks(fx, fy, cx, cy);

                double confidence = (-clm_model.GetConfidence() + 0.6) / 2.0;

                if (confidence < 0)
                    confidence = 0;
                else if (confidence > 1)
                    confidence = 1;

                List<double> pose = new List<double>();
                clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy, clm_params);

                // Visualisation
                Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0,0,0,0,200), (Action)(() =>
                {
                    if (latest_img == null)
                        latest_img = frame.CreateWriteableBitmap();

                    headOrientationLabel.Content = "Head orientation (degrees) - Roll: " + (int)(pose[5] * 180 / Math.PI + 0.5) + " Pitch: " + (int)(pose[3] * 180 / Math.PI + 0.5) + " Yaw: " + (int)(pose[4] * 180 / Math.PI + 0.5);
                    headPoseLabel.Content = "Head pose (mm.) - X: " + (int)pose[0] + " Y: " + (int)pose[1] + " Z: " + (int)pose[2];

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
                        video.OverlayPoints = landmarks;
                    }
                }));

                if (reset)
                {
                    clm_model.Reset();
                    reset = false;
                }


            }
            System.Console.Out.WriteLine("Thread finished");
            latest_img = null;
        }

        private void fileOpenClick(object sender, RoutedEventArgs e)
        {

            var d = new OpenFileDialog();
            if (d.ShowDialog(this) == true)
            {
                videoFile = d.FileName;

                // First complete the running of the thread
                if (processing_thread != null)
                {
                    // Let the other thread finish first
                    thread_running = false;

                    processing_thread.Join();
                }

                capture = new Capture(d.FileName);

                if (capture.isOpened())
                {
                    thread_running = true;

                    processing_thread = new Thread(VideoLoop);
                    processing_thread.Start();
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
                video.InvalidateArrange();

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

            // First close the cameras that might be open
            if (capture != null)
            {
                capture.Dispose();
            }

            CameraSelection cam_sec = new CameraSelection();
            cam_sec.ShowDialog();

            if (cam_sec.camera_selected)
            {
                int cam_id = cam_sec.selected_camera.Item1;
                int width = cam_sec.selected_camera.Item2;
                int height = cam_sec.selected_camera.Item3;

                capture = new Capture(cam_id, width, height);

                if (capture.isOpened())
                {

                    thread_running = true;

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
                }
            }
        }
    }
}
