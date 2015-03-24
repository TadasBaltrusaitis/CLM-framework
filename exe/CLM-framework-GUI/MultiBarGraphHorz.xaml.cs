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
    /// Interaction logic for MultiBarGraphHorz.xaml
    /// </summary>
    public partial class MultiBarGraphHorz : UserControl
    {
        int num_bars = 0;
        Dictionary<String, BarGraphHorizontal> graphs;

        // Name mapping
        Dictionary<String, String> mapping;

        public MultiBarGraphHorz()
        {
            InitializeComponent();

            graphs = new Dictionary<string,BarGraphHorizontal>();

            mapping = new Dictionary<string, string>();
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



        }

        public void Update(Dictionary<String, double> data)
        {
            // Create new bars if necessary
            if (num_bars != data.Count)
            {
                num_bars = data.Count;
                barGrid.Children.Clear();

                // Make sure AUs are sorted
                var data_labels = data.Keys.ToList();
                data_labels.Sort();

                foreach (var label in data_labels)
                {
                    BarGraphHorizontal newBar = new BarGraphHorizontal(label + " - " + mapping[label]);
                    barGrid.RowDefinitions.Add(new RowDefinition());
                    Grid.SetRow(newBar, graphs.Count);
                    graphs.Add(label, newBar);
                    barGrid.Children.Add(newBar);
                }
            }

            // Update the bars
            foreach (var value in data)
            {
                double old_value = graphs[value.Key].GetTarget();
                // some smoothing as well
                graphs[value.Key].SetValue(old_value * 0.15 + 0.85 * value.Value);               
            }
        }
    }
}
