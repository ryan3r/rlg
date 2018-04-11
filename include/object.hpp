#pragma once
#include <parser.hpp>
#include <algorithm>
#include <iostream>

class dungeon_t;

class Object {
public:
	static constexpr size_t WEAPON = 0;
	static constexpr size_t OFFHAND = 1;
	static constexpr size_t RANGED = 2;
	static constexpr size_t ARMOR = 3;
	static constexpr size_t HELMET = 4;
	static constexpr size_t CLOAK = 5;
	static constexpr size_t GLOVES = 6;
	static constexpr size_t BOOTS = 7;
	static constexpr size_t AMULET = 8;
	static constexpr size_t LIGHT = 9;
	static constexpr size_t RING = 10;
	static constexpr size_t UNKNOWN = 11;

	std::string type;
	std::vector<std::string> color;
	std::string name;
	std::string desc;
	Dice damage;
	Dice defense;
	int32_t speed;
	int32_t weight;
	size_t inventory_type = UNKNOWN;

	Object(std::string t, std::vector<std::string> c, std::string n, std::string d, Dice da, Dice de, Dice sp, Dice w) :
		type{ t }, color{ c }, name{ n }, desc{ d }, damage{ da }, defense{ de }, speed{ sp.roll() }, weight{ w.roll() } {
		std::transform(t.begin(), t.end(), t.begin(), toupper);

		if (t == "WEAPON") inventory_type = WEAPON;
		if (t == "OFFHAND") inventory_type = OFFHAND;
		if (t == "RANGED") inventory_type = RANGED;
		if (t == "ARMOR") inventory_type = ARMOR;
		if (t == "HELMET") inventory_type = HELMET;
		if (t == "CLOAK") inventory_type = CLOAK;
		if (t == "GLOVES") inventory_type = GLOVES;
		if (t == "BOOTS") inventory_type = BOOTS;
		if (t == "AMULET") inventory_type = AMULET;
		if (t == "LIGHT") inventory_type = LIGHT;
		if (t == "RING") inventory_type = RING;
	}

	static Object * from(ObjectBuilder *builder) {
		return new Object(builder->type, builder->color, builder->name, builder->desc,
			builder->damage, builder->defense, builder->speed, builder->weight);
	}

	static void gen_objects(dungeon_t*, std::vector<std::shared_ptr<Builder>>);
	char symbol() const;
};
