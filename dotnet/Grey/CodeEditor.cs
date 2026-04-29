using System;
using System.Collections.Generic;
using System.Text;

namespace Grey {
    public class CodeEditor : IDisposable {
        private readonly int _id;

        public CodeEditor(ProgrammingLanguage language) {
            _id = Native.code_editor_register((int)language);
        }

        public void Render() {
            Native.code_editor_render(_id);
        }

        public string Text {
            set {
                Native.code_editor_set_text(_id, value);
            }
        }

        public void Dispose() {
            if(!Native.code_editor_unregister(_id)) {
                throw new InvalidOperationException($"Failed to deregister code editor#{_id}");
            }
        }
    }
}
