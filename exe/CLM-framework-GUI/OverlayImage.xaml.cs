using System;
using System.Collections.Generic;
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

namespace CLM_framework_GUI
{
    /// <summary>
    /// Interaction logic for OverlayImage.xaml
    /// </summary>
    public partial class OverlayImage : Image
    {
        public OverlayImage()
        {
            InitializeComponent();
            OverlayLines = new List<Tuple<Point, Point>>();
            OverlayPoints = new List<Point>();
        }

        protected override void OnRender(DrawingContext dc)
        {
            base.OnRender(dc);

            if (OverlayLines == null)
                OverlayLines = new List<Tuple<Point, Point>>();

            if (OverlayPoints == null)
                OverlayPoints = new List<Point>();

            if (Source == null || !(Source is WriteableBitmap))
                return;

            var width = ((WriteableBitmap)Source).PixelWidth;
            var height = ((WriteableBitmap)Source).PixelHeight;

            foreach (var line in OverlayLines)
            {

                var p1 = new Point(ActualWidth * line.Item1.X / width, ActualHeight * line.Item1.Y / height);
                var p2 = new Point(ActualWidth * line.Item2.X / width, ActualHeight * line.Item2.Y / height);

                dc.DrawLine(new Pen(new SolidColorBrush(Color.FromArgb(200, (byte)(100 + (155 * (1-Confidence))), (byte)(100 + (155 * Confidence)), 100)), 2), p1, p2);
            } 
            
            foreach (var p in OverlayPoints)
            {

                var q = new Point(ActualWidth * p.X / width, ActualHeight * p.Y / height);

                dc.DrawEllipse(new SolidColorBrush(Color.FromArgb((byte)(200*Confidence),255,255,100)), null, q, 2, 2);
            }

            // TODO this should be scalable
            double scaling = ActualWidth / 400.0;

            int confidence_width = (int)(107.0 * scaling);
            int confidence_height = (int)(18.0 * scaling);

            Brush conf_brush = new SolidColorBrush(Color.FromRgb((byte)((1 - Confidence) * 255), (byte)(Confidence * 255), (byte)40));
            dc.DrawRoundedRectangle(conf_brush, new Pen(Brushes.Black, 0.5 * scaling), new Rect(ActualWidth - confidence_width - 1, 0, confidence_width, confidence_height), 3.0 * scaling, 3.0 * scaling);

            FormattedText txt = new FormattedText("Confidence: " + (int)(100 * Confidence) + "%", System.Globalization.CultureInfo.CurrentCulture, System.Windows.FlowDirection.LeftToRight, new Typeface("Verdana"), 12.0 * scaling, Brushes.Black);
            dc.DrawText(txt, new Point(ActualWidth - confidence_width + 2, 2));

            int fps_width = (int)(52.0 * scaling);
            int fps_height = (int)(18.0 * scaling);

            dc.DrawRoundedRectangle(Brushes.WhiteSmoke, new Pen(Brushes.Black, 0.5 * scaling), new Rect(ActualWidth / 2 - fps_width/2, ActualHeight - fps_height, fps_width, fps_height), 3.0 * scaling, 3.0 * scaling);
            FormattedText fps_txt = new FormattedText("FPS: " + (int)FPS, System.Globalization.CultureInfo.CurrentCulture, System.Windows.FlowDirection.LeftToRight, new Typeface("Verdana"), 12.0 * scaling, Brushes.Black);
            dc.DrawText(fps_txt, new Point(ActualWidth / 2 - fps_width / 2 + 2.0 * scaling, ActualHeight - fps_height + 1.0 * scaling));

            old_width = width;
            old_height = height;

        }

        public List<Tuple<Point, Point>> OverlayLines { get; set; }
        public List<Point> OverlayPoints { get; set; }        
        public double Confidence { get; set; }
        public double FPS { get; set; }

        int old_width;
        int old_height;
    }
}
