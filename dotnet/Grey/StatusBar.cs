using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Grey {
    public class StatusBar : IDisposable {
        public StatusBar() {
            Native.push_status_bar();
        }

        public void Dispose() {
            Native.pop_status_bar();
        }
    }
}
