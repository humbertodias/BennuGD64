import "mod_joy";
import "mod_key";
import "mod_text";
import "mod_video";

private
    int i;
    int njoys;
    int ready;
    int buttons;
    int button_id[32];
    int state[32];
    int dir_up;
    int dir_down;
    int dir_left;
    int dir_right;
    int hat;
    string status;

begin

    set_mode(320, 240, 16);
    set_fps(30, 0);

    status = "Waiting for joystick...";
    write(0, 10, 10, 0, "Joystick 0:");
    write_string(0, 10, 24, 0, &status);

    repeat

        njoys = joy_numjoysticks();

        if (njoys < 1)

            status = "Waiting for joystick...";
            dir_up = 0;
            dir_down = 0;
            dir_left = 0;
            dir_right = 0;

            if (ready)

                delete_text(all_text);
                write(0, 10, 10, 0, "Joystick 0:");
                write_string(0, 10, 24, 0, &status);

                for (i = 0; i < buttons && i < 32; i++)
                    state[i] = 0;
                end

                buttons = 0;
                ready = 0;

            end

        else

            if (ready == 0)

                ready = 1;
                status = joy_name(0);
                if (status == "")
                    status = "Joystick 0";
                end

                write(0, 10, 42, 0, "Up");
                write_int(0, 40, 42, 0, &dir_up);
                write(0, 70, 42, 0, "Down");
                write_int(0, 110, 42, 0, &dir_down);
                write(0, 160, 42, 0, "Left");
                write_int(0, 200, 42, 0, &dir_left);
                write(0, 240, 42, 0, "Right");
                write_int(0, 285, 42, 0, &dir_right);

                buttons = joy_numbuttons(0);

                for (i = 0; i < buttons && i < 32; i++)

                    button_id[i] = i;
                    state[i] = joy_getbutton(0, i);

                    if (i < 16)

                        write(0, 10, 62 + i * 10, 0, "Button");
                        write_int(0, 65, 62 + i * 10, 0, 0, &button_id[i]);
                        write(0, 85, 62 + i * 10, 0, 0, ":");
                        write_int(0, 100, 62 + i * 10, 0, 0, &state[i]);

                    else

                        write(0, 160, 62 + (i - 16) * 10, 0, "Button");
                        write_int(0, 215, 62 + (i - 16) * 10, 0, 0, &button_id[i]);
                        write(0, 235, 62 + (i - 16) * 10, 0, 0, ":");
                        write_int(0, 250, 62 + (i - 16) * 10, 0, 0, &state[i]);

                    end

                end

            end

            dir_up = 0;
            dir_down = 0;
            dir_left = 0;
            dir_right = 0;

            if (joy_numaxes(0) > 0)
                if (joy_getaxis(0, 0) < -16384) dir_left = 1; end
                if (joy_getaxis(0, 0) > 16384) dir_right = 1; end
            end

            if (joy_numaxes(0) > 1)
                if (joy_getaxis(0, 1) < -16384) dir_up = 1; end
                if (joy_getaxis(0, 1) > 16384) dir_down = 1; end
            end

            if (joy_numhats(0) > 0)
                hat = joy_gethat(0, 0);
                if (hat BAND JOY_HAT_UP) dir_up = 1; end
                if (hat BAND JOY_HAT_DOWN) dir_down = 1; end
                if (hat BAND JOY_HAT_LEFT) dir_left = 1; end
                if (hat BAND JOY_HAT_RIGHT) dir_right = 1; end
            end

            if (buttons > 12)
                if (joy_getbutton(0, 12)) dir_up = 1; end
                if (joy_getbutton(0, 13)) dir_down = 1; end
                if (joy_getbutton(0, 14)) dir_left = 1; end
                if (joy_getbutton(0, 15)) dir_right = 1; end
            end

            for (i = 0; i < buttons && i < 32; i++)
                state[i] = joy_getbutton(0, i);
            end

        end

        frame;

    until (key(_ESC));

end
