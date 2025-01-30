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
            bool can_scroll,
            bool center_on_screen,
            RenderFrameCallback callback);

        [DllImport(LibName)]
        internal static extern void sl(float offset);

        [DllImport(LibName)]
        internal static extern void label([MarshalAs(UnmanagedType.LPUTF8Str)] string text,
            Emphasis emphasis, int text_wrap_pos, bool enabled);

        [DllImport(LibName)]
        internal static extern bool button(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string text,
            Emphasis emphasis,
            bool is_enabled,
            bool is_small);

        [DllImport(LibName)]
        internal static extern void sep([MarshalAs(UnmanagedType.LPUTF8Str)] string text);

        [DllImport(LibName)]
        internal static extern bool accordion([MarshalAs(UnmanagedType.LPUTF8Str)] string header, bool default_open);

        [DllImport(LibName)]
        internal static extern bool hyperlink(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string text,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string? url_to_open);

        [DllImport(LibName)]
        internal static extern void notify([MarshalAs(UnmanagedType.LPUTF8Str)] string message);

        [DllImport(LibName)]
        internal static extern bool input(
            [MarshalAs(UnmanagedType.LPUTF8Str)] StringBuilder value,
            int value_max_length,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            bool enabled, float width, bool is_readonly);


        // -- tabs

        [DllImport(LibName)]
        internal static extern bool push_tab_bar([MarshalAs(UnmanagedType.LPUTF8Str)] string id);

        [DllImport(LibName)]
        internal static extern void pop_tab_bar();

        [DllImport(LibName)]
        internal static extern bool push_next_tab([MarshalAs(UnmanagedType.LPUTF8Str)] string title);

        [DllImport(LibName)]
        internal static extern void pop_next_tab();

        // -- tables

        [DllImport(LibName)]
        internal static extern bool push_table([MarshalAs(UnmanagedType.LPUTF8Str)] string id,
            int column_count,
            int row_count,
            float outer_width,
            float outer_height);

        [DllImport(LibName)]
        internal static extern void pop_table();

        [DllImport(LibName)]
        internal static extern void table_col(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            bool is_stretch);

        [DllImport(LibName)]
        internal static extern void table_headers_row();

        [DllImport(LibName)]
        internal static extern bool table_step(ref int display_start, ref int display_end);

        [DllImport(LibName)]
        internal static extern void table_next_row();

        [DllImport(LibName)]
        internal static extern void table_to_col(int i);
    }
}
