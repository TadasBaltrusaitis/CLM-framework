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
    public partial class SimpleImage : Image
    {
        public SimpleImage()
        {
            InitializeComponent();         
        }

        protected override void OnRender(DrawingContext dc)
        {
            base.OnRender(dc);

            if (Source == null || !(Source is WriteableBitmap))
                return;
        }
    }
}
