using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace DevOats.PicoPaperLib
{

    /// <summary>
    /// Represents the PicoPaper device and implements features at a functional level
    /// </summary>
    public class PicoPaperDevice
    {

        private AutoResetEvent responseWaitSignaller = new AutoResetEvent(false);
        private SerialPortConnector connection = new SerialPortConnector();
        private volatile DeviceResponse? lastResponse;

        private string AckMessageImageReceived = "IMG_RCVD";
        private string AckMessageClearDisplay = "CLR_SCR";
        private string AckMessageSplashScreen = "SPLASH";
        private string AckMessageBufferDisplayed = "DISPLAY";

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

        public PicoPaperDevice()
        {
            connection.SetOnResponseReceived(OnResponseReceived);
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
        public PicoPaperDeviceInfo Ident()
        {
            connection.SendDataByte(PicoPaperCommands.Ident);
            DeviceResponse response = WaitForResponse();

            PicoPaperDeviceInfo identInfo = ParseIdentInfo(response.Message);
            return identInfo;
        }


        /// <summary>
        /// Shows the device splash screen on the ePaper
        /// </summary>
        public void ShowSplash()
        {
            connection.SendDataByte(PicoPaperCommands.ShowSplashScreen);
            DeviceResponse response = WaitForResponse();

            ValidateAck(response, AckMessageSplashScreen);
        }


        /// <summary>
        /// Clears the ePaper display
        /// </summary>
        public void ClearDisplay()
        {
            connection.SendDataByte(PicoPaperCommands.ClearDisplay);
            DeviceResponse response = WaitForResponse();
            ValidateAck(response, AckMessageClearDisplay);
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
            connection.SendDataByte(PicoPaperCommands.ClearDisplay);    // Send the instruction without waiting for the response
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
            DeviceResponse response = WaitForResponse();
            ValidateAck(response, AckMessageImageReceived);

            connection.SendDataByte(PicoPaperCommands.DisplayImageBuffer);
            response = WaitForResponse();
            ValidateAck(response, AckMessageBufferDisplayed);

        }


        private void ValidateAck(DeviceResponse response, string ackMessage)
        {
            if(response.ResponseType == ResponseTypes.Error)
            {
                throw new PicoPaperException($"PicoPaper device error: {response.Message}");
            }

            if ((response.ResponseType != ResponseTypes.Ack) || (response.Message != ackMessage))
            {
                throw new PicoPaperException($"Unexpected device message received: {response.Message}");
            }
        }


        private void OnResponseReceived(DeviceResponse response)
        {
            lastResponse = response;
            responseWaitSignaller.Set();
        }


        private DeviceResponse WaitForResponse()
        {
            responseWaitSignaller.WaitOne(20000);
            DeviceResponse? response = lastResponse;
            lastResponse = null;

            if (response == null)
            {
                throw new ArgumentNullException("lastResponse", "No response received from device");
            }
            else
            {
                return response;
            }
        }


        private PicoPaperDeviceInfo ParseIdentInfo(string message)
        {
            PicoPaperDeviceInfo? info;

            JsonSerializerOptions serializeroptions = new()
            {
                PropertyNameCaseInsensitive = true
            };

            try
            {
                info = JsonSerializer.Deserialize<PicoPaperDeviceInfo>(message, serializeroptions);
            }
            catch(Exception ex)
            {
                throw new PicoPaperException($"Could not deserialize device info: {ex.Message}", ex);
            }
                
            if (info == null)
            {
                throw new PicoPaperException("Could not deserialize device info");
            }
            else
            {
                return info;
            }
            
        }
    }
}
