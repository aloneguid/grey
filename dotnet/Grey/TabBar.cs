using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Grey {

    public class TabItem : IDisposable {
        private readonly bool _rendered;

        public TabItem(string title) {
            _rendered = Native.push_next_tab(title);
        }
        public void Dispose() {
            Native.pop_next_tab();
        }

        public static implicit operator bool(TabItem ti) {
            return ti._rendered;
        }
    }

    public class TabBar : IDisposable {

        public TabBar() {
            Native.push_tab_bar("TabBar");
        }

        public void Dispose() {
            Native.pop_tab_bar();
        }
    }
}
