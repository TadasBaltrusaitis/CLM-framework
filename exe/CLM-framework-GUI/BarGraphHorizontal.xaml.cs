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
    /// Interaction logic for BarGraphHorizontal.xaml
    /// </summary>
    public partial class BarGraphHorizontal : UserControl
    {
        double targetValue = 0;

        public BarGraphHorizontal(String label)
        {
            InitializeComponent();
            Label.Content = label;
        }

        public void SetValue(double value)
        {
            targetValue = value;
            barPos.Width = targetValue * barContainerPos.ActualWidth;
        }

        public double GetTarget()
        {
            return targetValue;
        }
    }
}
