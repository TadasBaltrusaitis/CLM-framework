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
    /// <summary>
    /// Interaction logic for TimeSeriesPlot.xaml
    /// </summary>
    public partial class PCMPlot : UserControl
    {

        #region High-Resolution Timing
        static DateTime startTime;
        static Stopwatch sw = new Stopwatch();
        static PCMPlot()
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

        public double MaxPoints { get; set; }

        Queue<double> dataPoints = new Queue<double>();
        Brush brush;
        int brush_thickness;
        Color brush_color;

        public PCMPlot()
        {
            InitializeComponent();
            ShowLegend = false;
            ClipToBounds = true;
            MaxPoints = 8000;
            brush_thickness = 1;
            brush_color = Colors.Black;

            DispatcherTimer dt = new DispatcherTimer(TimeSpan.FromMilliseconds(20), DispatcherPriority.Background, Timer_Tick, Dispatcher);
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            PruneData();

            if (this.IsVisible)
                InvalidateVisual();
        }

        private void PruneData()
        {
            lock (dataPoints)
            {
                while (dataPoints.Count > 0 && dataPoints.Count > MaxPoints)
                    dataPoints.Dequeue();
            }
        }

        public void AddDataPoint(List<double> dps)
        {
            lock (dataPoints)
                for (int i = 0; i < dps.Count; ++i)
                { 
                    dataPoints.Enqueue(dps[i]);
                }
        }

        private FormattedText FT(string text, int size)
        {
            return new FormattedText(text, System.Globalization.CultureInfo.CurrentCulture, System.Windows.FlowDirection.LeftToRight, new Typeface("Verdana"), size, Brushes.Black);
        }

        public void AssocColor(Color b)
        {
            Color bTransparent = b;
            bTransparent.A = 0;
            
            GradientStopCollection gs = new GradientStopCollection();            
            gs.Add(new GradientStop(bTransparent, 0));
            gs.Add(new GradientStop(b, 0.2));
            LinearGradientBrush g = new LinearGradientBrush(gs, new Point(0, 0), Orientation == System.Windows.Controls.Orientation.Horizontal ? new Point(ActualWidth, 0) : new Point(0, ActualHeight));
            g.MappingMode = BrushMappingMode.Absolute;
            g.Freeze();
            brush = g;

            brush_color = b;
        }

        public void AssocThickness(int thickness)
        {
            brush_thickness = thickness;
        }
        
        protected override void OnRender(DrawingContext dc)
        {
            base.OnRender(dc);

            double[] localPoints;
            lock (dataPoints)
                localPoints = dataPoints.ToArray();

            PathFigure pf = null;

            for (int i = localPoints.Length - 1; i >= 0; i--)
            {
                double v = Math.Min(Math.Max(localPoints[i], 0), 1);

                double x = 0;
                double y = 0;

                if (Orientation == System.Windows.Controls.Orientation.Horizontal)
                {
                    //x = ActualWidth - (CurrentTime - localPoints[i].Time).TotalMilliseconds * (ActualWidth / historyLength.TotalMilliseconds);
                    x = ActualWidth * (1 - (double)i / MaxPoints);
                    y = ActualHeight - ActualHeight * v;
                }
                else
                {
                    y = ActualHeight * (1 - (double)i / MaxPoints); 
                    x = ActualWidth * v;
                }

                if(pf == null)
                {
                    pf = new PathFigure();
                    pf.IsClosed = false;
                    pf.StartPoint = new Point(x, y);
                }
                else
                {
                    pf.Segments.Add(new LineSegment(new Point(x, y), true));
                }
            }
            
            if(pf!=null)
            {
                Brush b = brush != null ? brush : Brushes.Black;

                int thickness = brush_thickness;

                PathGeometry pg = new PathGeometry(new PathFigure[] { pf });
                dc.DrawGeometry(null, new Pen(b, thickness), pg);            
            }
        }


    }
}
