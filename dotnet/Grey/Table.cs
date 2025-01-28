using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Grey {
    public class Table : IDisposable {

        public record Column(string Name, bool Stretch = false);

        private readonly bool _rendered;
        private int _col;

        public Table(string id, Column[] columns, int rowCount, float outerWidth = 0, float outerHeight = 0) {
            _rendered = Native.push_table(id, columns.Length, rowCount, outerWidth, outerHeight);

            if(_rendered) {
                foreach(Column col in columns) {
                    Native.table_col(col.Name ?? "", col.Stretch);
                }

                Native.table_headers_row();
            }
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
            _col = 0;
            Native.table_to_col(_col);
        }

        public void NextCol() {
            _col++;
            Native.table_to_col(_col);
        }

        public void Dispose() {
            Native.pop_table();
        }

        public static implicit operator bool(Table t) {
            return t._rendered;
        }
    }
}
