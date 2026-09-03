import "mod_draw";
import "mod_video";
import "mod_key";
import "mod_rand";


global

    int rain_x[400];
    int rain_y[400];

    /*
     * Fixed point.
     *
     * 10 = 1.0 pixel
     * 15 = 1.5 pixels
     * 44 = 4.4 pixels
     */
    int rain_speed[400];

    int rain_len[400];

    /*
     * Different brightness levels.
     */
    int rain_brightness[400];


function ResetDrop(int i)

begin

    /*
     * Horizontal position.
     */
    rain_x[i] =
        rand(0, 319);


    /*
     * Start above the screen.
     *
     * Y is stored in fixed point.
     */
    rain_y[i] =
        -(rand(0, 199) * 10);


    /*
     * Original C:
     *
     * 1.5 + random(0..2.9)
     *
     * Fixed point:
     *
     * 15 .. 44
     */
    rain_speed[i] =
        rand(15, 44);


    /*
     * Original C:
     *
     * 4 + rand() % 6
     *
     * = 4 .. 9
     */
    rain_len[i] =
        rand(4, 9);


    /*
     * Random brightness.
     */
    rain_brightness[i] =
        rand(0, 3);

end


Process Main()

private
    int i;

begin

    /*
     * =================================
     * VIDEO
     * =================================
     */

    set_mode(
        320,
        200,
        16
    );


    /*
     * =================================
     * INITIALIZE RAIN
     * =================================
     */

    i = 0;

    while (i < 400)

        ResetDrop(i);

        i = i + 1;

    end


    /*
     * Draw directly to the screen.
     */
    drawing_map(0, 0);


    /*
     * =================================
     * MAIN LOOP
     * =================================
     */

    repeat


        /*
         * =================================
         * BACKGROUND
         * =================================
         *
         * Dark blue:
         *
         * RGB(10, 10, 20)
         */
        drawing_color(2114);

        draw_box(
            0,
            0,
            319,
            199
        );


        /*
         * =================================
         * RAIN
         * =================================
         */

        i = 0;

        while (i < 400)


            /*
             * Move using fixed point.
             */
            rain_y[i] =
                rain_y[i] +
                rain_speed[i];


            /*
             * Reset after leaving screen.
             *
             * 200 pixels = 2000 fixed-point units.
             */
            if (rain_y[i] > 2000)

                ResetDrop(i);

            end


            /*
             * =================================
             * DROP COLOR
             * =================================
             *
             * Several shades of blue.
             */


            if (rain_brightness[i] == 0)

                /*
                 * Dark blue.
                 */
                drawing_color(4640);

            end


            if (rain_brightness[i] == 1)

                /*
                 * Medium blue.
                 */
                drawing_color(11615);

            end


            if (rain_brightness[i] == 2)

                /*
                 * Normal blue.
                 */
                drawing_color(32255);

            end


            if (rain_brightness[i] == 3)

                /*
                 * Bright blue.
                 */
                drawing_color(40191);

            end


            /*
             * =================================
             * DRAW DROP
             * =================================
             *
             * Convert fixed-point Y to
             * screen coordinates only here.
             */

            draw_line(
                rain_x[i],
                rain_y[i] / 10,
                rain_x[i],
                (rain_y[i] / 10) + rain_len[i]
            );


            i = i + 1;

        end


        /*
         * =================================
         * NEXT FRAME
         * =================================
         */

        frame;


    until (key(_ESC));

end
