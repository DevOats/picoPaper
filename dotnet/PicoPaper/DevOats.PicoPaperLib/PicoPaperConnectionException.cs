using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace DevOats.PicoPaperLib
{
    public class PicoPaperConnectionException : Exception
    {
        public PicoPaperConnectionException()
        {
        }

        public PicoPaperConnectionException(string? message) : base(message)
        {
        }

        public PicoPaperConnectionException(string? message, Exception? innerException) : base(message, innerException)
        {
        }

    }
}
