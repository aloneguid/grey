using Grey;
using static Grey.App;

bool isRunning = true;
string message = "1";

Grey.App.Run(ref isRunning, "Grey# Demo", () => {

    Label("Simple label followed by sep");
    Sep();
    Sep("sep with text");

    Label($"icons: {Icon.Num10k}");

    Label(message);

    if(Button("Exit")) {
        isRunning = false;
    }

    if(Button("msg+")) {
        message += "1";
    }

}, height: 400);
