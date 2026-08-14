import "mod_video"
import "mod_map"
import "mod_draw"
import "mod_key"
import "mod_proc"

PROCESS Main()
BEGIN
    set_mode(320, 240, 16);
    set_fps(30, 0);
    drawing_color(rgb(30, 70, 140));
    draw_box(0, 0, 319, 239);
    drawing_color(rgb(255, 200, 40));
    draw_fcircle(160, 120, 48);
    LOOP
        IF (key(_esc))
            exit();
        END
        FRAME;
    END
END
