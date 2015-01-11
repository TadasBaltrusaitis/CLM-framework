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
using System.Windows.Shapes;

using CLM_Interop;

namespace CLM_framework_GUI
{
    /// <summary>
    /// Interaction logic for CameraSelection.xaml
    /// </summary>
    public partial class CameraSelection : Window
    {

        List<Border> sample_images;
        List<ComboBox> combo_boxes;

        // id, width, height
        public Tuple<int, int, int> selected_camera;

        List<List<Tuple<int, int>>> resolutions_all;
        int selected_camera_idx = -1;

        // indicate if user clicked on camera
        public bool camera_selected = false;

        public CameraSelection()
        {
            InitializeComponent();

            // Finding the cameras here
            var cams = Capture.GetCameras();
            
            int i = 0;

            sample_images = new List<Border>();

            // Each cameras corresponding resolutions
            resolutions_all = new List<List<Tuple<int, int>>>();
            combo_boxes = new List<ComboBox>();

            foreach (var s in cams)
            {
                Console.WriteLine(s.Item1);

                var b = s.Item3.CreateWriteableBitmap();
                s.Item3.UpdateWriteableBitmap(b);
                b.Freeze();


                Dispatcher.Invoke(() =>
                {
                    int idx = i;
                    Image img = new Image();
                    img.Source = b;
                    img.Margin = new Thickness(20);
                    camerasPanel.ColumnDefinitions.Add(new ColumnDefinition());

                    Border img_border = new Border();
                    img_border.SetValue(Grid.ColumnProperty, i);
                    img_border.SetValue(Grid.RowProperty, 1);
                    img_border.CornerRadius = new CornerRadius(5);
                    img_border.Child = img;
                    sample_images.Add(img_border);

                    camerasPanel.Children.Add(img_border);

                    ComboBox resolutions = new ComboBox();
                    resolutions.Width = 80;
                    combo_boxes.Add(resolutions);

                    resolutions_all.Add(new List<Tuple<int,int>>());

                    foreach (var r in s.Item2)
                    {
                        resolutions.Items.Add(r.Item1 + "x" + r.Item2);
                        resolutions_all[resolutions_all.Count - 1].Add(new Tuple<int, int>(r.Item1, r.Item2));
                        
                    }

                    resolutions.SelectedIndex = (int)(s.Item2.Count / 2);

                    resolutions.SetValue(Grid.ColumnProperty, i);
                    resolutions.SetValue(Grid.RowProperty, 2);
                    camerasPanel.Children.Add(resolutions);

                    img.MouseDown += (sender, e) =>
                    {
                        camera_selected = true;
                        ChooseCamera(idx);
                    };


                });

                i++;

            }
            if (cams.Count > 0)
            {
                ChooseCamera(0);
            }
            else
            {
                string messageBoxText = "No cameras detected, please connect a webcam";
                string caption = "Camera error!";
                MessageBoxButton button = MessageBoxButton.OK;
                MessageBoxImage icon = MessageBoxImage.Warning;
                MessageBox.Show(messageBoxText, caption, button, icon);
                selected_camera_idx = -1;
                this.Close();
            }
        }

        private void ChooseCamera(int idx)
        {
            selected_camera_idx = idx;

            foreach (var img in sample_images)
            {
                img.BorderThickness = new Thickness(2);
                img.Background = Brushes.SkyBlue;
                img.BorderBrush = Brushes.Black;
            }
            sample_images[idx].BorderThickness = new Thickness(5);
            sample_images[idx].Background = Brushes.Honeydew;
            sample_images[idx].BorderBrush = Brushes.Green;

        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            camera_selected = true;

            int selected_res = combo_boxes[selected_camera_idx].SelectedIndex;
            Tuple<int, int> resolution_selected = resolutions_all[selected_camera_idx][selected_res];

            selected_camera = new Tuple<int, int, int>(selected_camera_idx, resolution_selected.Item1, resolution_selected.Item2);

            this.Close();

        }
    }
}
