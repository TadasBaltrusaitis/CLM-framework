using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace CLM_framework_GUI
{
    class AxesBorder : Border
    {

        public double MinVal { get; set; }
        public double MaxVal { get; set; }
        public int NumVertGrid { get; set; }

        public bool ShowXLabel { get; set; }
        public bool ShowYLabel { get; set; }

        public string RangeLabel { get; set; }

        public bool XTicks { get; set; }

        public Orientation Orientation { get; set; }

        public AxesBorder()
        {
            MinVal = -1;
            MaxVal = 1;
            NumVertGrid = 5;
            ShowXLabel = true;
            ShowYLabel = true;
            XTicks = true;
        }

        protected override void OnRender(DrawingContext dc)
        {
            base.OnRender(dc);

            if (Orientation == System.Windows.Controls.Orientation.Horizontal)
                RenderHorizontal(dc);
            else
                RenderVertical(dc);
        }

        private void RenderHorizontal(DrawingContext dc)
        {
            Pen p = new Pen(Brushes.Black, 1);
            Pen q = new Pen(Brushes.LightGray, 1);

            double padLeft = Padding.Left;
            double padBottom = Padding.Bottom - 2 + 10;
            double padTop = Padding.Top;
            double padRight = Padding.Right;

            // Draw horizontal gridlines

            double step_size = (MaxVal - MinVal)/(NumVertGrid - 1.0);

            for (int i = 0; i < NumVertGrid; i++)
            {
                double y = (int)(padTop + ((NumVertGrid - 1.0) - i) * ((ActualHeight - padBottom - padTop) / (NumVertGrid - 1.0))) - 0.5;

                double y_val = MinVal + i * step_size;

                if (y_val != 0)
                    dc.DrawLine(q, new Point(padLeft, y), new Point(ActualWidth - padRight, y));
                else
                    dc.DrawLine(p, new Point(padLeft, y), new Point(ActualWidth - padRight, y));

                dc.DrawLine(p, new Point(padLeft - 10, y), new Point(padLeft, y));

                //var t = FT((i / 2.0 - 1).ToString("0.0"), 10);
                var t = FT((MinVal + i * step_size).ToString("0.0"), 10);
                dc.DrawText(t, new Point(padLeft - t.Width - 12, y - t.Height / 2));
            }

            // Draw vertical gridlines

            for (int i = 0; i < 11; i++)
            {
                double x = (int)(padLeft + (10 - i) * ((ActualWidth - padLeft - padRight) / 10.0)) - 0.5;
                if (i < 10)
                    dc.DrawLine(q, new Point(x, ActualHeight - padBottom), new Point(x, padTop));
                dc.DrawLine(p, new Point(x, ActualHeight - padBottom + 10), new Point(x, ActualHeight - padBottom));

                if(XTicks)
                { 
                    var t = FT(i.ToString(), 10);
                    dc.DrawText(t, new Point(x - t.Width / 2, ActualHeight - padBottom + t.Height));
                }
            }

            // Draw y axis

            dc.DrawLine(p, new Point(((int)padLeft) - 0.5, padTop), new Point(((int)padLeft) - 0.5, ActualHeight - padBottom));

            // Draw x axis
            //dc.DrawLine(p, new Point(padLeft, ((int)((ActualHeight - padBottom - padTop) / 2 + padTop)) - 0.5), new Point(ActualWidth - padRight, ((int)((ActualHeight - padBottom - padTop) / 2 + padTop)) - 0.5));

            // Draw x axis label
            if(ShowXLabel)
            { 
                FormattedText ft = FT("History (seconds)", 20);
                dc.DrawText(ft, new Point(padLeft + (ActualWidth - padLeft - padRight) / 2 - ft.Width / 2, ActualHeight - ft.Height));
            }

            // Draw y axis label
            if(ShowYLabel)
            {
                FormattedText ft = FT(RangeLabel, 20);
                dc.PushTransform(new RotateTransform(-90));
                dc.DrawText(ft, new Point(-ft.Width - ActualHeight / 2 + ft.Width / 2, 0));
            }
        }

        private void RenderVertical(DrawingContext dc)
        {
            Pen p = new Pen(Brushes.Black, 1);
            Pen q = new Pen(Brushes.LightGray, 1);

            double padLeft = Padding.Left;
            double padBottom = Padding.Bottom - 2 + 10;
            double padTop = Padding.Top;
            double padRight = Padding.Right;

            // Draw horizontal gridlines

            for (int i = 0; i < 11; i++)
            {
                double y = (int)(padTop + (10 - i) * ((ActualHeight - padBottom - padTop) / 10.0)) - 0.5;
                if (i > 0)
                    dc.DrawLine(q, new Point(padLeft, y), new Point(ActualWidth - padRight, y));
                dc.DrawLine(p, new Point(padLeft - 10, y), new Point(padLeft, y));
                var t = FT(i.ToString(), 10);
                dc.DrawText(t, new Point(padLeft - t.Width - 12, y - t.Height / 2));
            }

            // Draw vertical gridlines

            for (int i = 0; i < 5; i++)
            {
                double x = (int)(padLeft + (4 - i) * ((ActualWidth - padLeft - padRight) / 4.0)) - 0.5;
                if (i < 10)
                    dc.DrawLine(q, new Point(x, ActualHeight - padBottom), new Point(x, padTop));
                dc.DrawLine(p, new Point(x, ActualHeight - padBottom + 10), new Point(x, ActualHeight - padBottom));

                var t = FT(((4-i) / 2.0 - 1).ToString("0.0"), 10);
                dc.DrawText(t, new Point(x - t.Width / 2, ActualHeight - padBottom + t.Height));
            }

            // Draw y axis

            dc.DrawLine(p, new Point(((int)((ActualWidth - padRight - padLeft) / 2 + padLeft)) - 0.5, padTop), new Point(((int)((ActualWidth - padRight - padLeft) / 2 + padLeft)) - 0.5, ActualHeight - padBottom));

            // Draw x axis
            dc.DrawLine(p, new Point(padLeft, ((int)((ActualHeight - padBottom))) - 0.5), new Point(ActualWidth - padRight, ((int)((ActualHeight - padBottom))) - 0.5));

            // Draw x axis label

            FormattedText ft = FT(RangeLabel, 20);
            dc.DrawText(ft, new Point(padLeft + (ActualWidth - padLeft - padRight) / 2 - ft.Width / 2, ActualHeight - ft.Height));

            // Draw y axis label

            ft = FT("History (seconds)", 20);
            dc.PushTransform(new RotateTransform(-90));
            dc.DrawText(ft, new Point(-ft.Width - ActualHeight / 2 + ft.Width / 2, 0));
        }

        private FormattedText FT(string text, int size)
        {
            return new FormattedText(text, CultureInfo.CurrentCulture, System.Windows.FlowDirection.LeftToRight, new Typeface("Verdana"), size, Brushes.Black);
        }

    }
}
