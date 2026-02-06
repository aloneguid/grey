using System.Text;
using Grey;
using static Grey.App;

bool isRunning = true;
bool isChecked = true;
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
bool alternateTableRowBg = false;
bool tableSelectables = false;
bool tableSelectableRow = false;
string[] choices = ["one", "two", "three"];
uint currentChoice = 0;

void Basics() {

    Label("labels:");
    foreach(Emphasis emp in Enum.GetValues<Emphasis>()) {
        SL();
        Label(emp.ToString(), emp);
    }

    Label("Buttons:");
    foreach(Emphasis emp in Enum.GetValues<Emphasis>()) {
        SL();
        Button(emp.ToString(), emp);
    }

    Label(message);

    if(Button("get current date (has tooltip)")) {
        message = DateTime.Now.ToString();
    }

    Label("Tooltips:");

    SL(); Label("simple"); TT("simple tooltip");
    SL(); Label("rich"); TT(() => {
        Sep("rich");
        Label("rich tooltip");
        Button("close");
    });

    SL();
    if(Button("Quit")) {
        isRunning = false;
    }

    SL();
    Button("small button", isSmall: true);

    Checkbox("checkbox", ref isChecked);
    SL();
    SmallCheckbox("small checkbox", ref isChecked);

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

    Sep("Mouse helpers");

    Label("hovered: ");
    SL();
    Label(IsHovered ? "yes" : "no");

    Label("left clicked: ");
    SL();
    Label(IsLeftClicked ? "yes" : "no");

    Label("right clicked: ");
    SL();
    Label(IsRightClicked ? "yes" : "no");

    // collision demo
    for(int i = 0; i < 5; i++) {
        if(i > 0) SL();

        IdFrame(i, () => {
            Button("collide");
        });
    }
}


Grey.App.Run("Grey# Demo", () => {

    // top menu
    MenuBar(() => {
        Menu("File", () => {
            if(MenuItem("Quit", true, Icon.Exit_to_app)) {
                isRunning = false;
            }
        });
    });

    TabBar("1", tb => {

        tb.TabItem("Basics", Basics);

        tb.TabItem("Lists", () => {
            if(Combo("combo", choices, ref currentChoice)) {
                Notify($"COMBO choice changed to {choices[currentChoice]}");
            }
            if(List("list", choices, ref currentChoice)) {
                Notify($"LIST choice changed to {choices[currentChoice]}");
            }
            Label($"current: {currentChoice}");
        });

        tb.TabItem("Icons", () => {
            Label($"{Icon.Num10k} {Icon.Fireplace} {Icon.Access_alarm}");
        });

        tb.TabItem("Spinner", () => {
            Slider(ref spinnerRadius, 1, 100, "radius");
            Slider(ref spinnerThickness, 1, 50, "thickness");
            Slider(ref spinnerSpeed, 0.1f, 10, "speed");
            Slider(ref spinnerDotCount, 1, 100, "dot count");
            SpinnerHboDots(spinnerRadius, spinnerThickness, spinnerSpeed, spinnerDotCount);
        });

        tb.TabItem("Editor", () => {
            Slider(ref mlHeight, -1000, 1000, "height");
            Checkbox("enabled", ref mlEnabled);
            Checkbox("use fixed font", ref mlUseFixedFont);

            if(InputMultiline("multiline", multilineText, mlHeight, false, mlEnabled, mlUseFixedFont)) {
                Notify("multiline changed");
            }
        });

        tb.TabItem("Collapsibles", () => {
            TreeNode("parent", true, false, (isOpen) => {
                if(isOpen) {
                    TreeNode("child 1");
                    TreeNode("child 2", false, false, (isOpen) => {
                        if(isOpen) {
                            TreeNode("grand 1");
                            TreeNode("grand 2");
                        }
                    });
                }
            });
        });

        tb.TabItem("Table", () => {
            Table("t0", ["id", "name", "description+"], ta => {
                ta.BeginRow();
                Label("1");
                ta.NextColumn();
                Label("Grey");
                ta.NextColumn();
                Label("A cross-platform immediate mode GUI library for .NET");

                // try tree inside table
                ta.BeginRow();
                TreeNode("root", true, false, (isOpen) => {
                    ta.NextColumn();
                    Label("--");
                    ta.NextColumn();
                    Label("--");

                    if(isOpen) {
                        ta.BeginRow();
                        Label("2");
                        ta.NextColumn();
                        Label("Two");
                        ta.NextColumn();
                        Label("Two is a good number");
                    }
                });
            });
        });

        tb.TabItem("Big table", () => {
            Checkbox("alg bg", ref alternateTableRowBg);
            Checkbox("selectable", ref tableSelectables);
            if(tableSelectables) {
                SL();
                Checkbox("entire row", ref tableSelectableRow);
            }

            BigTable("t0", new[] { "id", "name", "description+" }, 100000,
      (int row, int column) => {

          if(column == 0) {
              if(tableSelectables) {
                  if(Selectable($"{row}", tableSelectableRow)) {
                      Notify($"clicked {row}x{column}");
                  }
              } else {
                  Label(row.ToString());
              }
          } else {
              Label($"{row}x{column}");
          }
      },
                  alternateRowBg: alternateTableRowBg);
        });
    });

    // Status bar
    StatusBar(() => {
        PrintFps();
    });

    return isRunning;

}, height: 800, centerOnScreen: true, isScrollable: false, hasMenuBar: true);
