using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Grey {
    public class Table : IDisposable {

        private readonly bool _rendered;

        public Table(string id, int columnCount, int rowCount, float outerWidth = 0, float outerHeight = 0) {
            _rendered = Native.push_table(id, columnCount, rowCount, outerWidth, outerHeight);
        }

        public void SetupColumn(string label, bool stretch = false) {
            Native.table_col(label, stretch);
        }

        public void HeadersRow() {
            Native.table_headers_row();
        }

        public bool Step(out int displayStart, out int displayEnd) {
            int ds = 0;
            int de = 0;
            bool ret = Native.table_step(ref ds, ref de);
            displayStart = ds;
            displayEnd = de;
            return ret;
        }

        public void NextRow() {
            Native.table_next_row();
        }

        public void ToCol(int i) {
            Native.table_to_col(i);
        }

        public void Dispose() {
            Native.pop_table();
        }

        public static implicit operator bool(Table t) {
            return t._rendered;
        }
    }
}
