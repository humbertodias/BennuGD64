import "mod_draw";
import "mod_video";
import "mod_key";
import "mod_mouse";
import "mod_rand";
import "mod_text";


global

    int rocket_x[5];
    int rocket_y[5];

    int rocket_vx[5];
    int rocket_vy[5];

    int rocket_target[5];
    int rocket_active[5];

    int particle_x[500];
    int particle_y[500];

    int particle_old_x[500];
    int particle_old_y[500];

    int particle_vx[500];
    int particle_vy[500];

    int particle_gravity[500];

    int particle_life[500];
    int particle_active[500];

    int particle_color[500];


function CreateRocket()

private
    int i;
    int direction;

begin

    i = 0;

    while (i < 5)

        if (rocket_active[i] == 0)

            /*
             * Random launch position.
             */
            rocket_x[i] = rand(80, 720);
            rocket_y[i] = 590;


            /*
             * Vertical speed.
             */
            rocket_vy[i] = rand(7, 11);


            /*
             * Horizontal speed.
             *
             * This creates different launch angles.
             */
            direction = rand(-4, 4);

            if (direction == 0)

                direction = rand(-2, 2);

            end

            rocket_vx[i] = direction;


            /*
             * Random explosion height.
             */
            rocket_target[i] =
                rand(120, 360);


            rocket_active[i] = 1;


            /*
             * Stop searching.
             */
            i = 5;

        else

            i = i + 1;

        end

    end

end


function CreateExplosion(int x, int y)

private
    int i;
    int p;
    int index;

    int vx;
    int vy;

    int speed;
    int count;

begin

    /*
     * Different explosions have
     * different numbers of particles.
     */
    count = rand(35, 100);

    p = 0;


    while (p < count)

        index = -1;


        /*
         * Find an unused particle.
         */
        i = 0;

        while (i < 500)

            if (particle_active[i] == 0)

                index = i;

                i = 500;

            else

                i = i + 1;

            end

        end


        if (index >= 0)

            /*
             * Particles don't start at exactly
             * the same position.
             */
            particle_x[index] =
                x + rand(-3, 3);

            particle_y[index] =
                y + rand(-3, 3);


            particle_old_x[index] =
                particle_x[index];

            particle_old_y[index] =
                particle_y[index];


            /*
             * Completely random initial
             * direction.
             */
            vx = rand(-10, 10);
            vy = rand(-10, 10);


            /*
             * Different particles have
             * different energy.
             */
            speed = rand(1, 5);


            particle_vx[index] =
                vx * speed / 3;

            particle_vy[index] =
                vy * speed / 3;


            /*
             * Prevent stationary particles.
             */
            if (particle_vx[index] == 0)

                particle_vx[index] =
                    rand(-2, 2);

                if (particle_vx[index] == 0)

                    particle_vx[index] = 1;

                end

            end


            if (particle_vy[index] == 0)

                particle_vy[index] =
                    rand(-2, 2);

                if (particle_vy[index] == 0)

                    particle_vy[index] = -1;

                end

            end


            /*
             * Some particles receive
             * an additional impulse.
             */
            if (rand(0, 3) == 0)

                particle_vx[index] =
                    particle_vx[index] +
                    rand(-3, 3);

                particle_vy[index] =
                    particle_vy[index] +
                    rand(-3, 3);

            end


            /*
             * Each particle has its own gravity.
             */
            particle_gravity[index] =
                rand(1, 3);


            /*
             * Each particle has its own lifetime.
             */
            particle_life[index] =
                rand(20, 80);


            particle_active[index] = 1;


            /*
             * Random color.
             */
            particle_color[index] =
                rand(0, 5);

        end


        p = p + 1;

    end

end


Process Main()

private
    int i;

    int timer;
    int next_rocket;

    int mouse_was_down;

    int intro_text;
    int intro_timer;

