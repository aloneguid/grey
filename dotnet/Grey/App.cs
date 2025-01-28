namespace Grey {
    public static class App {
        public static void Run(ref bool isRunning, string title, Action renderFrame,
            int width = 800, int height = 600) {
            var callback = new Native.RenderFrameCallback(renderFrame);
            Native.app_run(ref isRunning, title, width, height, callback);
        }

        public static void Label(string text) {
            Native.label(text);
        }

        public static bool Button(string text) {
            return Native.button(text);
        }
    }
}
