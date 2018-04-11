#pragma once
#include <parser.hpp>
#include <algorithm>
#include <iostream>

class dungeon_t;

enum class ObjectType: size_t {
	WEAPON = 0,
	OFFHAND = 1,
	RANGED = 2,
	ARMOR = 3,
	HELMET = 4,
	CLOAK = 5,
	GLOVES = 6,
	BOOTS = 7,
	AMULET = 8,
	LIGHT = 9,
	RING = 10,
	UNKNOWN = 11
};

class Object {
public:
	std::string type;
	std::vector<std::string> color;
	std::string name;
	std::string desc;
	Dice damage;
	Dice defense;
	Dice speed;
	ObjectType inventory_type = ObjectType::UNKNOWN;

	Object(std::string t, std::vector<std::string> c, std::string n, std::string d, Dice da, Dice de, Dice sp) :
		type{ t }, color{ c }, name{ n }, desc{ d }, damage{ da }, defense{ de }, speed{ sp } {
		std::transform(t.begin(), t.end(), t.begin(), toupper);

		if (t == "WEAPON") inventory_type = ObjectType::WEAPON;
		if (t == "OFFHAND") inventory_type = ObjectType::OFFHAND;
		if (t == "RANGED") inventory_type = ObjectType::RANGED;
		if (t == "ARMOR") inventory_type = ObjectType::ARMOR;
		if (t == "HELMET") inventory_type = ObjectType::HELMET;
		if (t == "CLOAK") inventory_type = ObjectType::CLOAK;
		if (t == "GLOVES") inventory_type = ObjectType::GLOVES;
		if (t == "BOOTS") inventory_type = ObjectType::BOOTS;
		if (t == "AMULET") inventory_type = ObjectType::AMULET;
		if (t == "LIGHT") inventory_type = ObjectType::LIGHT;
		if (t == "RING") inventory_type = ObjectType::RING;
	}

	static Object * from(ObjectBuilder *builder) {
		return new Object(builder->type, builder->color, builder->name, builder->desc,
			builder->damage, builder->defense, builder->speed);
	}

	static void gen_objects(dungeon_t*, std::vector<std::shared_ptr<Builder>>);
	char symbol() const;
};
