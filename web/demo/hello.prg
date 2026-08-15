import "mod_video"
import "mod_map"
import "mod_draw"
import "mod_key"
import "mod_proc"
import "mod_text"

PROCESS Main()
BEGIN
    set_mode(320, 240, 16);
    set_fps(30, 0);
    write(0, 160, 120, ALIGN_CENTER, "Hello World");
    LOOP
        IF (key(_esc))
            exit();
        END
        FRAME;
    END
END
