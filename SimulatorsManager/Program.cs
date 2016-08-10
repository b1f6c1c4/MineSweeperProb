using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace SimulatorsManager
{
    internal static class Program
    {
        [DllImport("user32.dll")]
        private static extern bool SetProcessDPIAware();

        /// <summary>
        ///     应用程序的主入口点。
        /// </summary>
        [STAThread]
        private static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            //SetProcessDPIAware();

            Application.Run(new frmMain());
        }
    }
}
