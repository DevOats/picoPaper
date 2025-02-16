using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace DevOats.PicoPaperLib
{
    public class PicoPaperException : Exception
    {
        public PicoPaperException()
        {
        }

        public PicoPaperException(string? message) : base(message)
        {
        }

        public PicoPaperException(string? message, Exception? innerException) : base(message, innerException)
        {
        }

    }
}
