using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Loader.Forms
{
  public partial class ServerLoginDetailsDialog : Form
  {
    public ServerLoginDetailsDialog(string WebUsername, string WebPassword, string WebUrl)
    {
      InitializeComponent();

      hostnameLabel.Text = WebUrl;
      usernameTextBox.Text = WebUsername;
      passwordTextBox.Text = WebPassword;
    }

    private void OnLinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
    {
      Process.Start(new ProcessStartInfo(hostnameLabel.Text) { UseShellExecute = true });
    }

    private void submitButton_Click(object sender, EventArgs e)
    {
      Close();
    }
  }
}
