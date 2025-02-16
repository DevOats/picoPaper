using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DevOats.PicoPaperLib
{
    public class DeviceResponse
    {
        public string Message { get; private set; }

        public ResponseTypes ResponseType { get; private set; }

        public DeviceResponse(string message, ResponseTypes responseType)
        {
            Message = message;
            ResponseType = responseType;
        }
    }
}
