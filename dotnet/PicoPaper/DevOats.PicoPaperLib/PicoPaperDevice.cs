using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DevOats.PicoPaperLib
{

    /// <summary>
    /// Represents the PicoPaper device and implements features at a functional level
    /// </summary>
    public class PicoPaperDevice
    {

        private SerialPortConnector connection = new SerialPortConnector();

        /// <summary>
        /// Gets whether the serial port is connected
        /// </summary>
        public bool IsConnected
        {
            get
            {
                return connection.IsConnected;
            }
        }

        /// <summary>
        /// Open the connection to the PicoPaper device on the specified COM port
        /// </summary>
        /// <param name="portName">The comport name (e.g. "com4")</param>
        public void Connect(string portName)
        {
            connection.Connect(portName);
            connection.ResetCommProtocol();
        }


        /// <summary>
        /// Requests identification information from the device
        /// </summary>
        public void Ident()
        {
            connection.SendDataByte(PicoPaperCommands.Ident);
        }


        /// <summary>
        /// Shows the device splash screen on the ePaper
        /// </summary>
        public void ShowSplash()
        {
            connection.SendDataByte(PicoPaperCommands.ShowSplashScreen);
        }


        /// <summary>
        /// Clears the ePaper display
        /// </summary>
        public void ClearDisplay()
        {
            connection.SendDataByte(PicoPaperCommands.ClearDisplay);
        }


        /// <summary>
        /// Resets the communication protocol
        /// </summary>
        public void ResetCommProtocol()
        {
            connection.ResetCommProtocol();
        }


        /// <summary>
        /// Gracefully disconnects fromt the device by first clearing the display and then closing the serial port
        /// </summary>
        public void Disconnect()
        {
            // Clear the screen to prevent damage during long term storage
            // ToDo: Decide whether this should really be here :/
            ClearDisplay();
            Thread.Sleep(500);
            connection.Disconnect();
        }


        /// <summary>
        /// Displays a bitmap image on the ePaper display (Currently only 800 x 480 is supported)
        /// </summary>
        /// <param name="image">The image to be displayed</param>
        public void DisplayBitmap(Bitmap image)
        {

            if((image.Width != 800) || (image.Height != 480))
            {
                throw new ArgumentException("Unsupported image dimensions. Only 800 x 480 is supported");
            }

            ImageParser parser = new ImageParser();
            byte[] imgData = parser.ParseBitmap(image);

            connection.SendDataByte(PicoPaperCommands.StartImageTx);
            connection.SendDataBytes(imgData);
            connection.SendDataByte(PicoPaperCommands.DisplayImageBuffer);
        }
    }
}
