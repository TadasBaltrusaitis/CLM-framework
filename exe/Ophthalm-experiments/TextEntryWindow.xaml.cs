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

namespace Ophthalm_experiments
{
    /// <summary>
    /// Interaction logic for TextEntryWindow.xaml
    /// </summary>
    public partial class TextEntryWindow : Window
    {
        public TextEntryWindow()
        {
            InitializeComponent();
            
            this.KeyDown += new KeyEventHandler(TextEntry_KeyDown);
            
        }

        public string ResponseText
        {
            get { return ResponseTextBox.Text; }
            set { ResponseTextBox.Text = value; }
        }

        public bool RecordVideo
        {
            get { return (bool)RecordVideoCheckBox.IsChecked; }
            set { RecordVideoCheckBox.IsChecked = value; }
        }

        public bool RecordHeadPose
        {
            get { return (bool)RecordHeadPoseCheckBox.IsChecked; }
            set { RecordHeadPoseCheckBox.IsChecked = value; }
        }


        private void OKButton_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private void TextEntry_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                DialogResult = true;
            }
        }

    }
}
