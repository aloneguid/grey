using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Grey {
    static class Native {
        const string LibName = "xgrey";

        public delegate void RenderFrameCallback();

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void app_run(
            [MarshalAs(UnmanagedType.Bool)] ref bool is_running,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string title,
            int width,
            int height,
            bool has_menubar,
            RenderFrameCallback callback);

        [DllImport(LibName)]
        internal static extern void label([MarshalAs(UnmanagedType.LPUTF8Str)] string text);

        [DllImport(LibName)]
        internal static extern bool button([MarshalAs(UnmanagedType.LPUTF8Str)] string text);

        [DllImport(LibName)]
        internal static extern void sep([MarshalAs(UnmanagedType.LPUTF8Str)] string text);
    }
}
