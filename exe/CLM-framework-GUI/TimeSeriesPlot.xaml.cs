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

        public bool ShowLegend { get; set; }

        Queue<DataPoint> dataPoints = new Queue<DataPoint>();
        TimeSpan historyLength = TimeSpan.FromSeconds(10);
        Dictionary<int, Brush> brushes = new Dictionary<int, Brush>();
        Dictionary<int, int> brush_thicknesses = new Dictionary<int, int>();
        Dictionary<int, String> line_names = new Dictionary<int, String>();
        Dictionary<int, Color> brush_colors = new Dictionary<int, Color>();

        public TimeSeriesPlot()
        {
            InitializeComponent();
            ShowLegend = false;
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

        private FormattedText FT(string text, int size)
        {
            return new FormattedText(text, System.Globalization.CultureInfo.CurrentCulture, System.Windows.FlowDirection.LeftToRight, new Typeface("Verdana"), size, Brushes.Black);
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

            brush_colors[seriesId] = b;
        }

        public void AssocThickness(int seriesId, int thickness)
        {
            brush_thicknesses[seriesId] = thickness;
        }

        public void AssocName(int seriesId, String name)
        {
            line_names[seriesId] = name;
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

                int thickness = brush_thicknesses.ContainsKey(seriesId) ? brush_thicknesses[seriesId] : 2;

                PathGeometry pg = new PathGeometry(new PathFigure[] { pf });


                Pen p = new Pen(b, thickness);

                if(line_names.ContainsKey(seriesId) && line_names[seriesId].CompareTo("State rapport") == 0)
                {
                    double[] dashValues = { 5.0, 5.0, 5.0 };
                    p.DashStyle = new System.Windows.Media.DashStyle(dashValues, 0);
                }
                dc.DrawGeometry(null, p, pg);
            }

            if (ShowLegend && line_names.Count > 0)
            {
                int height_one = 18;
                int height = height_one * line_names.Count;

                Pen p = new Pen(Brushes.Black, 1);
                Brush legend_b = new SolidColorBrush(Color.FromRgb(255, 255, 255));

                dc.DrawRectangle(legend_b, p, new Rect(0, 1, 100, height));

                int i = 0;
                foreach (var key_name_pair in line_names)
                {
                    var line_name = key_name_pair.Value;
                    FormattedText ft = FT(line_name, 11);

                    // Draw the text
                    dc.DrawText(ft, new Point(15, 1 + height_one * i));
                    // Draw example lines

                    Brush legend_c = new SolidColorBrush(brush_colors[key_name_pair.Key]);
                    Pen p_line = new Pen(legend_c, brush_thicknesses[key_name_pair.Key]);
                    dc.DrawLine(p_line, new Point(0, 1 + height_one * i + height_one / 2), new Point(14, 1 + height_one * i + height_one / 2));
                    i++;
                }
            }

        }


    }
}
