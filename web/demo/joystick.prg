import "mod_joy";
import "mod_key";
import "mod_text";
import "mod_video";

private
    int i;
    int buttons;
    int button_id[32];
    int state[32];
    int text_id[32];

begin

    set_mode(320, 240, 16);

    write(0, 10, 10, 0, "Joystick 0:");

    if (joy_name(0) != "")

        write(0, 10, 30, 0, joy_name(0));

        buttons = joy_numbuttons(0);

        for (i = 0; i < buttons && i < 32; i++)

            button_id[i] = i;
            state[i] = joy_getbutton(0, i);

            if (i < 16)

                write(0, 10, 50 + i * 10, 0, "Button");
                write_int(0, 65, 50 + i * 10, 0, 0, &button_id[i]);
                write(0, 85, 50 + i * 10, 0, 0, ":");
                write_int(0, 100, 50 + i * 10, 0, 0, &state[i]);

            else

                write(0, 160, 50 + (i - 16) * 10, 0, 0, "Button");
                write_int(0, 215, 50 + (i - 16) * 10, 0, 0, &button_id[i]);
                write(0, 235, 50 + (i - 16) * 10, 0, 0, ":");
                write_int(0, 250, 50 + (i - 16) * 10, 0, 0, &state[i]);

            end

        end

        repeat

            for (i = 0; i < buttons && i < 32; i++)
                state[i] = joy_getbutton(0, i);
            end

            frame;

        until (key(_ESC));

    else

        write(0, 10, 30, 0, "No joystick detected");

        repeat
            frame;
        until (key(_ESC));

    end

end
