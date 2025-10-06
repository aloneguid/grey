using System;
using System.Text;

namespace Grey {
    public static class App {
        public static void Run(string title, Func<bool> renderFrame,
            int width = 800, int height = 600,
            bool hasMenuBar = false,
            bool isScrollable = true,
            bool centerOnScreen = false) {
            var callback = new Native.RenderFrameCallback(renderFrame);
            Native.app_run(title, width, height, hasMenuBar, isScrollable, centerOnScreen, callback);
        }

        public static void SL(float offset = 0) {
            Native.sl(offset);
        }
        public static void Label(string text, Emphasis emphasis = Emphasis.None, int textWrapPos = 0, bool isEnabled = true) {
            Native.label(text, emphasis, textWrapPos, isEnabled);
        }

        public static bool Checkbox(string label, ref bool isChecked) {
            return Native.checkbox(label, ref isChecked);
        }

        public static bool Button(string text, Emphasis emphasis = Emphasis.None,
            bool isEnabled = true, bool isSmall = false) {
            return Native.button(text, emphasis, isEnabled, isSmall);
        }

        public static void Sep(string text = "") {
            Native.sep(text);
        }

        public static bool Accordion(string header, bool defaultOpen = false) {
            return Native.accordion(header, defaultOpen);
        }

        public static bool Hyperlink(string text, string? urlToOpen = null) {
            return Native.hyperlink(text, urlToOpen);
        }

        public static void Notify(string message) {
            Native.notify(message);
        }

        public static bool Input(StringBuilder value, string label,
            bool enabled = true, float width = 0, bool is_readonly = false) {
            return Native.input_string(value, value.Capacity, label, enabled, width, is_readonly);
        }

        public static bool Input(ref string value, string label,
            bool enabled = true, float width = 0, bool is_readonly = false) {
            var sb = new StringBuilder(value, Math.Max(10, value.Length * 2));
            bool ret = Native.input_string(sb, sb.Capacity, label, enabled, width, is_readonly);
            if(ret) {
                value = sb.ToString();
            }
            return ret;
        }

        public static bool Input(ref int value, string label,
            bool enabled = true, float width = 0, bool is_readonly = false) {
            return Native.input_int(ref value, label, enabled, width, is_readonly);
        }

        public static bool InputMultiline(string id, StringBuilder value,
            float height = 0,
            bool autoscroll = false,
            bool isEnabled = true,
            bool useFixedFont = false) {
            return Native.input_multiline(id, value, value.Capacity, height, autoscroll, isEnabled, useFixedFont);
        }

        public static void SpinnerHboDots(float radius = 16, float thickness = 4, float speed = 1, int dotCount = 6) {
            Native.spinner_hbo_dots(radius, thickness, speed, dotCount);
        }

        public static bool Slider(ref float value, float min, float max, string label) {
            return Native.slider_float(ref value, min, max, label);
        }

        public static bool Slider(ref int value, int min, int max, string label) {
            return Native.slider_int(ref value, min, max, label);
        }

        /// <summary>
        /// A text-only tooltip appearing on curren item's hover
        /// </summary>
        /// <param name="text"></param>
        public static void Tooltip(string text) {
            Native.tooltip(text);
        }

        /// <summary>
        /// Combo selection widget
        /// </summary>
        /// <param name="label">Label to display</param>
        /// <param name="items">List of selection items</param>
        /// <param name="currentItem">Reference to currently selected item index</param>
        /// <param name="width"></param>
        /// <returns>True if selection has changed</returns>
        public static bool Combo(string label, string[] items, ref uint currentItem, float width = 0) {
            nuint c = (nuint)currentItem;
            bool r = Native.combo(label, items, items.Length, ref c, width);
            currentItem = (uint)c;
            return r;
        }

        public static void Table(string id, string[] columns, int rowCount, Action<int, int> cellRender,
            float outer_width = 0, float outerHeight = 0,
            bool alternateRowBg = false) {
            Native.table(id, columns, columns.Length, rowCount,
                outer_width, outerHeight,
                alternateRowBg,
                (rowIndex, columnIndex) => {
                    cellRender(rowIndex, columnIndex);
                });
        }

        public static void PrintFps() {
            Native.label_fps();
        }
    }
}