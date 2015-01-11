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

        public MainWindow()
        {
            InitializeComponent();

            //new Thread(ProcessLoop).Start();
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

            CLMParameters clm_params = new CLMParameters();
            CLM clm_model = new CLM();
            double fx = 500, fy = 500, cx = 0, cy = 0;

            DateTime? startTime = CurrentTime;

            var lastFrameTime = CurrentTime;

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

                if (reset)
                {
                    clm_model.Reset();                    
                    reset = false;
                }

                // The actual frame processing


                // Visualisation
                try
                {
                    Dispatcher.Invoke(() =>
                    {
                        if (latest_img == null)
                            latest_img = frame.CreateWriteableBitmap();

                        fpsLabel.Content = "Processing: " + processing_fps.GetFPS().ToString("0");
                        List<double> pose = new List<double>();
                        clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy, clm_params);

                        headOrientationLabel.Content = "Head orientation (degrees) - Roll: " + (int)(pose[5] * 180 / Math.PI + 0.5) + " Pitch: " + (int)(pose[3] * 180 / Math.PI + 0.5) + " Yaw: " + (int)(pose[4] * 180 / Math.PI + 0.5);
                        headPoseLabel.Content = "Head pose (mm.) - X: " + (int)pose[0] + " Y: " + (int)pose[1] + " Z: " + (int)pose[2];
                        
                        // TODO rem?
                        var landmarks_3D = clm_model.Calculate3DLandmarks(fx, fy, cx, cy);

                        //landmarks3DLabel.Content = "X:" + landmarks_3D[0].X + "Y:" + landmarks_3D[0].Y + "Z:" + landmarks_3D[0].Z + "X:" + landmarks_3D[30].X + "Y:" + landmarks_3D[30].Y + "Z:" + landmarks_3D[30].Z;

                        frame.UpdateWriteableBitmap(latest_img);
                        video.Source = latest_img;

                        if (!detectionSucceeding)
                        {
                            video.OverlayLines.Clear();
                            video.OverlayPoints.Clear();
                        }
                        else
                        {
                            video.OverlayLines = lines;
                            video.OverlayPoints = landmarks;
                            // TODO edit
                            video.Confidence = 1;
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

        private void fileOpenClick(object sender, RoutedEventArgs e)
        {
            var d = new OpenFileDialog();
            if (d.ShowDialog(this) == true)
            {
                videoFile = d.FileName;

                capture = new Capture(d.FileName);

                if (capture.isOpened())
                {
                    if (processing_thread != null)
                    {
                        // Let the other thread finish first
                        while (processing_thread.IsAlive)
                            thread_running = false;
                    }

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
            }
        }

        private void openWebcamClick(object sender, RoutedEventArgs e)
        {

            // First close the cameras that might be open
            if(capture != null)
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

                // TODO repetition here, need code that stops the old thread and then start the new one
                if (capture.isOpened())
                {
                    if (processing_thread != null)
                    {
                        // Let the other thread finish first
                        while (processing_thread.IsAlive)
                            thread_running = false;
                    }

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
