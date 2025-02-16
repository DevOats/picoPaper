using DevOats.PicoPaperLib;
using System.Drawing;

namespace DevOats.PicoPaperCmd
{
    internal class Program
    {
        static void Main(string[] args)
        {
            bool doRun = true;
            PicoPaperDevice paper = new PicoPaperDevice();

            paper.Connect("com4");


            while (doRun)
            {
                Console.WriteLine();
                Console.WriteLine();
                Console.WriteLine(" i  -  Ident");
                Console.WriteLine(" c  -  Clear Display");
                Console.WriteLine(" s  -  Show Splash screen");
                Console.WriteLine(" d  -  transmit and display test image");
                Console.WriteLine(" r  -  Reset communication protocol");
                Console.WriteLine();
                Console.WriteLine();
                Console.WriteLine(" x  -  Exit");

                char read = Console.ReadKey().KeyChar;

                

                switch (read)
                {
                    case 'i':
                        paper.Ident();
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
