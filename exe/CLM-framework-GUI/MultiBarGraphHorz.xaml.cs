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

        public MultiBarGraphHorz()
        {
            InitializeComponent();

            graphs = new Dictionary<string,BarGraphHorizontal>();
        }

        public void Update(Dictionary<String, double> data)
        {
            // Create new bars if necessary
            if (num_bars != data.Count)
            {
                num_bars = data.Count;
                barGrid.Children.Clear();
                foreach (var value in data)
                {
                    BarGraphHorizontal newBar = new BarGraphHorizontal(value.Key);
                    barGrid.RowDefinitions.Add(new RowDefinition());
                    Grid.SetRow(newBar, graphs.Count);
                    graphs.Add(value.Key, newBar);
                    barGrid.Children.Add(newBar);
                }
            }

            // Update the bars
            foreach (var value in data)
            {
                graphs[value.Key].SetValue(value.Value);               
            }
        }
    }
}
