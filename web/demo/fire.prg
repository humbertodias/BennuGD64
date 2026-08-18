import "mod_draw";
import "mod_video";
import "mod_key";
import "mod_rand";


global

    int fire_buffer[6000];

    int fire_intensity;

    int fire_palette[36];


function InitPalette()

begin

    /*
     * Doom Fire palette converted to RGB565.
     */

    fire_palette[0]  = 0;
    fire_palette[1]  = 2114;
    fire_palette[2]  = 6178;
    fire_palette[3]  = 10338;
    fire_palette[4]  = 14498;
    fire_palette[5]  = 18658;
    fire_palette[6]  = 22818;
    fire_palette[7]  = 26882;
    fire_palette[8]  = 33123;
    fire_palette[9]  = 37284;
    fire_palette[10] = 41476;
    fire_palette[11] = 45573;
    fire_palette[12] = 47621;
    fire_palette[13] = 53828;
    fire_palette[14] = 53860;
    fire_palette[15] = 53860;
    fire_palette[16] = 51780;
    fire_palette[17] = 51780;
    fire_palette[18] = 51847;
    fire_palette[19] = 49831;
    fire_palette[20] = 49895;
    fire_palette[21] = 49959;
    fire_palette[22] = 50026;
    fire_palette[23] = 47978;
    fire_palette[24] = 48042;
    fire_palette[25] = 48107;
    fire_palette[26] = 46091;
    fire_palette[27] = 46091;
    fire_palette[28] = 46156;
    fire_palette[29] = 46156;
    fire_palette[30] = 46221;
    fire_palette[31] = 44173;
    fire_palette[32] = 44237;
    fire_palette[33] = 44238;
    fire_palette[34] = 50543;
    fire_palette[35] = 65535;

end


function InitializeFire()

private

    int buffer_index;
    int base_x;

begin

    buffer_index = 0;

    while (buffer_index < 6000)

        fire_buffer[buffer_index] = 0;

        buffer_index = buffer_index + 1;

    end

    fire_intensity = 35;

    base_x = 0;

    while (base_x < 100)

        fire_buffer[
            59 * 100 + base_x
        ] = fire_intensity;

        base_x = base_x + 1;

    end

end


function UpdateFire()

private

    int fire_y;
    int fire_x;

    int source_index;
    int destination_index;

    int decay_value;
    int destination_x;

    int source_value;

begin

    fire_y = 1;

    while (fire_y < 60)

        fire_x = 0;

        while (fire_x < 100)

            source_index =
                fire_y * 100 + fire_x;

            decay_value =
                rand(0, 3);

            destination_x =
                fire_x - decay_value + 1;

            if (destination_x < 0)

                destination_x = 0;

            end

            if (destination_x > 99)

                destination_x = 99;

            end

            destination_index =
                (fire_y - 1) * 100 +
                destination_x;

            source_value =
                fire_buffer[source_index];

            if (source_value > decay_value)

                fire_buffer[destination_index] =
                    source_value - decay_value;

            else

                fire_buffer[destination_index] = 0;

            end

            fire_x = fire_x + 1;

        end

        fire_y = fire_y + 1;

    end

end


function UpdateBase()

private

    int base_x;

begin

    base_x = 0;

    while (base_x < 100)

        fire_buffer[
            59 * 100 + base_x
        ] = fire_intensity;

        base_x = base_x + 1;

    end

end


function DrawFire()

private

    int draw_y;
    int draw_x;

    int buffer_index;
    int palette_index;
    int sx;
    int sy;

begin

    draw_y = 0;

    while (draw_y < 60)

        draw_x = 0;

        while (draw_x < 100)

            buffer_index =
                draw_y * 100 + draw_x;

            palette_index =
                fire_buffer[buffer_index];

            if (palette_index < 0)

                palette_index = 0;

            end

            if (palette_index > 35)

                palette_index = 35;

            end

            drawing_color(
                fire_palette[palette_index]
            );

            sx = draw_x * 4;
            sy = draw_y * 4;

            draw_box(
                sx,
                sy,
                sx + 3,
                sy + 3
            );

            draw_x = draw_x + 1;

        end

        draw_y = draw_y + 1;

    end

end


Process Main()

private

    int key_value;

begin

    set_mode(
        400,
        240,
        16
    );

    set_fps(30, 0);

    InitPalette();

    InitializeFire();

    drawing_map(
        0,
        0
    );

    repeat

        if (key(_UP))

            fire_intensity =
                fire_intensity + 1;

            if (fire_intensity > 35)

                fire_intensity = 35;

            end

        end

        if (key(_DOWN))

            fire_intensity =
                fire_intensity - 1;

            if (fire_intensity < 0)

                fire_intensity = 0;

            end

        end

        if (key(_RIGHT))

            fire_intensity =
                fire_intensity + 3;

            if (fire_intensity > 35)

                fire_intensity = 35;

            end

        end

        if (key(_LEFT))

            fire_intensity =
                fire_intensity - 3;

            if (fire_intensity < 0)

                fire_intensity = 0;

            end

        end

        UpdateBase();

        UpdateFire();

        DrawFire();

        frame;

    until (key(_ESC));

end
