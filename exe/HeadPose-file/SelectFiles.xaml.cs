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

namespace HeadPoseFile
{
    /// <summary>
    /// Interaction logic for SelectFiles.xaml
    /// </summary>
    public partial class SelectFiles : Window
    {
        public SelectFiles()
        {
            InitializeComponent();
        }

        public bool record_from_image;

        public bool RecordHeadPose
        {
            get { return (bool)RecordHeadPoseCheckBox.IsChecked; }
            set { RecordHeadPoseCheckBox.IsChecked = value; }
        }

        public bool RecordLocationFile
        {
            get { return (bool)RecordFileLoc.IsChecked; }
            set { RecordFileLoc.IsChecked = value; }
        }

        public bool RecordLocationRecDir
        {
            get { return (bool)RecordRecDirLoc.IsChecked; }
            set { RecordRecDirLoc.IsChecked = value; }
        }

        public string[] files_chosen;

        private void OpenFileButton_Click(object sender, System.Windows.RoutedEventArgs e)
        {

            var d = new Microsoft.Win32.OpenFileDialog();
            d.Multiselect = true;
            
            if (sender == OpenVideoButton)
            {
                d.Filter = "Video files|*.avi;*.wmv;*.mov;*.mpg;*.mpeg";
                record_from_image = false;
            }
            else if (sender == OpenImageButton)
            {
                d.Filter = "Image files|*.jpg;*.jpeg;*.bmp;*.png;*.gif";
                record_from_image = true;
            }

            if (d.ShowDialog(this) == true)
            {

                files_chosen = d.FileNames;
                DialogResult = true;

            }

        }

        private void RecordHeadPoseCheckBox_Click(object sender, RoutedEventArgs e)
        {
            if (RecordHeadPose)
            {
                RecordRecDirLoc.IsEnabled = true;
                RecordFileLoc.IsEnabled = true;
            }
            else
            {
                RecordRecDirLoc.IsEnabled = false;
                RecordFileLoc.IsEnabled = false;
            }
        }


    }
}
