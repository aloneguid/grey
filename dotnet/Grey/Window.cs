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

        public Window(string title, bool canClose = true) {
            _isOpenHandle = GCHandle.Alloc(_isOpen, GCHandleType.Pinned);
            _title = title;
        }

        public bool IsOpen {
            get => _isOpen;
            set => _isOpen = value;
        }

        public void Run(Action renderContent) {
            _id = Native.window(_id, false, _title, ref _isOpen, () => {
                renderContent();
            });
        }

        public void Dispose() {
            Native.window(_id, true, "", ref _isOpen, () => { });
            if(_isOpenHandle.IsAllocated) {
                _isOpenHandle.Free();
            }
        }
    }
}
