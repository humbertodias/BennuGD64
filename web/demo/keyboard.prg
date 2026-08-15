import "mod_key";
import "mod_text";
import "mod_video";

private
    int dir_up;
    int dir_down;
    int dir_left;
    int dir_right;
    int k_enter;
    int k_space;
    int k_control;
    int k_alt;
    int k_shift;
    int k_tab;
    int k_w;
    int k_a;
    int k_s;
    int k_d;
    int k_z;
    int k_x;
    int k_c;
    int k_f1;
    int last_scan;

begin

    set_mode(320, 240, 16);
    set_fps(30, 0);

    write(0, 10, 10, 0, "Keyboard:");
    write(0, 10, 24, 0, "scan");
    write_int(0, 50, 24, 0, &last_scan);
    write(0, 90, 24, 0, "(Esc exits)");

    write(0, 10, 42, 0, "Up");
    write_int(0, 40, 42, 0, &dir_up);
    write(0, 70, 42, 0, "Down");
    write_int(0, 110, 42, 0, &dir_down);
    write(0, 160, 42, 0, "Left");
    write_int(0, 200, 42, 0, &dir_left);
    write(0, 240, 42, 0, "Right");
    write_int(0, 285, 42, 0, &dir_right);

    write(0, 10, 62, 0, "Enter");
    write_int(0, 70, 62, 0, &k_enter);
    write(0, 10, 72, 0, "Space");
    write_int(0, 70, 72, 0, &k_space);
    write(0, 10, 82, 0, "Control");
    write_int(0, 70, 82, 0, &k_control);
    write(0, 10, 92, 0, "Alt");
    write_int(0, 70, 92, 0, &k_alt);
    write(0, 10, 102, 0, "Shift");
    write_int(0, 70, 102, 0, &k_shift);
    write(0, 10, 112, 0, "Tab");
    write_int(0, 70, 112, 0, &k_tab);
    write(0, 10, 122, 0, "F1");
    write_int(0, 70, 122, 0, &k_f1);

    write(0, 160, 62, 0, "W");
    write_int(0, 200, 62, 0, &k_w);
    write(0, 160, 72, 0, "A");
    write_int(0, 200, 72, 0, &k_a);
    write(0, 160, 82, 0, "S");
    write_int(0, 200, 82, 0, &k_s);
    write(0, 160, 92, 0, "D");
    write_int(0, 200, 92, 0, &k_d);
    write(0, 160, 102, 0, "Z");
    write_int(0, 200, 102, 0, &k_z);
    write(0, 160, 112, 0, "X");
    write_int(0, 200, 112, 0, &k_x);
    write(0, 160, 122, 0, "C");
    write_int(0, 200, 122, 0, &k_c);

    repeat

        dir_up = 0;
        dir_down = 0;
        dir_left = 0;
        dir_right = 0;

        if (key(_UP) OR key(_W)) dir_up = 1; end
        if (key(_DOWN) OR key(_S)) dir_down = 1; end
        if (key(_LEFT) OR key(_A)) dir_left = 1; end
        if (key(_RIGHT) OR key(_D)) dir_right = 1; end

        k_enter = key(_ENTER);
        k_space = key(_SPACE);
        k_control = key(_CONTROL);
        k_alt = key(_ALT);
        k_shift = key(_L_SHIFT) OR key(_R_SHIFT);
        k_tab = key(_TAB);
        k_f1 = key(_F1);
        k_w = key(_W);
        k_a = key(_A);
        k_s = key(_S);
        k_d = key(_D);
        k_z = key(_Z);
        k_x = key(_X);
        k_c = key(_C);

        if (scan_code)
            last_scan = scan_code;
        end

        frame;

    until (key(_ESC));

end
