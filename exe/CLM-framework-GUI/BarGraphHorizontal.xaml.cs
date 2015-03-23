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

            Dictionary<String, String> mapping = new Dictionary<string, string>();
            mapping["AU01"] = "Inner Brow raiser";
            mapping["AU02"] = "Outer Brow raiser";
            mapping["AU04"] = "Brow lowerer";
            mapping["AU05"] = "Upper lid raiser";
            mapping["AU06"] = "Cheek raiser";
            mapping["AU07"] = "Lid tightener";
            mapping["AU09"] = "Nose wrinkler";
            mapping["AU10"] = "Upper lip raiser";
            mapping["AU12"] = "Lip corner puller (smile)";
            mapping["AU14"] = "Dimpler";
            mapping["AU15"] = "Lip corner depressor";
            mapping["AU17"] = "Chin Raiser";
            mapping["AU20"] = "Lip Stretcher";
            mapping["AU23"] = "Lip tightener";
            mapping["AU25"] = "Lips part";
            mapping["AU26"] = "Jaw drop";
            mapping["AU28"] = "Lip suck";
            mapping["AU45"] = "Blink";

            Label.Content = mapping[label];
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
