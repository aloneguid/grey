using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Grey {
    public class Window : IDisposable {
        private readonly int _id;
        private bool _isOpen = true;

        public Window(string title, bool canClose = true) {

            _id = Native.window_register(title, ref _isOpen);
        }

        public bool IsOpen {
            get => _isOpen;
            set => _isOpen = value;
        }

        public void Frame(Action renderContent) {
            Native.window_render(_id, () => {
                renderContent();
            });
        }

        public void Dispose() {
            if(!Native.window_unregister(_id)) {
                throw new InvalidOperationException($"Failed to deregister window#{_id}");
            }
        }
    }
}
