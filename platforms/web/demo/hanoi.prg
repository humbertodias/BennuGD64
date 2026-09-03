import "mod_draw";
import "mod_key";
import "mod_video";
import "mod_text";
import "mod_screen";
import "mod_map";
import "mod_proc";

#define NUM_DISKS 4
#define MIN_MOVES 15

global
    int tower[3][NUM_DISKS];
    int tower_count[3];
    int moves;
    int selected_tower;
    int source_tower;
    int selecting_destination;
    int auto_solve;
end


process main()
begin
    set_mode(640, 480, 16);
    set_fps(30, 0);

    /* Redraw the whole frame. Default partial restore leaves ghosts. */
    restore_type = COMPLETE_RESTORE;
    dump_type = COMPLETE_DUMP;

    set_text_color(rgb(255, 255, 255));

    reset_game();
    board();

    loop
        if (auto_solve == 0)
            if (key(_left))
                selected_tower--;
                if (selected_tower < 0)
                    selected_tower = 2;
                end
                while (key(_left))
                    frame;
                end
            end

            if (key(_right))
                selected_tower++;
                if (selected_tower > 2)
                    selected_tower = 0;
                end
                while (key(_right))
                    frame;
                end
            end

            if (key(_enter))
                if (selecting_destination == 0)
                    if (tower_count[selected_tower] > 0)
                        source_tower = selected_tower;
                        selecting_destination = 1;
                    end
                else
                    if (source_tower != selected_tower)
                        move_disk(source_tower, selected_tower);
                    end
                    source_tower = -1;
                    selecting_destination = 0;
                end
                while (key(_enter))
                    frame;
                end
            end

            if (key(_space))
                reset_game();
                auto_solve = 1;
                solve_hanoi(NUM_DISKS, 0, 1, 2);
                auto_solve = 0;
                while (key(_space))
                    frame;
                end
            end
        end

        if (key(_esc))
            exit();
        end

        frame;
    end
end


/* Draws every FRAME, including while main() is inside solve_hanoi(). */
process board()
private
    int i;
    int j;
    int pos_x;
    int pos_y;
    int disk;
begin
    drawing_map(0, 0);

    loop
        clear_screen();
        delete_text(ALL_TEXT);

        drawing_color(rgb(70, 70, 70));
        draw_box(80, 362, 560, 372);
        draw_box(154, 190, 166, 365);
        draw_box(314, 190, 326, 365);
        draw_box(474, 190, 486, 365);

        for (i = 0; i < 3; i++)
            for (j = 0; j < tower_count[i]; j++)
                /* Top disk of the source peg is drawn in-hand, not on the stack. */
                if (selecting_destination && i == source_tower && j == tower_count[i] - 1)
                    continue;
                end

                disk = tower[i][j];
                pos_x = 160 + (i * 160);
                pos_y = 350 - (j * 30);
                paint_disk(pos_x, pos_y, disk, 0);
            end
        end

        if (selecting_destination && source_tower >= 0 && tower_count[source_tower] > 0)
            disk = tower[source_tower][tower_count[source_tower] - 1];
            pos_x = 160 + (selected_tower * 160);
            paint_disk(pos_x, 165, disk, 1);
        end

        write(0, 320, 40, ALIGN_CENTER, "Tower of Hanoi");
        write(0, 320, 64, ALIGN_CENTER, "moves " + moves + "/" + MIN_MOVES);

        write(0, 160, 385, ALIGN_CENTER, "A");
        write(0, 320, 385, ALIGN_CENTER, "B");
        write(0, 480, 385, ALIGN_CENTER, "C");

        if (selecting_destination == 0)
            drawing_color(rgb(40, 100, 180));
            draw_box(125 + selected_tower * 160, 405, 195 + selected_tower * 160, 418);
        else
            drawing_color(rgb(50, 160, 90));
            draw_box(125 + source_tower * 160, 405, 195 + source_tower * 160, 418);
            drawing_color(rgb(180, 100, 50));
            draw_box(125 + selected_tower * 160, 422, 195 + selected_tower * 160, 435);
        end

        if (auto_solve)
            write(0, 320, 450, ALIGN_CENTER, "solving...");
        elseif (selecting_destination == 0)
            write(0, 320, 450, ALIGN_CENTER, "LEFT/RIGHT  ENTER: select   SPACE: solve   ESC: quit");
        else
            write(0, 320, 450, ALIGN_CENTER, "LEFT/RIGHT  ENTER: move");
        end

        if (key(_esc))
            exit();
        end

        frame;
    end
end


function paint_disk(pos_x, pos_y, disk, lifted)
private
    int disk_width;
begin
    disk_width = 25 + (disk * 20);

    if (disk == 1)
        drawing_color(rgb(90, 70, 170));
    elseif (disk == 2)
        drawing_color(rgb(100, 75, 180));
    elseif (disk == 3)
        drawing_color(rgb(110, 80, 190));
    else
        drawing_color(rgb(200, 90, 70));
    end

    draw_box(pos_x - disk_width, pos_y - 12, pos_x + disk_width, pos_y + 12);

    if (lifted)
        drawing_color(rgb(255, 255, 80));
        draw_rect(pos_x - disk_width - 2, pos_y - 14, pos_x + disk_width + 2, pos_y + 14);
    end

    write(0, pos_x, pos_y - 4, ALIGN_CENTER, "" + disk);
end


function reset_game()
private
    int i;
begin
    tower_count[0] = NUM_DISKS;
    tower_count[1] = 0;
    tower_count[2] = 0;

    for (i = 0; i < NUM_DISKS; i++)
        tower[0][i] = NUM_DISKS - i;
    end

    moves = 0;
    selected_tower = 0;
    source_tower = -1;
    selecting_destination = 0;
end


function move_disk(from_tower, to_tower)
private
    int disk;
begin
    if (tower_count[from_tower] <= 0)
        return;
    end

    disk = tower[from_tower][tower_count[from_tower] - 1];

    if (tower_count[to_tower] > 0)
        if (tower[to_tower][tower_count[to_tower] - 1] < disk)
            return;
        end
    end

    tower_count[from_tower]--;
    tower[to_tower][tower_count[to_tower]] = disk;
    tower_count[to_tower]++;
    moves++;
end


function solve_hanoi(number_disks, from_tower, auxiliary_tower, to_tower)
private
    int wait_frame;
begin
    if (number_disks <= 0)
        return;
    end

    solve_hanoi(number_disks - 1, from_tower, to_tower, auxiliary_tower);
    move_disk(from_tower, to_tower);

    for (wait_frame = 0; wait_frame < 12; wait_frame++)
        frame;
    end

    solve_hanoi(number_disks - 1, auxiliary_tower, from_tower, to_tower);
end
