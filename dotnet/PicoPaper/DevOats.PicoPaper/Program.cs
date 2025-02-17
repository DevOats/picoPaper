using DevOats.PicoPaper;
using DevOats.PicoPaperLib;
using PowerArgs;
using System.Drawing;

namespace DevOats.PicoPaperCmd
{
    internal class Program
    {
        static void Main(string[] args)
        {

            if (args.Length == 0)
            {
                RunInteractiveMode();
            }
            else
            {
                RunAsCmdLineTool(args);
            }
        }


        private static void RunAsCmdLineTool(string[] args)
        {
            try
            {
                var arguments = Args.InvokeAction<ApplicationArgsRunner>(args);
            }
            catch (ArgException e)
            {
                Console.WriteLine(e.Message);
                Console.WriteLine(ArgUsage.GenerateUsageFromTemplate<ApplicationArgsRunner>());
            }
        }


        private static void RunInteractiveMode()
        {
            bool doRun = true;

            Console.WriteLine("");
            Console.WriteLine("PicoPaper utility");
            Console.WriteLine("(c) 2025: DevOats");
            Console.WriteLine("");
            Console.Write("COM port number to connect to: ");
            string? portNumberString = Console.ReadLine();

            int portNumber = 0;

            try
            {
                if (portNumberString != null)
                {
                    portNumber = Int32.Parse(portNumberString);
                }
                else
                {
                    Console.WriteLine($"Invalid COM port number '{portNumberString}'");
                    return;
                }
            }
            catch (FormatException ex) 
            {
                Console.WriteLine($"Invalid COM port number '{portNumberString}'. {ex.Message}", ex);
                return;
            }

            string comPort = $"com{portNumber}";

            PicoPaperDevice paper = new PicoPaperDevice();

            paper.Connect(comPort);


            while (doRun)
            {
                Console.WriteLine();
                Console.WriteLine();
                Console.WriteLine(" i  -  Ident");
                Console.WriteLine(" c  -  Clear Display");
                Console.WriteLine(" s  -  Show Splash screen");
                Console.WriteLine(" d  -  Transmit and display test image");
                Console.WriteLine(" r  -  Reset communication protocol");
                Console.WriteLine();
                Console.WriteLine();
                Console.WriteLine(" x  -  Exit");

                char read = Console.ReadKey().KeyChar;

                switch (read)
                {
                    case 'i':
                        PicoPaperDeviceInfo info = paper.Ident();
                        PrintDeviceInfo(info);
                        break;
                    case 'c':
                        paper.ClearDisplay();
                        break;
                    case 's':
                        paper.ShowSplash();
                        break;
                    case 'd':
                        paper.DisplayBitmap(CreateTestBitmap());
                        break;
                    case 'r':
                        paper.ResetCommProtocol();
                        break;
                    case 'x':
                        doRun = false;
                        break;

                }
            }

            paper.Disconnect();
        }

        public static void PrintDeviceInfo(PicoPaperDeviceInfo info)
        {
            Console.WriteLine();
            Console.WriteLine("Device info:");
            Console.WriteLine($"   Device:    {info.Device}    ");
            Console.WriteLine($"   Version:   {info.Version}   ");
            Console.WriteLine($"   Display:                    ");
            Console.WriteLine($"              Type:        {info.Display.Type}");
            Console.WriteLine($"              Size:        {info.Display.Size}");
            Console.WriteLine($"              Resolution:      ");
            Console.WriteLine($"                           Width:   {info.Display.Resolution.Width}");
            Console.WriteLine($"                           Height:  {info.Display.Resolution.Height}");
            Console.WriteLine($"              Color:       {info.Display.Color}");
            Console.WriteLine($"              Format:      {info.Display.Format}");
        }


        private static Bitmap CreateTestBitmap()
        {
            Bitmap bmp = new(800, 480);

            Graphics gr = Graphics.FromImage(bmp);
            gr.Clear(Color.White);

            gr.DrawRectangle(Pens.Black, 1, 1, 798, 478);

            gr.DrawEllipse(Pens.Black, 400, 200, 200, 200);

            gr.DrawLine(Pens.Black, 500, 200, 500, 400);
            gr.DrawLine(Pens.Black, 400, 300, 600, 300);

            gr.FillEllipse(Brushes.Black, 200, 400, 50, 50);

            Font font = new Font("Times New Roman", 30.0f);

            gr.DrawString("It works :) !!", font, Brushes.Black, 400, 50);

            gr.FillRectangle(Brushes.Black, 50, 50, 200, 200);

            Pen thickWhite = new Pen(Color.White, 3);
            gr.DrawLine(thickWhite, 50, 50, 250, 250);

            return bmp;
        }
    }
}
