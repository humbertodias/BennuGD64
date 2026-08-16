import "mod_video";
import "mod_draw";
import "mod_key";
import "mod_rand";
import "mod_map";

#define W 320
#define H 200
#define SCALE 2
#define STARS 600

global
    float star_x[STARS];
    float star_y[STARS];
    float star_z[STARS];

    float speed;

process Main()

private
    int screen_map;
    int i;
    int star_sx;
    int star_sy;
    int bright;
    float fov;

begin

    set_mode(W * SCALE, H * SCALE, 16);

    speed = 0.5;
    fov = 120.0;

    /*
     * Cria mapa 8-bit.
     */
    screen_map = new_map(
        W * SCALE,
        H * SCALE,
        8
    );

    /*
     * O mapa é o gráfico deste processo.
     */
    graph = screen_map;

    /*
     * Coloca o centro do mapa no centro da tela.
     */
    x = W * SCALE / 2;
    y = H * SCALE / 2;

    /*
     * Inicializa estrelas.
     */
    for (i = 0; i < STARS; i++)

        star_x[i] =
            rand(-W / 2, W / 2 - 1);

        star_y[i] =
            rand(-H / 2, H / 2 - 1);

        star_z[i] =
            rand(1, W);

    end


    /*
     * Loop principal.
     */
    while (!key(_esc))

        /*
         * Aumenta velocidade.
         */
        if (key(_up))
            speed += 0.2;
        end

        /*
         * Diminui velocidade.
         */
        if (key(_down))

            if (speed > 0.2)
                speed -= 0.2;
            end

        end


        /*
         * Limpa o mapa.
         */
        map_clear(
            0,
            screen_map,
            0
        );


        /*
         * Atualiza estrelas.
         */
        for (i = 0; i < STARS; i++)

            /*
             * Move a estrela em direção ao observador.
             */
            star_z[i] -= speed;


            /*
             * Chegou perto demais.
             */
            if (star_z[i] <= 0.1)

                star_x[i] =
                    rand(-W / 2, W / 2 - 1);

                star_y[i] =
                    rand(-H / 2, H / 2 - 1);

                star_z[i] =
                    rand(1, W);

            end


            /*
             * Projeção 3D -> 2D.
             */
            star_sx =
                (star_x[i] / star_z[i])
                * fov
                + W / 2;

            star_sy =
                (star_y[i] / star_z[i])
                * fov
                + H / 2;


            /*
             * Fora da área visível.
             */
            if (star_sx < 0 ||
                star_sx >= W ||
                star_sy < 0 ||
                star_sy >= H)

                star_x[i] =
                    rand(-W / 2, W / 2 - 1);

                star_y[i] =
                    rand(-H / 2, H / 2 - 1);

                star_z[i] =
                    rand(1, W);

            else

                /*
                 * Calcula brilho.
                 */
                bright =
                    255
                    * (1.0 - star_z[i] / W);


                if (bright < 50)
                    bright = 50;
                end

                if (bright > 255)
                    bright = 255;
                end


                /*
                 * Converte para resolução SCALE.
                 */
                star_sx =
                    star_sx * SCALE;

                star_sy =
                    star_sy * SCALE;


                /*
                 * Estrela 3x3.
                 */

                map_put_pixel(
                    0,
                    screen_map,
                    star_sx,
                    star_sy,
                    bright
                );

                map_put_pixel(
                    0,
                    screen_map,
                    star_sx + 1,
                    star_sy,
                    bright
                );

                map_put_pixel(
                    0,
                    screen_map,
                    star_sx + 2,
                    star_sy,
                    bright
                );


                map_put_pixel(
                    0,
                    screen_map,
                    star_sx,
                    star_sy + 1,
                    bright
                );

                map_put_pixel(
                    0,
                    screen_map,
                    star_sx + 1,
                    star_sy + 1,
                    bright
                );

                map_put_pixel(
                    0,
                    screen_map,
                    star_sx + 2,
                    star_sy + 1,
                    bright
                );


                map_put_pixel(
                    0,
                    screen_map,
                    star_sx,
                    star_sy + 2,
                    bright
                );

                map_put_pixel(
                    0,
                    screen_map,
                    star_sx + 1,
                    star_sy + 2,
                    bright
                );

                map_put_pixel(
                    0,
                    screen_map,
                    star_sx + 2,
                    star_sy + 2,
                    bright
                );

            end

        end


        /*
         * Aguarda próximo frame.
         */
        frame;

    end


    /*
     * Libera o mapa.
     */
    unload_map(
        0,
        screen_map
    );

end
