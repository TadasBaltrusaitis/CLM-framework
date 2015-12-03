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
using NAudio;

using OpenCVWrappers;
using CLM_Interop;
using CLM_Interop.CLMTracker;
using FaceAnalyser_Interop;
using Camera_Interop;
using System.Windows.Threading;
using System.IO;
using System.Net.Sockets;
using System.Net;
using System.Xml;

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

        volatile int rapport_fixed = 4;

        // Will be reading the info differently if stuff is being loaded from the socket
        Socket listener;

        // For audio recording
        NAudio.Wave.WaveIn waveIn;
        int audio_device_number = 0;

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

            // TODO ability to choose the audio device
            int waveInDevices = NAudio.Wave.WaveIn.DeviceCount;
            for (int waveInDevice = 0; waveInDevice < waveInDevices; waveInDevice++)
            {
                NAudio.Wave.WaveInCapabilities deviceInfo = NAudio.Wave.WaveIn.GetCapabilities(waveInDevice);
                Console.WriteLine("Device {0}: {1}, {2} channels",
                    waveInDevice, deviceInfo.ProductName, deviceInfo.Channels);
            }

            if (waveInDevices > 0)
            {
                waveIn = new NAudio.Wave.WaveIn();
                new Thread(() => startAudioStreaming()).Start();
            }
        }

        void startAudioStreaming()
        {
            Thread.CurrentThread.IsBackground = true;
            waveIn.DeviceNumber = audio_device_number;
            waveIn.DataAvailable += waveIn_DataAvailable;
            int sampleRate = 8000; // 8 kHz
            int channels = 1; // mono
            waveIn.WaveFormat = new NAudio.Wave.WaveFormat(sampleRate, channels);
            waveIn.StartRecording();
            Console.WriteLine("Starting recording");
        }

        void waveIn_DataAvailable(object sender, NAudio.Wave.WaveInEventArgs e)
        {
            Thread.CurrentThread.IsBackground = true;
            List<double> pcms = new List<double>();
            for (int index = 0; index < e.BytesRecorded; index += 2)
            {
                short sample = (short)((e.Buffer[index + 1] << 8) |
                                        e.Buffer[index + 0]);
                float sample32 = sample / 32768f;
                pcms.Add(sample32 + 0.5);
                //Console.WriteLine(sample32);
                //ProcessSample(sample32);
            }

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
            {
                pcmPlot.AddDataPoint(pcms);
            }));

            Thread.Sleep(25);
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
                rapportPlot.AssocColor(1, Colors.Gray);
                rapportPlot.AssocName(1, "Trait rapport");
                rapportPlot.AssocName(0, "State rapport");
                rapportPlot.AssocThickness(0, 2);
                rapportPlot.AssocThickness(1, 1);

                attentionPlot.AssocColor(0, Colors.Red);
                attentionPlot.AssocColor(1, Colors.Blue);
                attentionPlot.AssocColor(2, Colors.Green);

                attentionPlot.AssocThickness(0, 3);
                attentionPlot.AssocName(0, "Attention");
                attentionPlot.AssocName(1, "Head attention");
                attentionPlot.AssocName(2, "Gaze attention");
                attentionPlot.AssocThickness(1, 2);
                attentionPlot.AssocThickness(2, 2);

                smilePlot.AssocColor(0, Colors.Green);
                smilePlot.AssocColor(1, Colors.Red);
                smilePlot.AssocName(0, "Smile");
                smilePlot.AssocName(1, "Frown");
                smilePlot.AssocThickness(0, 2);
                smilePlot.AssocThickness(1, 2);

                browPlot.AssocColor(0, Colors.Green);
                browPlot.AssocColor(1, Colors.Red);
                browPlot.AssocName(0, "Raise");
                browPlot.AssocName(1, "Furrow");
                browPlot.AssocThickness(0, 2);
                browPlot.AssocThickness(1, 2);

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

        double smile_cumm = 0;
        double frown_cumm = 0;
        double brow_up_cumm = 0;
        double brow_down_cumm = 0;

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

                    var au_regs = face_analyser.GetCurrentAUsReg();

                    double smile = (au_regs["AU12"] + au_regs["AU06"]) / 7.5 + 0.05;
                    double frown = (au_regs["AU15"] + au_regs["AU17"] + au_regs["AU04"]) / 10.0 + 0.05;

                    double brow_up = (au_regs["AU01"] + au_regs["AU02"]) / 7.5 + 0.05;
                    double brow_down = au_regs["AU04"] / 5.0 + 0.05;

                    Dictionary<int, double> smileDict = new Dictionary<int, double>();
                    smileDict[0] = 0.4 * smile_cumm + 0.6 * smile;
                    smileDict[1] = 0.4* frown_cumm + 0.6 * frown;
                    smilePlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = smileDict, Confidence = confidence });

                    Dictionary<int, double> browDict = new Dictionary<int, double>();
                    browDict[0] = 0.4 * brow_up_cumm + 0.6 * brow_up;
                    browDict[1] = 0.4 * brow_down_cumm + 0.6 * brow_down;
                    browPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = browDict, Confidence = confidence });

                    smile_cumm = smile;
                    frown_cumm = frown;
                    brow_up_cumm = brow_up;
                    brow_down_cumm = brow_down;

                    Dictionary<int, double> speechDict = new Dictionary<int, double>();
                    speechDict[0] = face_analyser.GetSpeech() + 0.05;
                    speechPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = speechDict, Confidence = confidence });

                    Dictionary<int, double> rapportDict = new Dictionary<int, double>();
                    rapportDict[0] = (face_analyser.GetRapport() - 1.0)/ 6.5;
                    rapportDict[1] = (rapport_fixed - 1.0)/ 6.0;
                    rapportPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = rapportDict, Confidence = confidence });

                    Dictionary<int, double> attentionDict = new Dictionary<int, double>();
                    attentionDict[0] = (face_analyser.GetAttention() - 1.0) / 6.5;
                    attentionDict[1] = face_analyser.GetHeadAttention();
                    attentionDict[2] = face_analyser.GetEyeAttention();
                    attentionPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = attentionDict, Confidence = confidence });

                    //Dictionary<int, double> valenceDict = new Dictionary<int, double>();
                    //valenceDict[0] = (face_analyser.GetValence() - 1.0) / 6.5;
                    //valenceDict[1] = face_analyser.GetArousal();
                    //valencePlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = valenceDict, Confidence = confidence });

                    Dictionary<int, double> avDict = new Dictionary<int, double>();
                    avDict[0] = (face_analyser.GetArousal() - 0.5) * 2.0;
                    avDict[1] = ((face_analyser.GetValence() - 1.0) / 6.5 - 0.5)*2;
                    avPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = avDict, Confidence = confidence });

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

        // Capturing and processing the video frame by frame
        private void SocketLoop()
        {
            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
            {

                rapportPlot.AssocColor(0, Colors.Blue);
                rapportPlot.AssocColor(1, Colors.Gray);
                rapportPlot.AssocName(1, "Trait rapport");
                rapportPlot.AssocName(0, "State rapport");
                rapportPlot.AssocThickness(0, 2);
                rapportPlot.AssocThickness(1, 1);

                //valencePlot.AssocColor(0, Colors.Green);
                //valencePlot.AssocColor(1, Colors.Red);
                attentionPlot.AssocColor(0, Colors.Red);
                attentionPlot.AssocColor(1, Colors.Blue);
                attentionPlot.AssocColor(2, Colors.Green);

                attentionPlot.AssocThickness(0, 3);
                attentionPlot.AssocName(0, "Attention");
                attentionPlot.AssocName(1, "Head attention");
                attentionPlot.AssocName(2, "Gaze attention");
                attentionPlot.AssocThickness(1, 2);
                attentionPlot.AssocThickness(2, 2);

                smilePlot.AssocColor(0, Colors.Green);
                smilePlot.AssocColor(1, Colors.Red);
                smilePlot.AssocName(0, "Smile");
                smilePlot.AssocName(1, "Frown");
                smilePlot.AssocThickness(0, 2);
                smilePlot.AssocThickness(1, 2);

                browPlot.AssocColor(0, Colors.Green);
                browPlot.AssocColor(1, Colors.Red);
                browPlot.AssocName(0, "Raise");
                browPlot.AssocName(1, "Furrow");
                browPlot.AssocThickness(0, 2);
                browPlot.AssocThickness(1, 2);

            }));

            Thread.CurrentThread.IsBackground = true;

            DateTime? startTime = CurrentTime;

            var lastFrameTime = CurrentTime;

            // TODO add an ability to change these through a calibration procedure or setting menu
            double fx, fy, cx, cy;
            fx = 500.0;
            fy = 500.0;
            cx = cy = -1;

            int frame_id = 0;
            Byte[] bytes = new Byte[2097152];
            listener.ReceiveBufferSize = 2097152;
            while (thread_running)
            {

                // Keep receiving the message
                String data = "";

                // Loop to receive all the data sent by the client.
                int i = listener.Receive(bytes);

                // Translate data bytes to a ASCII string.
                data += System.Text.Encoding.ASCII.GetString(bytes, 0, i);

                if (i == 0)
                {
                    Console.WriteLine("No message received");
                    continue;
                }

                // Just parse the string from and do multisense_vis bits, to make sure it's a valid message
                string begin_tag = "<multisense_vis>";
                string end_tag = "</multisense_vis>";
                int begin = data.IndexOf(begin_tag);
                int end = data.IndexOf(end_tag);
                if(begin < 0 || end < 0 || begin > end)
                {
                    Console.WriteLine("Message does not contain full XML");
                    continue;
                }

                data.Substring(begin, end + end_tag.Length);

                Console.WriteLine(i + " " + data.Length);

                // Read the XML file here
                XmlReader reader = XmlReader.Create(new StringReader(data));
                
                List<Tuple<double, double>> landmarks = null;
                
                Dictionary<string,double> au_regs = face_analyser.GetCurrentAUsReg();
                double confidence = 0;
                bool detection_succeeding = false;
                List<double> pcms = new List<double>();

                double rapport = 0;
                double arousal = 0;
                double valence = 0;
                double speech = 0;
                double eye_gaze = 0;
                double head_gaze = 0;
                double attention = 0;
                
                List<Tuple<Point, Point>> lines = null;
                List<Tuple<Point, Point>> gaze_lines = null;

                try { 
                    while (reader.Read())
                    {
                        switch (reader.NodeType)
                        {
                            case XmlNodeType.Element:
                                if (String.Compare(reader.Name, "gaze_lines") == 0)
                                {
                                    reader.Read();
                                    char[] delimiterChars = { ',' };
                                    string[] lines_s = reader.Value.Split(delimiterChars);

                                    gaze_lines = new List<Tuple<Point, Point>>();

                                    for (int k = 0; k < lines_s.Length; k += 4)
                                    {

                                        double px1 = 0, py1 = 0, px2 = 0, py2 = 0;
                                        Double.TryParse(lines_s[k], out px1);
                                        Double.TryParse(lines_s[k + 1], out py1);
                                        Double.TryParse(lines_s[k + 2], out px2);
                                        Double.TryParse(lines_s[k + 3], out py2);

                                        Point p1 = new Point(px1, py1);
                                        Point p2 = new Point(px2, py2);
                                        gaze_lines.Add(new Tuple<Point, Point>(p1, p2));
                                    }

                                }
                                if (String.Compare(reader.Name, "box") == 0)
                                {
                                    reader.Read();
                                    char[] delimiterChars = { ',' };
                                    string[] lines_s = reader.Value.Split(delimiterChars);

                                    lines = new List<Tuple<Point, Point>>();

                                    for (int k = 0; k < lines_s.Length; k+=4)
                                    {
                                        Point p1 = new Point(Double.Parse(lines_s[k]), Double.Parse(lines_s[k + 1]));
                                        Point p2 = new Point(Double.Parse(lines_s[k+2]), Double.Parse(lines_s[k + 3]));
                                        lines.Add(new Tuple<Point, Point>(p1, p2));
                                    }

                                }
                                if(String.Compare(reader.Name, "flm") == 0)
                                {
                                    reader.Read();
                                    char[] delimiterChars = {','};
                                    string[] lmarks = reader.Value.Split(delimiterChars);

                                    int n_lmarks = (lmarks.Length - 2) / 2;

                                    landmarks = new List<Tuple<double, double>>(n_lmarks);

                                    for(int k=0; k < n_lmarks; ++k)
                                    {
                                        landmarks.Add(new Tuple<double,double>(Double.Parse(lmarks[2+k]), Double.Parse(lmarks[2+k+n_lmarks])));
                                    }

                                }
                                if (String.Compare(reader.Name, "aus") == 0)
                                {
                                    reader.Read();
                                    char[] delimiterChars = { ',' };
                                    string[] aus = reader.Value.Split(delimiterChars);

                                    detection_succeeding = Int32.Parse(aus[2]) == 1;

                                    confidence = Double.Parse(aus[1]);

                                    au_regs["AU01"] = Double.Parse(aus[3]);
                                    au_regs["AU02"] = Double.Parse(aus[4]);
                                    au_regs["AU04"] = Double.Parse(aus[5]);
                                    au_regs["AU05"] = Double.Parse(aus[6]);
                                    au_regs["AU06"] = Double.Parse(aus[7]);
                                    au_regs["AU09"] = Double.Parse(aus[8]);
                                    au_regs["AU10"] = Double.Parse(aus[9]);
                                    au_regs["AU12"] = Double.Parse(aus[10]);
                                    au_regs["AU14"] = Double.Parse(aus[11]);
                                    au_regs["AU15"] = Double.Parse(aus[12]);
                                    au_regs["AU17"] = Double.Parse(aus[13]);
                                    au_regs["AU20"] = Double.Parse(aus[14]);
                                    au_regs["AU25"] = Double.Parse(aus[15]);
                                    au_regs["AU26"] = Double.Parse(aus[16]);

                                }
                                if (String.Compare(reader.Name, "rapport") == 0)
                                {
                                    reader.Read();
                                    char[] delimiterChars = { ',' };
                                    string[] rapports = reader.Value.Split(delimiterChars);

                                    rapport = Double.Parse(rapports[0]);
                                    arousal = Double.Parse(rapports[1]);
                                    valence = Double.Parse(rapports[2]);
                                    speech = Double.Parse(rapports[3]);
                                    eye_gaze = Double.Parse(rapports[4]);
                                    head_gaze = Double.Parse(rapports[5]);
                                    attention = Double.Parse(rapports[6]);
                                    
                                }
                                if (String.Compare(reader.Name, "image") == 0)
                                {
                                    reader.Read();                                
                                    byte[] to_send = Convert.FromBase64String(reader.Value);

                                    MemoryStream stream = new MemoryStream(to_send);

                                    JpegBitmapDecoder decoder = new JpegBitmapDecoder(stream, BitmapCreateOptions.PreservePixelFormat, BitmapCacheOption.Default);
                                    BitmapSource bitmapSource = decoder.Frames[0];
                                    latest_img = new WriteableBitmap(bitmapSource);
                                }
                                if (String.Compare(reader.Name, "audio") == 0)
                                {
                                    reader.Read();
                                    char[] delimiterChars = { ',' };
                                    string[] pcms_s = reader.Value.Split(delimiterChars);

                                    int n_samples = pcms_s.Length;

                                    //int max_pcm = 350;

                                    for (int k = 0; k < n_samples; ++k)
                                    {
                                        pcms.Add(((Double.Parse(pcms_s[k])) + 1.0)/2.0);
                                        //landmarks.Add(new Tuple<double, double>(Double.Parse(lmarks[2 + k]), Double.Parse(lmarks[2 + k + n_lmarks])));
                                    }

                                }
                                //Console.WriteLine(reader.Name);
                                break;
                            case XmlNodeType.Text:
                                //Console.WriteLine(reader.Value);
                                break;
                            case XmlNodeType.XmlDeclaration:
                            case XmlNodeType.ProcessingInstruction:
                                //Console.WriteLine(reader.Name + " " + reader.Value);
                                break;
                            case XmlNodeType.Comment:
                                //Console.WriteLine(reader.Value);
                                break;
                        }
                    }
                } catch (XmlException e){
                    continue;
                }

                lastFrameTime = CurrentTime;
                processing_fps.AddFrame();

                // This is more ore less guess work, but seems to work well enough
                if (cx == -1)
                {
                    fx = fx * (latest_img.Width / 640.0);
                    fy = fy * (latest_img.Height / 480.0);

                    fx = (fx + fy) / 2.0;
                    fy = fx;

                    cx = latest_img.Width / 2f;
                    cy = latest_img.Height / 2f;
                }

                if (confidence < 0)
                    confidence = 0;
                else if (confidence > 1)
                    confidence = 1;
                
                latest_img.Freeze();
                
                // Visualisation
                Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
                {

                    double smile = (au_regs["AU12"] + au_regs["AU06"]) / 7.5 + 0.05;
                    double frown = (au_regs["AU15"] + au_regs["AU17"] + au_regs["AU04"]) / 10.0 + 0.05;

                    double brow_up = (au_regs["AU01"] + au_regs["AU02"]) / 7.5 + 0.05;
                    double brow_down = au_regs["AU04"] / 5.0 + 0.05;

                    Dictionary<int, double> smileDict = new Dictionary<int, double>();
                    smileDict[0] = 0.4 * smile_cumm + 0.6 * smile;
                    smileDict[1] = 0.4 * frown_cumm + 0.6 * frown;
                    smilePlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = smileDict, Confidence = confidence });

                    Dictionary<int, double> browDict = new Dictionary<int, double>();
                    browDict[0] = 0.4 * brow_up_cumm + 0.6 * brow_up;
                    browDict[1] = 0.4 * brow_down_cumm + 0.6 * brow_down;
                    browPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = browDict, Confidence = confidence });

                    smile_cumm = smile;
                    frown_cumm = frown;
                    brow_up_cumm = brow_up;
                    brow_down_cumm = brow_down;

                    Dictionary<int, double> speechDict = new Dictionary<int, double>();
                    speechDict[0] = speech + 0.05;
                    speechPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = speechDict, Confidence = confidence });

                    Dictionary<int, double> rapportDict = new Dictionary<int, double>();
                    rapportDict[0] = (rapport - 1.0) / 6.0;
                    rapportDict[1] = rapport_fixed / 7.0;
                    rapportPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = rapportDict, Confidence = confidence });

                    Dictionary<int, double> attentionDict = new Dictionary<int, double>();
                    attentionDict[0] = (attention - 1.0) / 6.5;
                    attentionDict[1] = head_gaze;
                    attentionDict[2] = eye_gaze;
                    attentionPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = attentionDict, Confidence = confidence });

                    //Dictionary<int, double> valenceDict = new Dictionary<int, double>();
                    //valenceDict[0] = (face_analyser.GetValence() - 1.0) / 6.5;
                    //valenceDict[1] = face_analyser.GetArousal();
                    //valencePlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = valenceDict, Confidence = confidence });

                    Dictionary<int, double> avDict = new Dictionary<int, double>();
                    avDict[0] = (arousal - 0.5) * 2.0;
                    avDict[1] = ((valence - 1.0) / 6.5 - 0.5) * 2;
                    avPlot.AddDataPoint(new DataPoint() { Time = CurrentTime, values = avDict, Confidence = confidence });

                    pcmPlot.AddDataPoint(pcms);

                    video.Source = latest_img;
                    video.Confidence = confidence;
                    video.FPS = processing_fps.GetFPS();

                    if (!detection_succeeding)
                    {
                        video.OverlayLines.Clear();
                        video.OverlayPoints.Clear();
                        video.GazeLines.Clear();
                    }
                    else
                    {
                        //video.OverlayLines = lines;

                        List<Point> landmark_points = new List<Point>();
                        foreach (var p in landmarks)
                        {
                            landmark_points.Add(new Point(p.Item1, p.Item2));
                        }

                        video.OverlayPoints = landmark_points;

                        //video.GazeLines = gaze_lines;
                    }

                }));

                frame_id++;
                
            //latest_img = null;
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
                if(capture != null)
                    capture.Dispose();
            }
            if(face_analyser != null)
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

        private void openSocketClick(object sender, RoutedEventArgs e)
        {

            new Thread(() => openSocket()).Start();
        }

        private void openSocket()
        {
            StopTracking();

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 2, 0), (Action)(() =>
            {
                TextEntryWindow connection_settings = new TextEntryWindow();
                connection_settings.Icon = this.Icon;

                connection_settings.WindowStartupLocation = WindowStartupLocation.CenterScreen;

                if (connection_settings.ShowDialog() == true)
                {
                    string port = connection_settings.ResponsePortText;

                    thread_running = true;

                    // Create a connection and open one as well
                    //new Thread(() => CreateFakeConnection(port)).Start();

                    Thread.Sleep(100);
                    int port_i = Int32.Parse(port);

                    IPHostEntry ipHostInfo = Dns.Resolve(Dns.GetHostName());
                    IPAddress ipAddress = ipHostInfo.AddressList[0];
                    IPEndPoint remoteEP = new IPEndPoint(ipAddress,port_i);

                    //IPEndPoint remoteEP = new IPEndPoint(IPAddress.Parse(IP),port_i);

                    listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                    listener.Connect(remoteEP);

                    processing_thread = new Thread(() => SocketLoop());
                    processing_thread.Start();
                }
                
            }));
        }

        private void Window_KeyDown(object sender, KeyEventArgs e)
        {

            if (e.Key == Key.Up)
            {
                rapport_fixed = rapport_fixed + 1;
            }
            if (e.Key == Key.Down)
            {
                rapport_fixed = rapport_fixed - 1;
            }

            if (rapport_fixed > 7)
                rapport_fixed = 7;
            if (rapport_fixed < 1)
                rapport_fixed = 1;

        }

        //private void CreateFakeConnection(string port)
        //{
        //    int port_i = Int32.Parse(port);
        //    
        //    IPHostEntry ipHostInfo = Dns.Resolve(Dns.GetHostName());
        //    IPAddress ipAddress = ipHostInfo.AddressList[0];
        //    IPEndPoint localEndPoint = new IPEndPoint(ipAddress, port_i);
        //
        //    Socket server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        //    server.Bind(localEndPoint);
        //    //TcpListener server = new TcpListener(localAddr, port_i);
        //    //server.Start();
        //
        //    string xmlString = System.IO.File.ReadAllText("SampleMultiSense.xml");
        //
        //    //Console.WriteLine(xmlString);
        //    server.Listen(10);
        //
        //    while(thread_running)
        //    {
        //        Console.Write("Waiting for a connection... ");
        //        Socket handler = server.Accept();
        //        
        //        //TcpClient client = server.AcceptTcpClient();
        //        Console.WriteLine("Connected!");
        //
        //        // Get a stream object for reading and writing
        //        //NetworkStream stream = client.GetStream();
        //
        //        // Keep resending the same message
        //        while (thread_running)
        //        {
        //            byte[] to_send = System.Text.Encoding.ASCII.GetBytes(xmlString);
        //
        //            int bytes_send = handler.Send(to_send);
        //            
        //            //stream.Write(to_send, 0, to_send.Length);
        //            //stream.Write(xmlString.to)
        //            //Console.WriteLine("Data sent!" + bytes_send);
        //            Thread.Sleep(33);
        //        }
        //   }
        //
        //}

    }
}
