using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Grey {
    
    static class Native {
        const string _libName = "xgrey";

        public delegate bool RenderFrameCallback();

        public delegate void RenderCallback();

        public delegate void RenderTreeNodeCallback(bool is_open);
        
        [StructLayout(LayoutKind.Sequential)]
        public struct Style {
            public Emphasis emp;
        }
        
        [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void app_run(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string title,
            int width,
            int height,
            bool has_menubar,
            bool can_scroll,
            bool center_on_screen,
            RenderFrameCallback callback);

        [DllImport(_libName)]
        internal static extern void id_frame(int scope_id, RenderCallback c_callback);

        [DllImport(_libName)]
        internal static extern void sl(float offset);

        [DllImport(_libName)]
        internal static extern void lbl([MarshalAs(UnmanagedType.LPUTF8Str)] string text, ref Style style);

        [DllImport(_libName)]
        internal static extern bool selectable([MarshalAs(UnmanagedType.LPUTF8Str)] string text, bool span_columns);

        [DllImport(_libName)]
        internal static extern bool checkbox([MarshalAs(UnmanagedType.LPUTF8Str)] string label, ref bool is_checked, bool is_small);

        [DllImport(_libName)]
        internal static extern bool button(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string text,
            Emphasis emphasis,
            bool is_enabled,
            bool is_small);

        [DllImport(_libName)]
        internal static extern void sep([MarshalAs(UnmanagedType.LPUTF8Str)] string text);

        [DllImport(_libName)]
        internal static extern bool accordion([MarshalAs(UnmanagedType.LPUTF8Str)] string header, bool default_open);

        [DllImport(_libName)]
        internal static extern bool hyperlink(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string text,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string? url_to_open);

        [DllImport(_libName)]
        internal static extern void toast(Emphasis emphasis, [MarshalAs(UnmanagedType.LPUTF8Str)] string message);

        [DllImport(_libName)]
        internal static extern bool input_string(
            [MarshalAs(UnmanagedType.LPUTF8Str)] StringBuilder value,
            int value_max_length,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            bool enabled, float width, bool is_readonly);

        [DllImport(_libName)]
        internal static extern bool input_int(
            ref int value,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            bool enabled, float width, bool is_readonly);

        [DllImport(_libName)]
        internal static extern bool input_multiline(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string id,
            [MarshalAs(UnmanagedType.LPUTF8Str)] StringBuilder value,
            int value_max_length,
            float height,
            bool autoscroll,
            bool enabled,
            bool use_fixed_font);

        [DllImport(_libName)]
        internal static extern void spinner_hbo_dots(float radius, float thickness, float speed, int dot_count);

        [DllImport(_libName)]
        internal static extern bool slider_float(ref float value, float min, float max,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            float step, bool ticks, Emphasis emphasis);

        [DllImport(_libName)]
        internal static extern bool slider_int(ref int value, int min, int max,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            int step, bool ticks, Emphasis emphasis);

        [DllImport(_libName)]
        internal static extern void tt([MarshalAs(UnmanagedType.LPUTF8Str)] string text, ShowDelay delay);

        [DllImport(_libName)]
        internal static extern void rich_tt(RenderCallback c_callback, ShowDelay delay);

        [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool combo(
            [MarshalAs(UnmanagedType.LPUTF8Str)]  string label,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)] string[] options,
            int options_size,
            ref uint selected,
            float width);

        [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool list(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)] string[] options,
            int options_size,
            ref uint selected,
            float width);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void RenderTableCellCallback(int row, int col);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void RenderPtrCallback(IntPtr user_data);

        [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void big_table(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string id,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)] string[] columns,
            int columns_size,
            int row_count,
            float outer_width,
            float outer_height,
            bool alternate_row_bg,
            RenderTableCellCallback cell_callback);

        [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void table(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string id,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)] string[] columns,
            int columns_size,
            float outer_width,
            float outer_height,
            bool alternate_row_bg,
            RenderPtrCallback cell_callback);

        [DllImport(_libName)]
        internal static extern bool table_begin_row(nint table_ptr);

        [DllImport(_libName)]
        internal static extern bool table_next_column(nint table_ptr);


        [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void tree_node(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
            bool open_by_default,
            bool is_leaf,
            bool span_all_cols,
            RenderTreeNodeCallback content_callback);

        // -- tabs

        [DllImport(_libName)]
        internal static extern void tab_bar([MarshalAs(UnmanagedType.LPUTF8Str)] string id, RenderPtrCallback render_callback);

        [DllImport(_libName)]
        internal static extern void tab(nint tab_bar_ptr,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string title,
            bool unsaved, bool selected,
            RenderCallback c_render_callback);

        [DllImport(_libName)]
        internal static extern void status_bar(RenderCallback c_callback);

        [DllImport(_libName)]
        internal static extern bool is_hovered();

        [DllImport(_libName)]
        internal static extern bool is_leftclicked();

        [DllImport(_libName)]
        internal static extern bool is_rightclicked();

        // application menus

        [DllImport(_libName)]
        internal static extern void menu_bar(RenderCallback c_callback);

        [DllImport(_libName)]
        internal static extern void menu([MarshalAs(UnmanagedType.LPUTF8Str)] string label, RenderCallback c_callback);

        [DllImport(_libName)]
        internal static extern bool menu_item(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string c_text,
            bool reserve_icon_space,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string c_icon);

        // windowing

        [DllImport(_libName)]
        internal static extern int window(
            int id,
            bool unregister,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string title,
            int width, int height,
            ref bool is_open,
            RenderCallback c_callback
            );

        // code editor
        [DllImport(_libName)]
        internal static extern int code_editor(int id, bool unregister, int language,
            [MarshalAs (UnmanagedType.LPUTF8Str)]
            string? c_text);

        // system debug

        [DllImport(_libName)]
        internal static extern void get_debug_info(ref float fps);

    }
}
