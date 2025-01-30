using System.Text;
using Grey;
using static Grey.App;

bool isRunning = true;
string message = "1";

Grey.App.Run("Grey# Demo", () => {

    using(new TabBar()) {
        using(var ti = new TabItem("Basics")) {
            if(ti) {
                Label("simple label");
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
            }
        }

        using(var ti = new TabItem("Icons")) {
            if(ti) {
                Label($"{Icon.Num10k} {Icon.Fireplace}");
            }
        }

        using(var ti = new TabItem("Tables")) {
            if(ti) {

                int totalRows = 100;
                Label($"Simple table with {totalRows} rows");
                using(var tbl = new Table("t0", ["id", "name", "description+"])) {
                    if(tbl) {

                        for(int i = 0; i < totalRows; i++) {
                            tbl.BeginRow();

                            tbl.BeginCol();
                            Label(i.ToString());

                            tbl.BeginCol();
                            Label($"name {i}");
                            tbl.BeginCol();
                            Label($"description {i}");
                        }
                    }
                }
            }
        }

    }

    return isRunning;

}, height: 800, centerOnScreen: true);
