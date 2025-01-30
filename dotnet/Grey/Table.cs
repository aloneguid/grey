using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Grey {
    public class Table : IDisposable {

        public record Column(string Name, bool Stretch = false);

        private readonly bool _rendered;

        public Table(string id, string[] columns, float outerWidth = 0, float outerHeight = 0) {
            _rendered = Native.push_table(id, columns.Length, outerWidth, outerHeight);

            if(_rendered) {
                foreach(string col in columns) {
                    Native.table_col(col);
                }

                Native.table_begin_data();
            }
        }

        public void BeginRow() {
            Native.table_begin_row();
        }

        public void BeginCol() {
            Native.table_begin_col();
        }

        public void Dispose() {
            Native.pop_table();
        }

        public static implicit operator bool(Table t) {
            return t._rendered;
        }
    }
}
