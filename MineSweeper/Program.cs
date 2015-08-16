using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace MineSweeper
{
    internal static class Program
    {

        [DllImport("user32.dll")]
        private static extern bool SetProcessDPIAware();

        [DllImport("gdi32.dll")]
        private static extern int GetDeviceCaps(IntPtr hdc, int nIndex);

        [DllImport("user32.dll")]
        private static extern IntPtr GetDC(IntPtr hWnd);

        [DllImport("user32.dll")]
        // ReSharper disable once InconsistentNaming
        private static extern bool ReleaseDC(IntPtr hWnd, IntPtr hDC);

        public static float ScaleFactor;

        /// <summary>
        ///     应用程序的主入口点。
        /// </summary>
        [STAThread]
        private static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            SetProcessDPIAware();

            var hdc = GetDC(IntPtr.Zero);
            ScaleFactor = GetDeviceCaps(hdc, 88) / 96F;
            ReleaseDC(IntPtr.Zero, hdc);

            Application.Run(new Configuration());
        }
    }
}
