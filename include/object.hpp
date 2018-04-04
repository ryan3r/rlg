#pragma once
#include <parser.hpp>

#include <algorithm>

class dungeon_t;

class Object {
public:
	std::string type;
	std::vector<std::string> color;

	Object(std::string t, std::vector<std::string> c) : type{ t }, color { c } {
		std::transform(type.begin(), type.end(), type.begin(), toupper);
	}

	static Object * from(ObjectBuilder *builder) {
		return new Object(builder->type, builder->color);
	}

	static void gen_objects(dungeon_t*, std::vector<std::shared_ptr<Builder>>);
	char symbol() const;
};
