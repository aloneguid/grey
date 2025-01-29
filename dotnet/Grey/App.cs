using System.Text;

namespace Grey {
    public static class App {
        public static void Run(ref bool isRunning, string title, Action renderFrame,
            int width = 800, int height = 600, bool hasMenuBar = false) {
            var callback = new Native.RenderFrameCallback(renderFrame);
            Native.app_run(ref isRunning, title, width, height, hasMenuBar, callback);
        }

        public static void SL(float offset = 0) {
            Native.sl(offset);
        }

        public static void Label(string text) {
            Native.label(text);
        }

        public static bool Button(string text, Emphasis emphasis = Emphasis.None) {
            return Native.button(text, emphasis);
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
            return Native.input(value, value.Capacity, label, enabled, width, is_readonly);
        }

        public static bool Input(ref string value, string label,
            bool enabled = true, float width = 0, bool is_readonly = false) {
            var sb = new StringBuilder(value, Math.Max(10, value.Length * 2));
            bool ret = Native.input(sb, sb.Capacity, label, enabled, width, is_readonly);
            if(ret) {
                value = sb.ToString();
            }
            return ret;
        }
    }
}