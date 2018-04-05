#include <dungeon.hpp>
#include <object.hpp>

void Object::gen_objects(dungeon_t *d, std::vector<std::shared_ptr<Builder>> builders) {
	for (int32_t i = 0; i < rand_range(10, 20); i++) {
		pair_t p;

		// pick a location for the object
		do {
			uint32_t room = rand_range(1, d->rooms.size() - 1);
			p.y = rand_range(d->rooms[room].position.y,
				(d->rooms[room].position.y +
					d->rooms[room].size.y - 1));
			p.x = rand_range(d->rooms[room].position.x,
				(d->rooms[room].position.x +
					d->rooms[room].size.x - 1));
		} while(d->objpair(p));

		// try to create an object
		while (builders.size() > 0) {
			size_t index = rand_range(0, builders.size() - 1);

			ObjectBuilder *builder = dynamic_cast<ObjectBuilder*>(builders[index].get());

			// enforce rarity
			if (rand_range(0, 99) >= builder->rarity) continue;

			Object *obj = Object::from(builder);
			d->objpair(p) = obj;

			// make sure this object is never seen again
			if (builder->artifact) {
				builders.erase(builders.begin() + index);
			}

			break;
		}
	}
}

char Object::symbol() const {
	if (type == "WEAPON") return '|';
	if (type == "OFFHAND") return ')';
	if (type == "RANGED") return '}';
	if (type == "ARMOR") return '[';
	if (type == "HELMET") return ']';
	if (type == "CLOAK") return '(';
	if (type == "GLOVES") return '{';
	if (type == "BOOTS") return '\\';
	if (type == "RING") return '=';
	if (type == "AMULET") return '"';
	if (type == "LIGHT") return '_';
	if (type == "SCROLL") return '~';
	if (type == "BOOK") return '?';
	if (type == "FLASK") return '!';
	if (type == "GOLD") return '$';
	if (type == "AMMUNITION") return '/';
	if (type == "FOOD") return ',';
	if (type == "WAND") return '-';
	if (type == "CONTAINER") return '%';
	if (type == "STACK") return '&';

	return 'x';
}
