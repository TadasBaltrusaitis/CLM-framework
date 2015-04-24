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
    /// Interaction logic for MultiBarGraph.xaml
    /// </summary>
    public partial class MultiBarGraph : UserControl
    {

        int num_bars = 0;
        List<BarGraph> graphs;

        public MultiBarGraph()
        {
            InitializeComponent();

            graphs = new List<BarGraph>();
        }

        public void Update(List<double> data)
        {
            // Create new bars if necessary
            if (num_bars != data.Count)
            {
                num_bars = data.Count;
                barGrid.Children.Clear();
                foreach (var value in data)
                {
                    BarGraph newBar = new BarGraph();
                    graphs.Add(newBar);
                    barGrid.ColumnDefinitions.Add(new ColumnDefinition());
                    Grid.SetColumn(newBar, graphs.Count);
                    barGrid.Children.Add(newBar);

                }
            }

            // Update the bars
            for(int i = 0 ; i < data.Count; ++i)
            {
                graphs[i].SetValue(data[i]);
            }
        }
    }
}
