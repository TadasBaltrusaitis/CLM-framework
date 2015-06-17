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
using System.Windows.Threading;
using System.Threading;

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

        public bool no_cameras_found = false;

        public List<Tuple<String, List<Tuple<int, int>>, OpenCVWrappers.RawImage>> cams;

        public void PopulateCameraSelections()
        {
            this.KeyDown += new KeyEventHandler(CameraSelection_KeyDown);

            // Finding the cameras here
            if (cams == null)
            {
                String root = AppDomain.CurrentDomain.BaseDirectory;
                cams = Camera_Interop.Capture.GetCameras(root);
            }

            int i = 0;

            sample_images = new List<Border>();

            // Each cameras corresponding resolutions
            resolutions_all = new List<List<Tuple<int, int>>>();
            combo_boxes = new List<ComboBox>();

            foreach (var s in cams)
            {

                var b = s.Item3.CreateWriteableBitmap();
                s.Item3.UpdateWriteableBitmap(b);
                b.Freeze();

                Dispatcher.Invoke(() =>
                {
                    int idx = i;
                    Image img = new Image();
                    img.Source = b;
                    img.Margin = new Thickness(5);

                    ColumnDefinition col_def = new ColumnDefinition();
                    ThumbnailPanel.ColumnDefinitions.Add(col_def);

                    Border img_border = new Border();
                    img_border.SetValue(Grid.ColumnProperty, i);
                    img_border.SetValue(Grid.RowProperty, 0);
                    img_border.CornerRadius = new CornerRadius(5);

                    StackPanel img_panel = new StackPanel();

                    Label camera_name_label = new Label();
                    camera_name_label.Content = s.Item1;
                    camera_name_label.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
                    img_panel.Children.Add(camera_name_label);
                    img.Height = 200;
                    img_panel.Children.Add(img);
                    img_border.Child = img_panel;

                    sample_images.Add(img_border);

                    ThumbnailPanel.Children.Add(img_border);

                    ComboBox resolutions = new ComboBox();
                    resolutions.Width = 80;
                    combo_boxes.Add(resolutions);

                    resolutions_all.Add(new List<Tuple<int, int>>());

                    foreach (var r in s.Item2)
                    {
                        resolutions.Items.Add(r.Item1 + "x" + r.Item2);
                        resolutions_all[resolutions_all.Count - 1].Add(new Tuple<int, int>(r.Item1, r.Item2));

                    }

                    resolutions.SelectedIndex = 0;
                    for (int res = 0; res < s.Item2.Count; ++res)
                    {
                        if (s.Item2[res].Item1 >= 640 && s.Item2[res].Item2 >= 480)
                        {
                            resolutions.SelectedIndex = res;
                            break;
                        }
                    }
                    resolutions.SetValue(Grid.ColumnProperty, i);
                    resolutions.SetValue(Grid.RowProperty, 2);
                    ThumbnailPanel.Children.Add(resolutions);

                    img_panel.MouseDown += (sender, e) =>
                    {
                        ChooseCamera(idx);
                    };

                    resolutions.DropDownOpened += (sender, e) =>
                    {
                        ChooseCamera(idx);
                    };

                });

                i++;

            }
            if (cams.Count > 0)
            {
                no_cameras_found = false;
                Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
                {
                    ChooseCamera(0);
                }));
            }
            else
            {
                string messageBoxText = "No cameras detected, please connect a webcam";
                string caption = "Camera error!";
                MessageBoxButton button = MessageBoxButton.OK;
                MessageBoxImage icon = MessageBoxImage.Warning;
                MessageBox.Show(messageBoxText, caption, button, icon);
                selected_camera_idx = -1;
                no_cameras_found = true;
                Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
                {
                    this.Close();
                }));
            }
        }

        public CameraSelection()
        {

            InitializeComponent();

            // We want to display the loading screen first
            Thread load_cameras = new Thread(LoadCameras);
            load_cameras.Start();
        }

        public void LoadCameras()
        {
            Thread.CurrentThread.IsBackground = true;
            PopulateCameraSelections();

            Dispatcher.Invoke(DispatcherPriority.Render, new TimeSpan(0, 0, 0, 0, 200), (Action)(() =>
            {
                LoadingGrid.Visibility = System.Windows.Visibility.Hidden;
                camerasPanel.Visibility = System.Windows.Visibility.Visible;
            }));
        }

        public CameraSelection(List<Tuple<String, List<Tuple<int, int>>, OpenCVWrappers.RawImage>> cams)
        {
            InitializeComponent();
            this.cams = cams;
            PopulateCameraSelections();
        }

        private void ChooseCamera(int idx)
        {
            selected_camera_idx = idx;

            foreach (var img in sample_images)
            {
                img.BorderThickness = new Thickness(1);
                img.BorderBrush = Brushes.Gray;
            }
            sample_images[idx].BorderThickness = new Thickness(4);
            sample_images[idx].BorderBrush = Brushes.Green;

        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Select();
        }

        private void CameraSelection_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                Select();
            }
        }

        private void Select()
        {
            camera_selected = true;

            int selected_res = combo_boxes[selected_camera_idx].SelectedIndex;
            Tuple<int, int> resolution_selected = resolutions_all[selected_camera_idx][selected_res];

            selected_camera = new Tuple<int, int, int>(selected_camera_idx, resolution_selected.Item1, resolution_selected.Item2);

            this.Close();
        }

        // Do not close it as user might want to open it again
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
        }

    }
}
