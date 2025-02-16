using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DevOats.PicoPaperLib
{
    /// <summary>
    /// Contains commands that can be issues to the PicoPaper device
    /// </summary>
    internal static class PicoPaperCommands
    {
        /// <summary>
        /// Requests device identification
        /// </summary>
        public const byte Ident = 0x01;

        /// <summary>
        /// Start transferring image data to the image buffer
        /// </summary>
        public const byte StartImageTx = 0x02;

        /// <summary>
        /// Displays the image that is present in the image buffer
        /// </summary>
        public const byte DisplayImageBuffer = 0x03;

        /// <summary>
        /// Clear the ePaper display
        /// </summary>
        public const byte ClearDisplay = 0x04;

        /// <summary>
        /// Show the device splash screen
        /// </summary>
        public const byte ShowSplashScreen = 0x05;

    }
}
