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
    }
}
