using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Grey {
    public enum Emphasis : int {
        None = 0,
        Primary = 1,
        Secondary = 2,
        Success = 3,
        Error = 4,
        Warning = 5,
        Info = 6
    };

    public enum ShowDelay : int {
        Immediate = 0,
        Quick,
        Normal,
        Slow
    };
}
