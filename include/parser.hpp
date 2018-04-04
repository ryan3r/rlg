#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <tuple>
#include <dice.hpp>
#include <functional>

enum class FileType {
	monster,
	object
};

class ParserError : public RlgError {};

class Builder {
public:
	virtual void validate() = 0;
};

class Parser {
private:
	static FileType parse_header(std::istream &in);
	void go_to_next_begin(); // also consumes next START declaration

protected:
	std::istream & in;
	FileType type;
	std::shared_ptr<Builder> $builder;

	Parser(std::istream &i, FileType t) : in{ i }, type{ t } {}

	std::vector<std::string> parse_list();
	Dice parse_dice();
	std::string parse_text_block();

	virtual void parse_field(const std::string&) = 0;
	virtual Builder* alloc_builder() = 0;

	std::vector<std::shared_ptr<Builder>> parse();

public:
	static std::vector<std::shared_ptr<Builder>> parse_file(const std::string&);
};

class MonsterParser : public Parser {
	virtual void parse_field(const std::string&);
	virtual Builder* alloc_builder();

public:
	MonsterParser(std::istream &i) : Parser(i, FileType::monster) {}
};

class MonsterBuilder: public Builder {
public:
	bool has_name = false;
	bool has_desc = false;
	bool has_color = false;
	bool has_speed = false;
	bool has_abilities = false;
	bool has_hp = false;
	bool has_damage = false;
	bool has_rarity = false;
	bool has_symbol = false;

	std::string name;
	std::string desc;
	std::vector<std::string> color;
	Dice speed;
	std::vector<std::string> abilities;
	Dice hp;
	Dice damage;
	uint16_t rarity;
	char symbol;

	virtual void validate();
};

class ObjectParser : public Parser {
	virtual void parse_field(const std::string&);
	virtual Builder* alloc_builder();

public:
	ObjectParser(std::istream &i) : Parser(i, FileType::object) {}
};

class ObjectBuilder : public Builder {
public:
	bool has_name = false;
	bool has_desc = false;
	bool has_type = false;
	bool has_color = false;
	bool has_hit = false;
	bool has_dam = false;
	bool has_dodge = false;
	bool has_def = false;
	bool has_weight = false;
	bool has_speed = false;
	bool has_attr = false;
	bool has_val = false;
	bool has_art = false;
	bool has_rrty = false;

	std::string name;
	std::string desc;
	std::string type;
	std::vector<std::string> color;
	Dice hit;
	Dice damage;
	Dice dodge;
	Dice defense;
	Dice weight;
	Dice speed;
	Dice attr;
	Dice value;
	bool artifact;
	uint16_t rarity;

	virtual void validate();
};
