using System;
using System.Collections.Generic;
using System.Text;

namespace Grey {
    public class CodeEditor : IDisposable {
        private int _id = -1;
        private string? _setText;

        public CodeEditor(ProgrammingLanguage language) {
            Language = language;
        }

        public ProgrammingLanguage Language;

        public void Render() {
            _id = Native.code_editor(_id, false, (int)Language, _setText);
            _setText = null;
        }

        public string Text {
            set {
                _setText = value;
            }
        }

        public void Dispose() {
            Native.code_editor(_id, true, 0, null);
        }
    }
}
