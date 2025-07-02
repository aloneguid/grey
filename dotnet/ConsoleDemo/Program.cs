using System.Text;
using Grey;
using static Grey.App;

bool isRunning = true;
string message = "1";
int number = 42;
float spinnerRadius = 16;
float spinnerThickness = 4;
float spinnerSpeed = 1;
int spinnerDotCount = 8;
StringBuilder multilineText = new StringBuilder("initial text\nanother line", 1000);
float mlHeight = 0;
bool mlEnabled = true;
bool mlUseFixedFont = false;


Grey.App.Run("Grey# Demo", () => {

    // top menu
    using(var bar = new MenuBar()) {
        if(bar) {
            using(MenuBar.Menu mFile = bar.M("File")) {
                if(mFile) {
                    if(mFile.Item("Quit")) {
                        isRunning = false;
                    }
                }
            }
        }
    }

    Label("fps:"); SL(); PrintFps();

    using(new TabBar()) {
        using(var ti = new TabItem("Basics")) {
            if(ti) {
                Label("simple label");
                Tooltip("simple text tooltip");
                SL();
                Label("primary", Emphasis.Primary);
                SL();
                Label("error", Emphasis.Error);
                SL();
                Label("disabled label", isEnabled: false);
                Sep();
                Sep("sep with text");

                Label(message);

                if(Button("get current date")) {
                    message = DateTime.Now.ToString();
                }

                Button("no emphasis", Emphasis.None);
                SL();
                Button("primary", Emphasis.Primary);
                SL();
                Button("error", Emphasis.Error);
                SL();
                Button("disabled", isEnabled: false);
                SL();
                Button("small", isSmall: true);

                if(Button("Quit")) {
                    isRunning = false;
                }

                if(Accordion("Normal accordion")) {
                    Label("accordion content");
                }

                if(Accordion("Open accordion", true)) {
                    Label("accordion content (open)");
                }
                if(Hyperlink("click me")) {
                    Notify("hyperlink clicked");
                }

                Hyperlink("blog", "https://www.aloneguid.uk/posts/");

                if(Input(ref message, "default input")) {
                    Notify("input changed");
                }

                Input(ref message, "disabled input", false);
                Input(ref message, "readonly input", is_readonly: true);
                Input(ref message, "width explicitly set to 400", width: 400);
                Input(ref number, "number input");
            }
        }

        using(var ti = new TabItem("Icons")) {
            if(ti) {
                Label($"{Icon.Num10k} {Icon.Fireplace}");
            }
        }

        // Spinner
        using(var ti = new TabItem("Spinner")) {
            if(ti) {
                Slider(ref spinnerRadius, 1, 100, "radius");
                Slider(ref spinnerThickness, 1, 50, "thickness");
                Slider(ref spinnerSpeed, 0.1f, 10, "speed");
                Slider(ref spinnerDotCount, 1, 100, "dot count");
                SpinnerHboDots(spinnerRadius, spinnerThickness, spinnerSpeed, spinnerDotCount);
            }
        }

        // Multiline editor
        using(var ti = new TabItem("Editor")) {
            if(ti) {
                Slider(ref mlHeight, -1000, 1000, "height");
                Checkbox("enabled", ref mlEnabled);
                Checkbox("use fixed font", ref mlUseFixedFont);

                if(InputMultiline("multiline", multilineText, mlHeight, false, mlEnabled, mlUseFixedFont)) {
                    Notify("multiline changed");
                }
            }
        }

        using(var ti = new TabItem("Tables")) {
            if(ti) {
                Table("t0", new[] { "id", "name", "description+" }, 100000,
                    (int row, int column) => {
                        Label($"{row}x{column}");
                    });
            }
        }

        // Status bar
        using(new StatusBar()) {
            Label("status bar content");
        }

    }

    return isRunning;

}, height: 800, centerOnScreen: true, isScrollable: false, hasMenuBar: true);
