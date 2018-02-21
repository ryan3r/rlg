#include <monster.h>

void entity_move(dungeon_t *d, entity_t *ch) {
    uint8_t random_move = entity_get_attr(ch, attr_erratic) && rand_range(0, 1);

    d->entity_map[ch->y][ch->x] = NULL;

    if(entity_get_attr(ch, attr_telepathy) && !random_move) {
        corridor_path_t path = (entity_get_attr(ch, attr_tunneling) ? d->paths_tunneling : d->paths)[ch->y][ch->x];

        ch->y = path.from[dim_y];
        ch->x = path.from[dim_x];
    }
    else {
        uint8_t x = ch->x;
        uint8_t y = ch->y;
        uint8_t iter = 0;

        do {
            y = ch->y + rand_range(-1, 1);
            x = ch->x + rand_range(-1, 1);
        } while((entity_get_attr(ch, attr_tunneling) || hardnessxy(x, y) == 0) && ++iter < 200);

        if(iter < 200) {
            ch->y = y;
            ch->x = x;
        }
    }

    if(d->entity_map[ch->y][ch->x]) {
        entity_set_attr(d->entity_map[ch->y][ch->x], attr_alive, 0);
    }

    d->entity_map[ch->y][ch->x] = ch;

    // destroy any rock we move through
    if(hardnessxy(ch->x, ch->y) > 0) {
        hardnessxy(ch->x, ch->y) = 0;
        mapxy(ch->x, ch->y) = ter_floor_hall;
    }
}