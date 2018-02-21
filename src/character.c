#include <character.h>
#include <macros.h>
#include <dungeon.h>

void entity_init(entity_t *ch, uint8_t is_pc) {
    ch->attrs = 0;
    ch->turn = 0;
    entity_set_attr(ch, attr_alive, 1);

    if(is_pc) {
        entity_set_attr(ch, attr_pc, 1);
        ch->speed = 10;
        return;
    }

    ch->speed = rand_range(5, 20);

    entity_set_attr(ch, attr_intelligence, rand_range(0, 1));
    entity_set_attr(ch, attr_telepathy, rand_range(0, 1));
    entity_set_attr(ch, attr_tunneling, rand_range(0, 1));
    entity_set_attr(ch, attr_erratic, rand_range(0, 1));
}
