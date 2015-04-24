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
    /// Interaction logic for BarGraph.xaml
    /// </summary>
    public partial class BarGraph : UserControl
    {
        private double targetValue = 0;
        
        public BarGraph()
        {
            InitializeComponent();
        }

        public void SetValue(double value)
        {
            targetValue = 1.5 * value;
            if (targetValue > 0)
            {
                if (targetValue > barContainerPos.ActualHeight)
                    targetValue = barContainerPos.ActualHeight;

                barPos.Height = targetValue;
                barNeg.Height = 0;
            }
            if (targetValue < 0)
            {
                if (-targetValue > barContainerNeg.ActualHeight)
                    targetValue = -barContainerNeg.ActualHeight;

                barPos.Height = 0;
                barNeg.Height = -targetValue;
            }
        }

    }
}