begin

    /*
     * =================================
     * VIDEO
     * =================================
     */

    set_mode(
        800,
        600,
        16
    );


    /*
     * =================================
     * INITIALIZE ROCKETS
     * =================================
     */

    i = 0;

    while (i < 5)

        rocket_active[i] = 0;

        i = i + 1;

    end


    /*
     * =================================
     * INITIALIZE PARTICLES
     * =================================
     */

    i = 0;

    while (i < 500)

        particle_active[i] = 0;

        i = i + 1;

    end


    /*
     * =================================
     * DRAWING
     * =================================
     */

    drawing_map(0, 0);


    /*
     * =================================
     * INTRO SCREEN
     * =================================
     */

    drawing_color(0);

    draw_box(
        0,
        0,
        799,
        599
    );


    /*
     * Instruction.
     */
    intro_text = write(
        0,
        400,
        280,
        4,
        "Click anywhere to create fireworks!"
    );


    /*
     * Wait for up to 2 seconds.
     *
     * 60 FPS * 2 seconds = 120 frames.
     */
    intro_timer = 0;

    mouse_was_down = 0;


    while (intro_timer < 120)

        /*
         * Clicking skips the intro.
         */
        if (mouse.left == 1)

            intro_timer = 120;

        end


        frame;

        intro_timer = intro_timer + 1;

    end


    /*
     * Remove intro text.
     */
    delete_text(
        intro_text
    );


    /*
     * If the user clicked to skip
     * the intro, wait for release.
     *
     * This prevents that same click
     * from creating a firework.
     */
    while (mouse.left == 1)

        frame;

    end


    mouse_was_down = 0;


    /*
     * =================================
     * INITIAL TIMER
     * =================================
     */

    timer = 0;

    next_rocket = rand(10, 40);


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
         */

        drawing_color(0);

        draw_box(
            0,
            0,
            799,
            599
        );


        /*
         * =================================
         * MOUSE FIREWORK
         * =================================
         */

        if (mouse.left == 1)

            /*
             * Detect only the initial click.
             */
            if (mouse_was_down == 0)

                CreateExplosion(
                    mouse.x,
                    mouse.y
                );

            end


            mouse_was_down = 1;

        else

            mouse_was_down = 0;

        end


        /*
         * =================================
         * AUTOMATIC ROCKETS
         * =================================
         */

        timer = timer + 1;


        if (timer >= next_rocket)

            CreateRocket();

            timer = 0;

            /*
             * Random interval.
             */
            next_rocket =
                rand(8, 45);

        end


        /*
         * =================================
         * ROCKETS
         * =================================
         */

        i = 0;


        while (i < 5)

            if (rocket_active[i] == 1)

                /*
                 * Rocket head.
                 */
                drawing_color(16777215);

                draw_box(
                    rocket_x[i] - 2,
                    rocket_y[i] - 2,
                    rocket_x[i] + 2,
                    rocket_y[i] + 2
                );


                /*
                 * Rocket trail.
                 *
                 * It follows the direction
                 * of the rocket.
                 */
                drawing_color(8421504);

                draw_line(
                    rocket_x[i],
                    rocket_y[i],
                    rocket_x[i] -
                        rocket_vx[i] * 3,
                    rocket_y[i] +
                        rocket_vy[i] * 3
                );


                /*
                 * Horizontal movement.
                 */
                rocket_x[i] =
                    rocket_x[i] +
                    rocket_vx[i];


                /*
                 * Vertical movement.
                 */
                rocket_y[i] =
                    rocket_y[i] -
                    rocket_vy[i];


                /*
                 * Occasionally change direction.
                 */
                if (rand(0, 20) == 0)

                    rocket_vx[i] =
                        rocket_vx[i] +
                        rand(-1, 1);

                end


                /*
                 * Limit horizontal speed.
                 */
                if (rocket_vx[i] > 5)

                    rocket_vx[i] = 5;

                end


                if (rocket_vx[i] < -5)

                    rocket_vx[i] = -5;

                end


                /*
                 * Explode at target height.
                 */
                if (rocket_y[i] <= rocket_target[i])

                    CreateExplosion(
                        rocket_x[i],
                        rocket_y[i]
                    );

                    rocket_active[i] = 0;

                end


                /*
                 * Remove rocket if it
                 * leaves the screen.
                 */
                if (rocket_x[i] < 0)

                    rocket_active[i] = 0;

                end


                if (rocket_x[i] > 800)

                    rocket_active[i] = 0;

                end

            end


            i = i + 1;

        end


        /*
         * =================================
         * PARTICLES
         * =================================
         */

        i = 0;


        while (i < 500)

            if (particle_active[i] == 1)

                /*
                 * Save previous position.
                 */
                particle_old_x[i] =
                    particle_x[i];

                particle_old_y[i] =
                    particle_y[i];


                /*
                 * Move particle.
                 */
                particle_x[i] =
                    particle_x[i] +
                    particle_vx[i];

                particle_y[i] =
                    particle_y[i] +
                    particle_vy[i];


                /*
                 * Gravity.
                 */
                particle_vy[i] =
                    particle_vy[i] +
                    particle_gravity[i];


                /*
                 * =================================
                 * COLORS
                 * =================================
                 */

                if (particle_color[i] == 0)

                    drawing_color(16753920);

                end


                if (particle_color[i] == 1)

                    drawing_color(65535);

                end


                if (particle_color[i] == 2)

                    drawing_color(16711935);

                end


                if (particle_color[i] == 3)

                    drawing_color(16776960);

                end


                if (particle_color[i] == 4)

                    drawing_color(16777215);

                end


                if (particle_color[i] == 5)

                    drawing_color(65280);

                end


                /*
                 * Particle trail.
                 */
                draw_line(
                    particle_old_x[i],
                    particle_old_y[i],
                    particle_x[i],
                    particle_y[i]
                );


                /*
                 * Particle head.
                 */
                draw_box(
                    particle_x[i] - 1,
                    particle_y[i] - 1,
                    particle_x[i] + 1,
                    particle_y[i] + 1
                );


                /*
                 * Lifetime.
                 */
                particle_life[i] =
                    particle_life[i] - 1;


                if (particle_life[i] <= 0)

                    particle_active[i] = 0;

                end


                /*
                 * Remove particles outside screen.
                 */
                if (particle_y[i] > 600)

                    particle_active[i] = 0;

                end


                if (particle_x[i] < 0)

                    particle_active[i] = 0;

                end


                if (particle_x[i] > 800)

                    particle_active[i] = 0;

                end

            end


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
