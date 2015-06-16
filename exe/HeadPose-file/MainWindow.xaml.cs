using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing.Imaging;
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
using Camera_Interop;
using CLM_framework_GUI;
using System.Collections.Concurrent;

using ZeroMQ;
using System.Windows.Threading;
using System.Drawing;

namespace HeadPose_file
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

        bool record_head_pose;
        // Indicates if the output file is written in the same directory as the media file or in a recording directory
        bool record_file_dir;
        
        // Capturing and displaying the images
        CLM_framework_GUI.OverlayImage display_img;

        // Some members for displaying the results
        private Camera_Interop.Capture capture;
        private WriteableBitmap latest_img;

        // For visualisation and pose computation
        double fx = 500, fy = 500, cx = 0, cy = 0;
        volatile bool reset = false;

        // For recording
        string record_root = "./head_pose_from_files/";
        bool record_from_image;

        int img_width;
        int img_height;

        // Useful for visualising things
        private volatile int skip_frames = 0;

        volatile bool running = true;
        volatile bool pause = false;

        Uri iconUri;

        string[] files_chosen;

        // Locking objects
        object next_file_lock = new object();

        public MainWindow()
        {
            InitializeComponent();

            DateTime now = DateTime.Now;

            if (now > new DateTime(2015, 9, 1, 0, 0, 0, 0))
            {
                string messageBoxText = "The version of the software has expired. Please contact Tadas Baltrušaitis (Tadas.Baltrusaitis@cl.cam.ac.uk) for an updated version.";
                string caption = "Version expired! (after 2015-September-01)";
                MessageBoxButton button = MessageBoxButton.OK;
                MessageBoxImage icon = MessageBoxImage.Error;
                MessageBoxResult result = MessageBox.Show(messageBoxText, caption, button, icon); 
                this.Close();
            }

            // Set the icon
            iconUri = new Uri("logo1.ico", UriKind.RelativeOrAbsolute);
            this.Icon = BitmapFrame.Create(iconUri);

            // Warn about the liability
            Liability liab = new Liability();
            liab.Icon = BitmapFrame.Create(iconUri);
            liab.ShowDialog();

            if (!liab.continue_pressed)
            {
                this.Close();
                return;
            }

            BitmapImage src = new BitmapImage();
            src.BeginInit();
            src.UriSource = new Uri("logo1.png", UriKind.RelativeOrAbsolute);
            src.CacheOption = BitmapCacheOption.OnLoad;
            src.EndInit();
            
            logoLabel.Source = src;

            // Make sure output directory exists
            if (!System.IO.Directory.Exists(record_root))
            {
                System.IO.Directory.CreateDirectory(record_root);
            }

            // First make the user chooose a file or a set of files
            SelectFiles();

            // Create an overlay image for display purposes
            display_img = new CLM_framework_GUI.OverlayImage();

            display_img.SetValue(Grid.RowProperty, 1);
            display_img.SetValue(Grid.ColumnProperty, 1);
            MainGrid.Children.Add(display_img);            

            // Start the tracking now
            processing_thread = new Thread(VideoLoop);
            processing_thread.Start();

        }

        private bool ProcessFrame(CLM clm_model, CLMParameters clm_params, RawImage frame, RawImage grayscale_frame, double fx, double fy, double cx, double cy)
        {
            bool detection_succeeding = clm_model.DetectLandmarksInVideo(grayscale_frame, clm_params);
            return detection_succeeding;

        }

        private void SelectFiles()
        {
            HeadPoseFile.SelectFiles file_select = new HeadPoseFile.SelectFiles();
            file_select.Icon = BitmapFrame.Create(iconUri);
            file_select.ShowDialog();

            // Check if file is selected if not - close the application
            if (file_select.files_chosen.Count() != 0)
            {
                files_chosen = file_select.files_chosen;

                record_head_pose = file_select.RecordHeadPose;
                record_from_image = file_select.record_from_image;

                if (record_from_image)
                {
                    ResetButton.Visibility = System.Windows.Visibility.Hidden;
                    NextFiveFrameButton.Visibility = System.Windows.Visibility.Collapsed;
                    PauseButton.Visibility = System.Windows.Visibility.Collapsed;
                    NextFrameButton.Visibility = System.Windows.Visibility.Collapsed;
                }
                else
                {
                    ResetButton.Visibility = System.Windows.Visibility.Visible;
                    NextFiveFrameButton.Visibility = System.Windows.Visibility.Visible;
                    PauseButton.Visibility = System.Windows.Visibility.Visible;
                    NextFrameButton.Visibility = System.Windows.Visibility.Visible;
                }

                if (record_head_pose)
                    record_file_dir = file_select.RecordLocationFile;

            }
            else
            {
                this.Close();
            }

        }

        // Capturing and processing the video frame by frame
        private void VideoLoop()
        {

            Thread.CurrentThread.IsBackground = true;

            String root = AppDomain.CurrentDomain.BaseDirectory;

            // This should be pulled out
            CLMParameters clm_params = new CLMParameters(root);
            CLM clm_model = new CLM(clm_params);

            DateTime? startTime = CurrentTime;

            var lastFrameTime = CurrentTime;

            if(record_head_pose && record_file_dir)
                record_root = System.IO.Path.GetDirectoryName(files_chosen[0]);

            int file_counter = 0;

            // Create the capture device for each file (needs to be a loop)
            foreach (string file in files_chosen)            
            {
                // This implies reset has been called
                if (!running)
                    break;

                file_counter++;

                capture = new Camera_Interop.Capture(file);
                img_width = capture.width;
                img_height = capture.height;

                // Set appropriate fx and cx values (these are mainly guesswork)
                fx = 500 * (img_width / 640.0);
                fy = 500 * (img_height / 480.0);

                fx = (fx + fy) / 2.0;
                fy = fx;

                cx = img_width / 2.0;
                cy = img_height / 2.0;

                // Reset image as next video or image might be different size
                latest_img = null;

                if (capture.isOpened() && running)
                {
                    // Creating a recording file
                    System.IO.StreamWriter output_head_pose_file = null;

                    int frame_number = 1;

                    if(record_head_pose)
                    {
                        string output_file_name = System.IO.Path.GetFileNameWithoutExtension(file);
                        output_file_name = System.IO.Path.Combine(record_root, output_file_name + ".pose.txt");

                        output_head_pose_file = new System.IO.StreamWriter(output_file_name);
                        if (record_from_image)
                        {
                            output_head_pose_file.WriteLine("success, pose_X(mm), pose_Y(mm), pose_Z(mm), pitch(deg), yaw(deg), roll(deg)");
                        }
                        else
                        {
                            output_head_pose_file.WriteLine("frame number, success, pose_X(mm), pose_Y(mm), pose_Z(mm), pitch(deg), yaw(deg), roll(deg)");
                        }
                    }

                    while (running)
                    {

                        Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
                        {
                            PauseButton.IsEnabled = true;
                        }));

                        //////////////////////////////////////////////
                        // CAPTURE FRAME AND DETECT LANDMARKS FOLLOWED BY THE REQUIRED IMAGE PROCESSING
                        //////////////////////////////////////////////

                        RawImage frame = null;
                        try
                        {
                            frame = capture.GetNextFrame(false);
                            if (frame.Width == 0)
                                break;
                        }
                        catch (Camera_Interop.CaptureFailedException)
                        {
                            break;
                        }

                        lastFrameTime = CurrentTime;
                        processing_fps.AddFrame();

                        var grayFrame = capture.GetCurrentFrameGray();

                        bool detectionSucceeding = ProcessFrame(clm_model, clm_params, frame, grayFrame, fx, fy, cx, cy);

                        if (record_head_pose)
                        {
                            // Add objects to recording queues
                            List<double> pose = new List<double>();
                            clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy, clm_params);

                            // Write out the pose
                            String output_pose_line = "";
                            if (!record_from_image)
                                output_pose_line += (frame_number + ", ");

                            if (detectionSucceeding)
                                output_pose_line += "1";
                            else
                                output_pose_line += "0";

                            for (int i = 0; i < 6; ++i)
                            {
                                double num = pose[i];
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

                        List<Tuple<System.Windows.Point, System.Windows.Point>> lines = null;
                        List<System.Windows.Point> landmarks = new List<System.Windows.Point>();
                        if (detectionSucceeding)
                        {
                            List<Tuple<double,double>> landmarks_doubles = clm_model.CalculateLandmarks();

                            foreach(var p in landmarks_doubles)
                                landmarks.Add(new System.Windows.Point(p.Item1, p.Item2));

                            lines = clm_model.CalculateBox((float)fx, (float)fy, (float)cx, (float)cy);
                        }

                        if (reset)
                        {
                            clm_model.Reset();
                            reset = false;
                        }

                        // Visualisation updating
                        try
                        {
                            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0,0,0,0,200), (Action)(() =>
                            {
                                if (latest_img == null)
                                    latest_img = frame.CreateWriteableBitmap();

                                List<double> pose = new List<double>();
                                clm_model.GetCorrectedPoseCameraPlane(pose, fx, fy, cx, cy, clm_params);

                                int yaw = (int)(pose[4] * 180 / Math.PI + 0.5);
                                int yaw_abs = Math.Abs(yaw);

                                int roll = (int)(pose[5] * 180 / Math.PI + 0.5);
                                int roll_abs = Math.Abs(roll);

                                int pitch = (int)(pose[3] * 180 / Math.PI + 0.5);
                                int pitch_abs = Math.Abs(pitch);

                                YawLabel.Content = yaw_abs + "°";
                                RollLabel.Content = roll_abs + "°";
                                PitchLabel.Content = pitch_abs + "°";

                                if (yaw > 0)
                                    YawLabelDir.Content = "Left";
                                else if(yaw < 0)
                                    YawLabelDir.Content = "Right";
                                else
                                    YawLabelDir.Content = "Straight";

                                if (pitch > 0)
                                    PitchLabelDir.Content = "Down";
                                else if (pitch < 0)
                                    PitchLabelDir.Content = "Up";
                                else
                                    PitchLabelDir.Content = "Straight";

                                if (roll > 0)
                                    RollLabelDir.Content = "Right";
                                else if (roll < 0)
                                    RollLabelDir.Content = "Left";
                                else
                                    RollLabelDir.Content = "Straight";

                                XPoseLabel.Content = (int)pose[0] + " mm";
                                YPoseLabel.Content = (int)pose[1] + " mm";
                                ZPoseLabel.Content = (int)pose[2] + " mm";

                                double confidence = (- clm_model.GetConfidence() + 1) /2.0;

                                if (confidence < 0)
                                    confidence = 0;
                                else if (confidence > 1)
                                    confidence = 1;

                                frame.UpdateWriteableBitmap(latest_img);
                        
                                display_img.Source = latest_img;
                                display_img.Confidence = confidence;
                                display_img.FPS = processing_fps.GetFPS();
                                if (!detectionSucceeding)
                                {
                                    display_img.OverlayLines.Clear();
                                    display_img.OverlayPoints.Clear();
                                }
                                else
                                {
                                    display_img.OverlayLines = lines;
                                    display_img.OverlayPoints = landmarks;
                        
                                }

                                // updating the progress bar
                                if (!record_from_image)
                                {
                                    display_img.Progress = capture.GetProgress();
                                }

                            }));

                            while (running && pause && skip_frames == 0)
                            {
                                Thread.Sleep(10);
                            }

                            if (skip_frames > 0)
                                skip_frames--;

                            frame_number++;
                        }                        
                        catch (TaskCanceledException)
                        {
                            // Quitting
                            break;
                        }
                    }
                    if (record_head_pose)
                    {
                        output_head_pose_file.Close();
                    }
                }
                else
                {

                    string messageBoxText = "Failed to open the file: " + file + " check if it's actually an image or video file";
                    string caption = "Failed to open the file";
                    MessageBoxButton button = MessageBoxButton.OK;
                    MessageBoxImage icon = MessageBoxImage.Warning;

                    // Display message box
                    MessageBox.Show(messageBoxText, caption, button, icon);
                    this.Close();
                }
                Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
                {
                    PauseButton.IsEnabled = false;
                }));

                // Only lock if no next file and it has not been stopped
                if (running && file_counter < files_chosen.Length)
                {
                    lock (next_file_lock)
                    {
                        // Do not move to next file before the next button is pressed
                        Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
                        {
                            NextFileButton.IsEnabled = true;
                        }));

                        Monitor.Wait(next_file_lock);

                        Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
                        {
                            NextFileButton.IsEnabled = false;
                        }));
                    }
                }
            }   
        }

        private void ResetButton_Click(object sender, RoutedEventArgs e)
        {
            reset = true;
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {

            // Stop capture and tracking
            running = false;
            
            lock (next_file_lock)
            {
                Monitor.Pulse(next_file_lock);
            }

            if (processing_thread != null)
            {
                processing_thread.Join();
            }

            if(capture != null)
                capture.Dispose();

        }

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            pause = !pause;

            if (pause)
            {
                PauseButton.Content = "Resume";
            }
            else
            {
                PauseButton.Content = "Pause";
            }

            if (pause)
            {
                NextFrameButton.IsEnabled = true;
                NextFiveFrameButton.IsEnabled = true;
            }
            else
            {
                NextFrameButton.IsEnabled = false;
                NextFiveFrameButton.IsEnabled = false;
            }
        }

        private void ReplayButton_Click(object sender, RoutedEventArgs e)
        {
            // Restart the videos
            running = false;
            processing_thread.Join();
            
            // Start the tracking now
            processing_thread = new Thread(VideoLoop);
            running = true;
            processing_thread.Start();

        }

        private void SkipButton_Click(object sender, RoutedEventArgs e)
        {
            if (sender == NextFrameButton)
            {
                skip_frames += 1;
            }
            if (sender == NextFiveFrameButton)
            {
                skip_frames += 5;
            }
        }

        private void NextFileButton_Click(object sender, RoutedEventArgs e)
        {
            lock (next_file_lock)
            {
                // Indicate that the next file should be opened
                Monitor.Pulse(next_file_lock);
            }
        }

        private void OpenFiles_Click(object sender, RoutedEventArgs e)
        {
            // Finish current
            running = false;
            processing_thread.Join();

            // Select files
            SelectFiles();

            // Start the tracking now
            processing_thread = new Thread(VideoLoop);
            running = true;
            processing_thread.Start();
        }

        private void ScreenshotButton_Click(object sender, RoutedEventArgs e)
        {

            int Top = (int)this.Top;
            int Left = (int)this.Left;

            int Width = (int)this.Width;
            int Height = (int)this.Height;

            using (Bitmap bmpScreenCapture = new Bitmap(Width,
                                                        Height))
            {
                using (System.Drawing.Graphics g = Graphics.FromImage(bmpScreenCapture))
                {
                    g.CopyFromScreen(Left,
                                     Top,
                                     0, 0,
                                     bmpScreenCapture.Size,
                                     CopyPixelOperation.SourceCopy);
                    
                    // Write out the bitmap here encoded by a time-stamp?
                    String fname = record_root + DateTime.Now.ToString("yyyy-MMM-dd--HH-mm-ss") + ".png";
                    bmpScreenCapture.Save(fname, ImageFormat.Png);
                }
            }
        }

    }
}