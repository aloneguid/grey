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
bool editorRich = false;
CodeEditor? ce = null;
bool mlUseFixedFont = false;
bool alternateTableRowBg = false;
bool tableSelectables = false;
bool tableSelectableRow = false;
string[] choices = ["one", "two", "three"];
uint currentChoice = 0;
bool xwnd2Show = true;

// Windows
bool WindowHasTitleBar = true;
bool WindowIsOpen = false;
bool WindowUseIsOpen = true;
float WindowOpacity = 1.0f;
float WindowBorder = .0f;
bool WindowScrollable = true;

// Image
bool ImageRounded = false;
float ImageRounding = 5.0f;
float ImageScale = 0.5f;


void Basics() {

    if(Accordion("Icons")) {
        Lbl(Icon.Access_alarms);
    }

    if(Accordion("Label styles")) {
        Lbl("");
        foreach(Emphasis emp in Enum.GetValues<Emphasis>()) {
            SL();
            Lbl(emp.ToString(), new Style{Emp = emp});
        }
    }

    if(Accordion("Windows")) {
        Checkbox("show window", ref WindowIsOpen);
        Checkbox("display close button", ref WindowUseIsOpen);
        Checkbox("title bar", ref WindowHasTitleBar);
        Slider(ref WindowOpacity, 0.0f, 1.0f, "opacity", 0.1f);
        Checkbox("scrollable", ref WindowScrollable);
        Slider(ref WindowBorder, 0.0f, 10.0f, "border", 0.1f);
        
        Lbl("window API is not marshalled yet", new Style{Emp = Emphasis.Error});

        if(WindowIsOpen) {
            // if(w::wnd w1{
            //     "windows 1", {
            //         .open_ptr = use_is_open ? &is_open : nullptr,
            //             .opacity = opacity,
            //             .show_title_bar = title_bar,
            //             .border = border,
            //             .scrollable = scrollable
            //     }
            // }) {
            //     w::lbl("DPI: ", {.emp = emphasis::primary});
            //     w::sl();
            //     w::lbl(format("{}", w::scale));
            //
            //     w::lbl("Position:", {.emp = emphasis::primary});
            //     w::sl();
            //     w::lbl(format("{}", w1.pos()));
            //
            //     w::lbl("Size: ", {.emp = emphasis::primary});
            //     w::sl();
            //     w::lbl(format("{}", w1.size()));
            // }
        }
    }

    if(Accordion("Image")) {
        Lbl("Image API is not marshalled yet.");
        // auto texture = app->get_texture("luna");
        // if(texture) {
        //     w::checkbox("rounded", img_rounded);
        //     w::slider(img_scale, 0.1f, 3.0f, "scale");
        //     if(img_rounded) {
        //         w::slider(img_rounding, 1, 50, "rounding");
        //         w::image_rounded(*app, "luna", texture->size * img_scale, img_rounding);
        //     } else {
        //         w::image(*app, "luna", texture->size * img_scale);
        //     }
        // }
    }
    
    // ---

    Lbl("Buttons:");
    foreach(Emphasis emp in Enum.GetValues<Emphasis>()) {
        SL();
        Button(emp.ToString(), emp);
    }

    Lbl(message);

    if(Button("get current date (has tooltip)")) {
        message = DateTime.Now.ToString();
    }

    Lbl("Tooltips:");

    SL(); Lbl("simple"); TT("simple tooltip");
    SL(); Lbl("rich"); TT(() => {
        Sep("rich");
        Lbl("rich tooltip");
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
        Lbl("accordion content");
    }

    if(Accordion("Open accordion", true)) {
        Lbl("accordion content (open)");
    }
    if(Hyperlink("click me")) {
        Toast(Emphasis.Info, "hyperlink clicked");
    }

    Hyperlink("blog", "https://www.aloneguid.uk/posts/");

    if(Input(ref message, "default input")) {
        Toast(Emphasis.Info, "input changed");
    }

    Input(ref message, "disabled input", false);
    Input(ref message, "readonly input", is_readonly: true);
    Input(ref message, "width explicitly set to 400", width: 400);
    Input(ref number, "number input");

    Sep("Mouse helpers");

    Lbl("hovered: ");
    SL();
    Lbl(IsHovered ? "yes" : "no");

    Lbl("left clicked: ");
    SL();
    Lbl(IsLeftClicked ? "yes" : "no");

    Lbl("right clicked: ");
    SL();
    Lbl(IsRightClicked ? "yes" : "no");

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
                Toast(Emphasis.Info, $"COMBO choice changed to {choices[currentChoice]}");
            }
            if(List("list", choices, ref currentChoice)) {
                Toast(Emphasis.Info, $"LIST choice changed to {choices[currentChoice]}");
            }
            Lbl($"current: {currentChoice}");
        });

        tb.TabItem("Icons", () => {
            Lbl($"{Icon.Num10k} {Icon.Fireplace} {Icon.Access_alarm}");
        });

        tb.TabItem("Spinner", () => {
            Slider(ref spinnerRadius, 1, 100, "radius");
            Slider(ref spinnerThickness, 1, 50, "thickness");
            Slider(ref spinnerSpeed, 0.1f, 10, "speed");
            Slider(ref spinnerDotCount, 1, 100, "dot count");
            // SpinnerHboDots(spinnerRadius, spinnerThickness, spinnerSpeed, spinnerDotCount);
        });

        tb.TabItem("Editor", () => {
            Slider(ref mlHeight, -1000, 1000, "height");
            Checkbox("enabled", ref mlEnabled);
            Checkbox("use fixed font", ref mlUseFixedFont);
            Checkbox("rich editor", ref editorRich);

            if(editorRich) {
                if(ce == null) {
                    ce = new CodeEditor(ProgrammingLanguage.Json);
                    ce.Text = multilineText.ToString();
                }
                ce.Render();
            } else {
                if(InputMultiline("multiline", multilineText, mlHeight, false, mlEnabled, mlUseFixedFont)) {
                    Toast(Emphasis.Info, "multiline changed");
                }
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
                Lbl("1");
                ta.NextColumn();
                Lbl("Grey");
                ta.NextColumn();
                Lbl("A cross-platform immediate mode GUI library for .NET");

                // try tree inside table
                ta.BeginRow();
                TreeNode("root", true, false, (isOpen) => {
                    ta.NextColumn();
                    Lbl("--");
                    ta.NextColumn();
                    Lbl("--");

                    if(isOpen) {
                        ta.BeginRow();
                        Lbl("2");
                        ta.NextColumn();
                        Lbl("Two");
                        ta.NextColumn();
                        Lbl("Two is a good number");
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
                      Toast(Emphasis.Info, $"clicked {row}x{column}");
                  }
              } else {
                  Lbl(row.ToString());
              }
          } else {
              Lbl($"{row}x{column}");
          }
      },
                  alternateRowBg: alternateTableRowBg);
        });
    });

    // Status bar
    StatusBar(() => {
        Lbl($"{Fps:F1} | {GreyVersion}");
    });

    return isRunning;

}, height: 800, centerOnScreen: true, isScrollable: false, hasMenuBar: true);
