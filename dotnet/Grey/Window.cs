using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Grey {
    public class Window : IDisposable {
        private int _id = -1;
        private bool _isOpen = true;
        private GCHandle _isOpenHandle;
        private readonly string _title;
        private readonly int _initialWidth;
        private readonly int _initialHeight;

        public Window(string title, bool canClose = true, int initialWidth = 0, int initialHeight = 0) {
            _isOpenHandle = GCHandle.Alloc(_isOpen, GCHandleType.Pinned);
            _title = title;
            _initialWidth = initialWidth;
            _initialHeight = initialHeight;
        }

        public bool IsOpen {
            get => _isOpen;
            set => _isOpen = value;
        }

        public void Run(Action renderContent) {
            _id = Native.window(_id, false, _title, _initialWidth, _initialHeight, ref _isOpen, () => {
                renderContent();
            });
        }

        public void Dispose() {
            Native.window(_id, true, "", 0, 0, ref _isOpen, () => { });
            if(_isOpenHandle.IsAllocated) {
                _isOpenHandle.Free();
            }
        }
    }
}
