using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
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
using System.Windows.Threading;

namespace CLM_framework_GUI
{
    public class DataPoint
    {
        public DataPoint()
        {
            Time = TimeSeriesPlot.CurrentTime;
            
        }
        public DateTime Time { get; set; }

        public Dictionary<int, double> values = new Dictionary<int,double>();

        public double Confidence { get; set; }
    }
    /// <summary>
    /// Interaction logic for TimeSeriesPlot.xaml
    /// </summary>
    public partial class TimeSeriesPlot : UserControl
    {

        #region High-Resolution Timing
        static DateTime startTime;
        static Stopwatch sw = new Stopwatch();
        static TimeSeriesPlot()
        {
            startTime = DateTime.Now;
            sw.Start();
        }
        public static DateTime CurrentTime
        {
            get { return startTime + sw.Elapsed; }
        }
        #endregion


        public Orientation Orientation { get; set; }

        Queue<DataPoint> dataPoints = new Queue<DataPoint>();
        TimeSpan historyLength = TimeSpan.FromSeconds(10);
        Dictionary<int, Brush> brushes = new Dictionary<int, Brush>();
        public TimeSeriesPlot()
        {
            InitializeComponent();
            ClipToBounds = true;
            DispatcherTimer dt = new DispatcherTimer(TimeSpan.FromMilliseconds(20), DispatcherPriority.Background, Timer_Tick, Dispatcher);
        }

        private void PruneData()
        {
            lock (dataPoints)
            {
                while (dataPoints.Count > 0 && dataPoints.Peek().Time < CurrentTime - historyLength - TimeSpan.FromSeconds(2))
                    dataPoints.Dequeue();
            }
        }

        public void AddDataPoint(DataPoint dp)
        {
            lock (dataPoints)
                dataPoints.Enqueue(dp);
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            PruneData();

            if (this.IsVisible) 
                InvalidateVisual();
        }

        public void AssocColor(int seriesId, Color b)
        {
            Color bTransparent = b;
            bTransparent.A = 0;
            
            GradientStopCollection gs = new GradientStopCollection();
            gs.Add(new GradientStop(bTransparent, 0));
            gs.Add(new GradientStop(b, 0.2));
            LinearGradientBrush g = new LinearGradientBrush(gs, new Point(0, 0), Orientation == System.Windows.Controls.Orientation.Horizontal ? new Point(ActualWidth, 0) : new Point(0, ActualHeight));
            g.MappingMode = BrushMappingMode.Absolute;
            g.Freeze();
            brushes[seriesId] = g;
        }

        protected override void OnRender(DrawingContext dc)
        {
            base.OnRender(dc);

            DataPoint[] localPoints;
            lock (dataPoints)
                localPoints = dataPoints.ToArray();

            var pfs = new Dictionary<int, PathFigure>();

            for (int i = 0; i < localPoints.Length; i++)
            {
                var ptTime = localPoints[i].Time;
                var ptAge = (DateTime.Now - ptTime).TotalSeconds;
                foreach (var kvp in localPoints[i].values)
                {
                    var seriesId = kvp.Key;

                    double v = Math.Min(Math.Max(kvp.Value, 0), 1);


                    double x = 0; 
                    double y = 0; 

                    if (Orientation == System.Windows.Controls.Orientation.Horizontal)
                    {
                        x = ActualWidth - (CurrentTime - localPoints[i].Time).TotalMilliseconds * (ActualWidth / historyLength.TotalMilliseconds);
                        y = ActualHeight - ActualHeight * v;
                    }
                    else
                    {
                        y = ActualHeight - (CurrentTime - localPoints[i].Time).TotalMilliseconds * (ActualHeight / historyLength.TotalMilliseconds);
                        x = ActualWidth * v;
                    }

                    if (!pfs.ContainsKey(seriesId))
                    {
                        pfs[seriesId] = new PathFigure();
                        pfs[seriesId].IsClosed = false;
                        pfs[seriesId].StartPoint = new Point(x, y);
                    }
                    else
                    {
                        pfs[seriesId].Segments.Add(new LineSegment(new Point(x, y), true));
                    }
                }
            }


            foreach (var kvp in pfs)
            {
                var seriesId = kvp.Key;
                var pf = kvp.Value;

                Brush b = brushes.ContainsKey(seriesId) ? brushes[seriesId] : Brushes.Black;
                    
                PathGeometry pg = new PathGeometry(new PathFigure[] { pf });
                dc.DrawGeometry(null, new Pen(b, 2), pg);
            }
        }


    }
}
