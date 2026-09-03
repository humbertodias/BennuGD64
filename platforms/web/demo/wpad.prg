import "mod_wpad";
import "mod_key";
import "mod_mouse";
import "mod_text";
import "mod_video";

private
    int i;
    int chan;
    int ready;
    int batt;
    int is_bb;
    int ir_x;
    int ir_y;
    int ir_z;
    int ir_angle;
    int pitch;
    int roll;
    int accel_x;
    int accel_y;
    int accel_z;
    int mx;
    int my;
    int mleft;
    int mright;
    int bb_tl;
    int bb_tr;
    int bb_bl;
    int bb_br;
    int bb_x;
    int bb_y;
    int pad_ready[4];
    int rumble;
    int k_left;
    int k_right;
    string status;

begin

    set_mode(320, 240, 16);
    set_fps(30, 0);

    chan = 0;
    status = "Waiting for Wiimote...";

    write(0, 10, 10, 0, "WPAD");
    write_int(0, 50, 10, 0, &chan);
    write(0, 70, 10, 0, ":");
    write_string(0, 10, 24, 0, &status);

    write(0, 10, 42, 0, "Ready");
    write_int(0, 55, 42, 0, &ready);
    write(0, 90, 42, 0, "Batt");
    write_int(0, 130, 42, 0, &batt);
    write(0, 180, 42, 0, "BB");
    write_int(0, 210, 42, 0, &is_bb);

    write(0, 10, 54, 0, "IR X");
    write_int(0, 45, 54, 0, &ir_x);
    write(0, 100, 54, 0, "Y");
    write_int(0, 120, 54, 0, &ir_y);
    write(0, 175, 54, 0, "Z");
    write_int(0, 195, 54, 0, &ir_z);

    write(0, 10, 66, 0, "Ang");
    write_int(0, 45, 66, 0, &ir_angle);
    write(0, 100, 66, 0, "Pit");
    write_int(0, 130, 66, 0, &pitch);
    write(0, 185, 66, 0, "Rol");
    write_int(0, 215, 66, 0, &roll);

    write(0, 10, 78, 0, "Acc X");
    write_int(0, 55, 78, 0, &accel_x);
    write(0, 110, 78, 0, "Y");
    write_int(0, 130, 78, 0, &accel_y);
    write(0, 185, 78, 0, "Z");
    write_int(0, 205, 78, 0, &accel_z);

    write(0, 10, 90, 0, "Mouse");
    write_int(0, 55, 90, 0, &mx);
    write_int(0, 100, 90, 0, &my);
    write(0, 155, 90, 0, "L");
    write_int(0, 175, 90, 0, &mleft);
    write(0, 210, 90, 0, "R");
    write_int(0, 230, 90, 0, &mright);

    write(0, 10, 102, 0, "Pads");
    write_int(0, 55, 102, 0, &pad_ready[0]);
    write_int(0, 90, 102, 0, &pad_ready[1]);
    write_int(0, 125, 102, 0, &pad_ready[2]);
    write_int(0, 160, 102, 0, &pad_ready[3]);

    write(0, 10, 114, 0, "BB TL");
    write_int(0, 55, 114, 0, &bb_tl);
    write(0, 100, 114, 0, "TR");
    write_int(0, 125, 114, 0, &bb_tr);
    write(0, 170, 114, 0, "BL");
    write_int(0, 195, 114, 0, &bb_bl);
    write(0, 240, 114, 0, "BR");
    write_int(0, 265, 114, 0, &bb_br);

    write(0, 10, 126, 0, "BB X");
    write_int(0, 55, 126, 0, &bb_x);
    write(0, 110, 126, 0, "Y");
    write_int(0, 130, 126, 0, &bb_y);
    write(0, 175, 126, 0, "Rumble");
    write_int(0, 230, 126, 0, &rumble);

    write(0, 10, 150, 0, "1-4 / Left-Right: channel");
    write(0, 10, 162, 0, "Space / A: rumble");
    write(0, 10, 174, 0, "Esc / B: exit");
    write(0, 10, 186, 0, "Angle/pitch/roll = 1/1000 deg");
    write(0, 10, 198, 0, "Desktop: mouse = WPAD 0 IR");

    repeat

        if (key(_1)) chan = 0; end
        if (key(_2)) chan = 1; end
        if (key(_3)) chan = 2; end
        if (key(_4)) chan = 3; end

        if (key(_RIGHT) && k_right == 0)
            if (chan < 3) chan++; end
        end
        if (key(_LEFT) && k_left == 0)
            if (chan > 0) chan--; end
        end
        k_left = key(_LEFT);
        k_right = key(_RIGHT);

        for (i = 0; i < 4; i++)
            if (wpad_is_ready(i) > 0)
                pad_ready[i] = 1;
            else
                pad_ready[i] = 0;
            end
        end

        if (wpad_is_ready(chan) > 0)

            ready = 1;
            status = "Wiimote ready";
            batt = wpad_info(chan, WPAD_BATT);
            is_bb = wpad_info(chan, WPAD_IS_BB);
            ir_x = wpad_info(chan, WPAD_X);
            ir_y = wpad_info(chan, WPAD_Y);
            ir_z = wpad_info(chan, WPAD_Z);
            ir_angle = wpad_info(chan, WPAD_ANGLE);
            pitch = wpad_info(chan, WPAD_PITCH);
            roll = wpad_info(chan, WPAD_ROLL);
            accel_x = wpad_info(chan, WPAD_ACCELX);
            accel_y = wpad_info(chan, WPAD_ACCELY);
            accel_z = wpad_info(chan, WPAD_ACCELZ);

            if (is_bb)
                bb_tl = wpad_info_bb(chan, WPAD_WTL);
                bb_tr = wpad_info_bb(chan, WPAD_WTR);
                bb_bl = wpad_info_bb(chan, WPAD_WBL);
                bb_br = wpad_info_bb(chan, WPAD_WBR);
                bb_x = wpad_info_bb(chan, WPAD_X);
                bb_y = wpad_info_bb(chan, WPAD_Y);
            else
                bb_tl = 0;
                bb_tr = 0;
                bb_bl = 0;
                bb_br = 0;
                bb_x = 0;
                bb_y = 0;
            end

            rumble = 0;
            if (key(_SPACE) || mouse.left)
                rumble = 1;
            end
            wpad_rumble(chan, rumble);

        else

            ready = 0;
            status = "Waiting for Wiimote...";
            batt = 0;
            is_bb = 0;
            ir_x = 0;
            ir_y = 0;
            ir_z = 0;
            ir_angle = 0;
            pitch = 0;
            roll = 0;
            accel_x = 0;
            accel_y = 0;
            accel_z = 0;
            bb_tl = 0;
            bb_tr = 0;
            bb_bl = 0;
            bb_br = 0;
            bb_x = 0;
            bb_y = 0;
            if (rumble)
                wpad_rumble(chan, 0);
            end
            rumble = 0;

        end

        mx = mouse.x;
        my = mouse.y;
        mleft = mouse.left;
        mright = mouse.right;

        frame;

    until (key(_ESC) || mouse.right);

    wpad_rumble(chan, 0);

end
