using static Grey.App;

bool isRunning = true;
string message = "1";

Grey.App.Run(ref isRunning, "Hello, World!", () => {
    Label("hohoo");
    Label(message);

    if(Button("Exit")) {
        isRunning = false;
    }

    if(Button("msg+")) {
        message += "1";
    }

}, height: 400);
