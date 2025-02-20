using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DevOats.PicoPaperLib
{
    public class PicoPaperDeviceInfo
    {
        public string Device { get; set; } = default!;
        public string Version { get; set; } = default!;
        public DisplayInfo Display { get; set; } = default!;
        public string Board { get; set; } = default!;
        public string Id { get; set; } = default!;
    }
}
