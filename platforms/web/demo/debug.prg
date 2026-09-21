import "mod_video"
import "mod_key"
import "mod_text"
import "mod_proc"
import "mod_debug"

PROCESS Main()
PRIVATE
    int n = 0;
BEGIN
    set_mode(320, 240, 16);
    set_fps(30, 0);
    enemy();
    DEBUG;
    LOOP
        n++;
        write(0, 160, 16, 1, "n=" + n);
        IF (key(_esc))
            exit();
        END
        FRAME;
    END
END

PROCESS enemy()
PRIVATE
    int hp = 10;
BEGIN
    LOOP
        hp--;
        IF (hp <= 0)
            hp = 10;
        END
        FRAME;
    END
END
