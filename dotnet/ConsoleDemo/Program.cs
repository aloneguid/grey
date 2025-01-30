using System.Text;
using Grey;
using static Grey.App;

bool isRunning = true;
string message = "1";

Grey.App.Run(ref isRunning, "Grey# Demo", () => {

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

                int totalRows = 1000000;
                Label($"Large table with {totalRows} rows");
                using(var tbl = new Table("t0", [
                    new Table.Column("id"),
                    new Table.Column("name"),
                    new Table.Column("description", true)
                ], totalRows, 0, 200)) {
                    if(tbl) {

                        while(tbl.Step(out int ds, out int de)) {
                            for(int i = ds; i < de; i++) {
                                tbl.NextRow();

                                Label(i.ToString());

                                tbl.NextCol();
                                Label($"name {i}");
                                tbl.NextCol();
                                Label($"description {i}");
                            }
                        }
                    }
                }

                int totalColumns = 100;
                Label($"Large table with {totalColumns} columns");
                Table.Column[] cols = Enumerable.Range(0, totalColumns).Select(i => new Table.Column($"column {i}")).ToArray();
                using(var tbl = new Table("t1", cols, totalRows)) {
                    if(tbl) {
                        while(tbl.Step(out int ds, out int de)) {
                            for(int i = ds; i < de; i++) {
                                tbl.NextRow();
                                for(int j = 0; j < totalColumns; j++) {
                                    if(j >0)
                                        tbl.NextCol();
                                    Label($"{i}x{j}");
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}, height: 800, centerOnScreen: true);
