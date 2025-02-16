using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace DevOats.PicoPaperLib
{

    /// <summary>
    /// Represents and handles the connection to PicoPaper device over the serial port and the low-level protocol
    /// </summary>
    internal class SerialPortConnector
    {

        private char DatabyteStartChar = ':';
        private char ProtocolResetChar = '/';

        private const int DefaultBaudrate = 115200;
        private const Parity DefaultParity = Parity.None;
        private const int DefaultDataBits = 8;
        private const StopBits DefaultStopBits = StopBits.One;
        private const Handshake DefaultHandshake = Handshake.None;

        private SerialPort serialPort = new SerialPort();
        private Thread readerThread;


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
        /// Connects to the PicoPaper device over the specified serial port
        /// </summary>
        /// <param name="portName">The port name (e.g. "com4")</param>
        /// <exception cref="PicoPaperConnectionException"></exception>
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
                throw new PicoPaperConnectionException($"Could not open the serial port to the PicoPaper device. Error: {ex.Message}", ex);
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
                try
                {
                    char message = (char)serialPort.ReadChar();
                    Console.Write(message);
                }
                catch (TimeoutException) { }    // Is to be expected every 500ms because we told it to
                catch (OperationCanceledException) { }  // Gets thrown when we close the serial port
            }
        }

    }
}
