using DevOats.PicoPaperLib;
using PowerArgs;
using System.Drawing;

namespace DevOats.PicoPaper
{

    internal class ApplicationArgsRunner
    {

        [ArgDescription("Shows help")]
        [ArgShortcut("-?")]
        [HelpHook()]
        public bool Help { get; set; }


        [ArgActionMethod]
        [ArgDescription("Clears the PicoPaper display")]
        [ArgShortcut("-c")]
        public void ClearDisplay([ArgRequired] [ArgDescription("The name of the serial port (e.g. com4)")]  string port)
        {
            PrintSplashScreen("Clearing the display");
            PicoPaperDevice device = ConnectToPicoPaper(port);
            device.ClearDisplay();
            Disconnect(device);
            Console.WriteLine("Done");
        }


        [ArgActionMethod]
        [ArgDescription("Shows the splash screen on the PicoPaper display")]
        [ArgShortcut("-s")]
        public void ShowSplash([ArgRequired] [ArgDescription("The name of the serial port (e.g. com4)")] string port)
        {
            PrintSplashScreen("Showing splash screen");
            PicoPaperDevice device = ConnectToPicoPaper(port);
            device.ShowSplash();
            Disconnect(device);
            Console.WriteLine("Done");
        }


        [ArgActionMethod]
        [ArgDescription("Retrieves identification information from the PicoPaper device")]
        [ArgShortcut("-i")]
        public void Identify([ArgRequired] [ArgDescription("The name of the serial port (e.g. com4)")] string port)
        {
            PrintSplashScreen("Retrieving device information");
            PicoPaperDevice device = ConnectToPicoPaper(port);
            PicoPaperDeviceInfo info = device.Ident();
            PicoPaperCmd.Program.PrintDeviceInfo(info);
            Disconnect(device);
            Console.WriteLine("Done");
        }

        
        [ArgActionMethod]
        [ArgDescription("Uploads a bitmap to the PicoPaper device and displays it")]
        [ArgShortcut("-d")]
        public void DisplayBitmap(
            [ArgRequired] [ArgDescription("The name of the serial port (e.g. com4)")] string port,
            [ArgRequired] [ArgDescription("The path to the bitmap file")] string bitmapPath)
        {
            PrintSplashScreen("Uploading image");
            PicoPaperDevice device = ConnectToPicoPaper(port);
            Bitmap bmp = new(bitmapPath);
            device.DisplayBitmap(bmp);
            Disconnect(device);
            Console.WriteLine("Done");
        }


        private PicoPaperDevice ConnectToPicoPaper(string comPort)
        {
            PicoPaperDevice picoPaper = new PicoPaperDevice();
            picoPaper.Connect(comPort);
            return picoPaper;
        }


        private void Disconnect(PicoPaperDevice picoPaper)
        {
            picoPaper.Disconnect(false);
        }


        private void PrintSplashScreen(string action)
        {
            Console.WriteLine("PicoPaper Utility");
            Console.WriteLine("(c) 2005: DevOats");
            Console.WriteLine("");
            Console.WriteLine(action);
        }


    }
}
