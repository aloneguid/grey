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
    }

}, height: 400);
