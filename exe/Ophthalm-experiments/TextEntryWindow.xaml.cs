using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
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

        // Do not allow illegal characters like
        private void ResponseTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            Regex regex = new Regex("[/:*?<>|\"]");
            Regex regex2 = new Regex(@"[\\]");
            MatchCollection matches = regex.Matches(ResponseTextBox.Text);
            MatchCollection matches2 = regex2.Matches(ResponseTextBox.Text);
            if (matches.Count > 0 || matches2.Count > 0)
            {
                for (int i = matches.Count - 1; i >= 0; --i)
                {
                    // Remove the illegal characters
                    ResponseTextBox.Text = ResponseTextBox.Text.Substring(0, matches[i].Index) + ResponseTextBox.Text.Substring(matches[i].Index + 1);
                }

                //tell the user
                for (int i = matches2.Count - 1; i >= 0; --i)
                {
                    // Remove the illegal characters
                    ResponseTextBox.Text = ResponseTextBox.Text.Substring(0, matches2[i].Index) + ResponseTextBox.Text.Substring(matches2[i].Index + 1);
                    
                }
                warningLabel.Visibility = System.Windows.Visibility.Visible;
                ResponseTextBox.SelectionStart = ResponseTextBox.Text.Length;
            }
            else
            {
                warningLabel.Visibility = System.Windows.Visibility.Collapsed;
            }
        }

    }
}
