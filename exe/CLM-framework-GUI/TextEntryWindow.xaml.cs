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

namespace CLM_framework_GUI
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

        public string ResponsePortText
        {
            get { return ResponsePortTextBox.Text; }
            set { ResponsePortTextBox.Text = value; }
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
