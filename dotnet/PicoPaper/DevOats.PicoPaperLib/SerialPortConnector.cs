using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace DevOats.PicoPaperLib
{

    public delegate void ResponseReceivedDelegate(DeviceResponse data);

    /// <summary>
    /// Represents and handles the connection to PicoPaper device over the serial port and the low-level protocol
    /// </summary>
    internal class SerialPortConnector
    {

        private const string responseMsgAckStart = "~ACK#";
        private const string responseMsgErrorStart = "~ERR#";
        private const string responseMsgEnd = "^";

        private char DatabyteStartChar = ':';
        private char ProtocolResetChar = '/';

        private const int DefaultBaudrate = 115200;
        private const Parity DefaultParity = Parity.None;
        private const int DefaultDataBits = 8;
        private const StopBits DefaultStopBits = StopBits.One;
        private const Handshake DefaultHandshake = Handshake.None;

        private SerialPort serialPort = new SerialPort();
        private Thread readerThread;

        private ResponseReceivedDelegate? responseReceivedHandler = null;

        /// <summary>
        /// Gets whether the serial port is open
        /// </summary>
        public bool IsConnected { 
            get
            {
                return serialPort.IsOpen;
            }
        }


        /// <summary>
        /// Constructor
        /// </summary>
        public SerialPortConnector()
        {
            readerThread = new Thread(ReadThreadMethod);
            readerThread.IsBackground = true;
            readerThread.Name = "SerialPortReader";
        }


        /// <summary>
        /// Sets the handler for when a response is received
        /// </summary>
        /// <param name="handler"></param>
        public void SetOnResponseReceived(ResponseReceivedDelegate handler)
        {
            responseReceivedHandler = handler;
        }


        /// <summary>
        /// Connects to the PicoPaper device over the specified serial port
        /// </summary>
        /// <param name="portName">The port name (e.g. "com4")</param>
        /// <exception cref="PicoPaperException"></exception>
        public void Connect(string portName)
        {
            try
            {
                serialPort.PortName = portName;
                serialPort.BaudRate = DefaultBaudrate;
                serialPort.Parity = DefaultParity;
                serialPort.DataBits = DefaultDataBits;
                serialPort.StopBits = DefaultStopBits;
                serialPort.Handshake = DefaultHandshake;
                serialPort.DtrEnable = true;
                serialPort.RtsEnable = true;
                serialPort.ReadTimeout = 500;
                serialPort.WriteTimeout = 500;
                serialPort.Open();
                
                readerThread.Start();
            }
            catch(Exception ex)
            {
                throw new PicoPaperException($"Could not open the serial port to the PicoPaper device. Error: {ex.Message}", ex);
            }
        }


        /// <summary>
        /// Closes the connection to the serial port
        /// </summary>
        public void Disconnect()
        {
            serialPort.Close();
        }


        /// <summary>
        /// Sends the protocol reset character
        /// </summary>
        public void ResetCommProtocol()
        {
            serialPort.Write($"{ProtocolResetChar}");
        }


        /// <summary>
        /// Sends a single databyte to the PicoPaper device
        /// </summary>
        /// <param name="data"></param>
        public void SendDataByte(byte data)
        {
            StringBuilder builder = new StringBuilder();
            builder.Append(DatabyteStartChar);
            builder.AppendFormat("{0:x2}", data);

            string message = builder.ToString();
            serialPort.Write(message);
        }


        /// <summary>
        /// Sends the specified data as DataBytes to the PicoPaper device
        /// </summary>
        /// <param name="data"></param>
        public void SendDataBytes(byte[] data)
        {
            foreach(byte dataByte in data)
            {
                SendDataByte(dataByte);
            }
        }


        private void ReadThreadMethod()
        {
            while (serialPort.IsOpen)
            {
                string rx = String.Empty;
                try
                {
                    rx = serialPort.ReadLine();
                    
                }
                catch (TimeoutException) { }    // Is to be expected every 500ms because we told it to
                catch (OperationCanceledException) { }  // Gets thrown when we close the serial port

                if (rx != String.Empty)
                {
                    ProcessReceivedMessage(rx);
                }
            }
        }


        private void ProcessReceivedMessage(string rawMessage)
        {
            ResponseTypes responseType;
            string message = rawMessage;
            
            message = message.Trim();

            if (message.StartsWith(responseMsgAckStart) && message.EndsWith(responseMsgEnd))
            {
                responseType = ResponseTypes.Ack;
                message = message.Remove(0, responseMsgAckStart.Length);
                message = message.Remove(message.Length - responseMsgEnd.Length);
            }
            else if (message.StartsWith(responseMsgErrorStart) && message.EndsWith(responseMsgEnd))
            {
                responseType = ResponseTypes.Error;
                message = message.Remove(0, responseMsgErrorStart.Length);
                message = message.Remove(message.Length - responseMsgEnd.Length);
            }
            else
            {
                responseType = ResponseTypes.Invalid;
            }

            message = message.Trim();

            DeviceResponse response = new DeviceResponse(message, responseType);
            responseReceivedHandler?.Invoke(response);
        }

    }
}
