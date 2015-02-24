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
    /// Interaction logic for Liability.xaml
    /// </summary>
    public partial class Liability : Window
    {
        public bool continue_pressed = false;

        public Liability()
        {
            InitializeComponent();

            this.KeyDown += new KeyEventHandler(TextEntry_KeyDown);
            FocusManager.SetFocusedElement(this, ContinueButton);
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            continue_pressed = true;
            this.Close();
        }

        private void TextEntry_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                continue_pressed = true;
                DialogResult = true;
            }
        }

    }
}
