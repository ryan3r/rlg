#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

typedef struct {
    uint32_t attrs;
    uint8_t x;
    uint8_t y;
    uint8_t speed;
    int32_t turn;
} entity_t;

typedef enum {
    attr_intelligence = 1,
    attr_telepathy = 2,
    attr_tunneling = 4,
    attr_erratic = 8,
    attr_pc = 16,
    attr_alive = 32
} attribute_t;

#define entity_get_attr(ch, attr) ((ch)->attrs & attr)
#define entity_toggle_attr(ch, attr) (((ch)->attrs ^= attr) & attr)

#define entity_set_attr(ch, attr, value) \
    (value ? ((ch)->attrs |= attr) : (ch->attrs &= ~attr));

void entity_init(entity_t *ch, uint8_t is_pc);

#endif