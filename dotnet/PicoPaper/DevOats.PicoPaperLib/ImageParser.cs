using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Net.NetworkInformation;
using System.Text;
using System.Threading.Tasks;

namespace DevOats.PicoPaperLib
{

    /// <summary>
    /// Parses a bitmap into the 1 BitPerPixel image format used by the monochrome ePaper display
    /// </summary>
    internal class ImageParser
    {
        /// <summary>
        /// A variable used by the pixel offset calculation. Taken from WaveShare reference code.
        /// </summary>
        private int widthByte;


        /// <summary>
        /// Converts the bitmap in a format that the PicoPaper can display.
        /// Currently only 1bpp Black&White is supported
        /// </summary>
        /// <param name="bmp">The bitmap image</param>
        /// <returns>A byte array that represents a 1bpp Format that the epaper can display</returns>
        public byte[] ParseBitmap(Bitmap bmp)
        {
            Color sourceColor;
            BwColors destinationColor;

            int dataSize = SetDataLength(bmp.Width, bmp.Height);
            widthByte = GetWidthByte(bmp.Width);

            byte[] result = new byte[dataSize];

            for(int x = 0; x < bmp.Width; x++)
            {
                for(int y = 0; y < bmp.Height; y++)
                {

                    sourceColor = bmp.GetPixel(x, y);

                    if((sourceColor.R == Color.White.R)
                        && (sourceColor.G == Color.White.G)
                        && (sourceColor.B == Color.White.B))
                    {
                        destinationColor = BwColors.White;
                    }
                    else
                    {
                        destinationColor = BwColors.Black;
                    }

                    SetPixel(result, x, y, destinationColor);
                }
            }

            return result;
        }


        /// <summary>
        /// Calculates the number of bytes needed for an image of the given size. Taken from WaveShare reference code
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <returns></returns>
        private int SetDataLength(int width, int height)
        {
            return ((width % 8 == 0) ? (width / 8) : (width / 8 + 1)) * height;
        }


        private void SetPixel(byte[] data, int x, int y, BwColors color)
        {
            int Addr = x / 8 + y * widthByte;

            byte Rdata = data[Addr];

            if (color == BwColors.Black)
            {
                data[Addr] = (byte)(Rdata & ~(0x80 >> (x % 8)));
            }
            else
            {
                data[Addr] = (byte)(Rdata | (0x80 >> (x % 8)));
            }
        }


        private int GetWidthByte(int width)
        {
            return (width % 8 == 0) ? (width / 8) : (width / 8 + 1);
        }

    }
}
