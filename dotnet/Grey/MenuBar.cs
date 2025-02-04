using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Grey {
    public class MenuBar : IDisposable {
        private readonly bool _rendered;

        public class Menu : IDisposable {
            private readonly bool _rendered;

            public Menu(string title) {
                _rendered = Native.push_menu(title);
            }

            public static implicit operator bool(Menu m) {
                return m._rendered;
            }

            public void Dispose() {
                Native.pop_menu();
            }

            public bool Item(string text, bool reserveIconSpace = true, string icon = "") {
                return Native.menu_item(text, reserveIconSpace, icon);
            }
        }

        public MenuBar() {
            _rendered = Native.push_menu_bar();
        }

        public static implicit operator bool(MenuBar mb) {
            return mb._rendered;
        }

        public void Dispose() {
            Native.pop_menu_bar();
        }

        public Menu M(string title) {
            return new Menu(title);
        }
    }
}
