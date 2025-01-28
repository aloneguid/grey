using Grey;
using static Grey.App;

bool isRunning = true;
string message = "1";

Grey.App.Run(ref isRunning, "Grey# Demo", () => {

    using(new TabBar()) {
        using(var ti = new TabItem("Basics")) {
            if(ti) {
                Label("Simple label followed by sep");
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

                using(var tbl = new Table("x", [
                    new Table.Column("id"),
                    new Table.Column("name"),
                    new Table.Column("description", true)
                ], totalRows)) {
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
            }
        }
    }

}, height: 400);
