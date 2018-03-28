#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <tuple>

enum class FileType {
	monster,
	object
};

class ParserError: public std::exception {
private:
	std::string msg;

public:
	ParserError(const std::string s) : msg{ s } {}

	const char* what() const noexcept {
		return _strdup(msg.c_str());
	}
};

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
	std::tuple<uint32_t, uint32_t, uint32_t> parse_dice();
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
	std::tuple<uint32_t, uint32_t, uint32_t> speed;
	std::vector<std::string> abilities;
	std::tuple<uint32_t, uint32_t, uint32_t> hp;
	std::tuple<uint32_t, uint32_t, uint32_t> damage;
	uint16_t rarity;
	char symbol;

	virtual void validate();
};