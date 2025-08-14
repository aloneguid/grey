using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Grey {
    static class Native {
        const string LibName = "xgrey";

        public delegate bool RenderFrameCallback();

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void app_run(
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
        internal static extern bool checkbox([MarshalAs(UnmanagedType.LPUTF8Str)] string label, ref bool is_checked);

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
        internal static extern bool input_string(
            [MarshalAs(UnmanagedType.LPUTF8Str)] StringBuilder value,
            int value_max_length,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            bool enabled, float width, bool is_readonly);

        [DllImport(LibName)]
        internal static extern bool input_int(
            ref int value,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            bool enabled, float width, bool is_readonly);

        [DllImport(LibName)]
        internal static extern bool input_multiline(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string id,
            [MarshalAs(UnmanagedType.LPUTF8Str)] StringBuilder value,
            int value_max_length,
            float height,
            bool autoscroll,
            bool enabled,
            bool use_fixed_font);

        [DllImport(LibName)]
        internal static extern void spinner_hbo_dots(float radius, float thickness, float speed, int dot_count);

        [DllImport(LibName)]
        internal static extern void slider_float(ref float value, float min, float max,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label);

        [DllImport(LibName)]
        internal static extern void slider_int(ref int value, int min, int max,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label);

        [DllImport(LibName)]
        internal static extern void tooltip([MarshalAs(UnmanagedType.LPUTF8Str)] string text);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void RenderTableCellCallback(int row, int col);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void table(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string id,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)] string[] columns,
            int columns_size,
            int row_count,
            float outer_width,
            float outer_height,
            bool alternate_row_bg,
            RenderTableCellCallback cell_callback);


        // -- tabs

        [DllImport(LibName)]
        internal static extern bool push_tab_bar([MarshalAs(UnmanagedType.LPUTF8Str)] string id);

        [DllImport(LibName)]
        internal static extern void pop_tab_bar();

        [DllImport(LibName)]
        internal static extern bool push_next_tab([MarshalAs(UnmanagedType.LPUTF8Str)] string title);

        [DllImport(LibName)]
        internal static extern void pop_next_tab();

        // -- status bar

        [DllImport(LibName)]
        internal static extern void push_status_bar();

        [DllImport(LibName)]
        internal static extern void pop_status_bar();

        // application menus

        [DllImport(LibName)]
        internal static extern bool push_menu_bar();

        [DllImport(LibName)]
        internal static extern void pop_menu_bar();

        [DllImport(LibName)]
        internal static extern bool push_menu([MarshalAs(UnmanagedType.LPUTF8Str)] string label);

        [DllImport(LibName)]
        internal static extern void pop_menu();

        [DllImport(LibName)]
        internal static extern bool menu_item(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string c_text,
            bool reserve_icon_space,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string c_icon);

        // system debug

        [DllImport(LibName)]
        internal static extern void label_fps();

    }
}
