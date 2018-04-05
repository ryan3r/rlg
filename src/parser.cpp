#include "parser.hpp"
#include <sstream>
#include <regex>
#include <cctype>
#include <iostream>

// parse the header of a file and determine whether it is a monster or an object
FileType Parser::parse_header(std::istream &in) {
	std::string word;

	if (!(in >> word) || word != "RLG327") {
		throw ParserError("Expceted RLG327 in the header");
	}

	if (!(in >> word) || (word != "MONSTER" && word != "OBJECT")) {
		throw ParserError("Expceted OBJECT or MONSTER in header");
	}

	FileType type = word == "MONSTER" ? FileType::monster : FileType::object;

	if (!(in >> word) || word != "DESCRIPTION") {
		throw ParserError("Expceted DESCRIPTION in the header");
	}

	uint32_t version;
	if (!(in >> version)) {
		throw ParserError("Expected version");
	}

	if (version != 1) {
		throw ParserError("Version 1 is the only supported version.");
	}

	return type;
}

// Finds and consumes the next START declaration
void Parser::go_to_next_begin() {
	std::string word;

	// eat words until we reach start
	while (in >> word && word != "BEGIN");

	if (word != "BEGIN") {
		if (!in) {
			return;
		}
		else {
			throw ParserError("Expected block (aka BEGIN)");
		}
	}

	if (!(in >> word)) {
		throw ParserError("Unexpected EOF after token BEGIN");
	}

	if (word != (type == FileType::monster ? "MONSTER" : "OBJECT")) {
		std::stringstream error;

		error << "Expected " << (type == FileType::monster ? "MONSTER" : "OBJECT")
			<< " but got " << word;

		throw ParserError(error.str());
	}
}

// Parse a space separated list of words
std::vector<std::string> Parser::parse_list() {
	std::vector<std::string> words;
	std::string word, line_s;

	std::getline(in, line_s);
	std::stringstream line(line_s);

	while (line >> word) {
		// TODO: validate words

		words.push_back(word);
	}

	return words;
}

std::regex dice_expr("^\\s*(-?\\d+)\\+(-?\\d+)d(-?\\d+)\\s*$");

// Parse a dice specifier
Dice Parser::parse_dice() {
	std::string line;
	in >> line;

	std::smatch matches;
	if (!std::regex_match(line, matches, dice_expr)) {
		std::stringstream error;

		error << "Expected dice specifier in the for <base> + <dice> d <sides> but got "
				<< line;

		throw ParserError(error.str());
	}

	return Dice(stoi(matches[1].str()), stoi(matches[2].str()), stoi(matches[3].str()));
}

// Parse a block of text like the DESC field
std::string Parser::parse_text_block() {
	std::string line;
	std::stringstream text;

	// eat empty line
	std::getline(in, line);

	if (line.size() > 0) {
		throw ParserError("Expected new line before text block");
	}

	bool isFirst = true;

	// read the lines
	while (std::getline(in, line) && line != ".") {
		if (line.size() > 77) {
			throw ParserError("The maximum line length for a text block is 76 characters. In line: " + line);
		}

		if (!isFirst) text << "\n";

		text << line;

		isFirst = false;
	}

	if (!in) {
		throw ParserError("Unexpected EOF in text block");
	}

	return text.str();
}

// Parse a series of OBJECT or MONSTER blocks
std::vector<std::shared_ptr<Builder>> Parser::parse() {
	std::vector<std::shared_ptr<Builder>> builders;

	for(;;) {
		$builder = std::shared_ptr<Builder>(alloc_builder());
		go_to_next_begin();

		if (!in) break;

		std::string word = "";

		try {
			for (;;) {
				if (!(in >> word)) {
					throw ParserError("Expected field or END but found EOF");
				}

				if (word == "END") break;

				parse_field(word);
			}

			$builder->validate();

			builders.push_back($builder);
		}
		catch (const std::exception &ex) {
			std::cerr << ex.what() << std::endl;
		}
	}

	return builders;
}

// parse a file and determine if it is an OBJECT or a MONSTER
std::vector<std::shared_ptr<Builder>> Parser::parse_file(const std::string &file_name) {
	std::ifstream file(file_name);

	FileType type = parse_header(file);

	std::unique_ptr<Parser> parser(
		(type == FileType::monster) ?
			(Parser*) new MonsterParser(file) :
			(Parser*) new ObjectParser(file)
	);

	return parser->parse();
}

// pick the appropriate parser for each field
void MonsterParser::parse_field(const std::string &name) {
	MonsterBuilder *builder = (MonsterBuilder*) $builder.get();

	if (name == "DESC") {
		builder->desc = parse_text_block();
		builder->has_desc = true;
	}
	else if (name == "NAME") {
		// remove leading spaces
		while (isspace(in.peek())) in.get();

		std::getline(in, builder->name);
		builder->has_name = true;
	}
	else if (name == "COLOR") {
		builder->color = parse_list();
		builder->has_color = true;
	}
	else if (name == "SPEED") {
		builder->speed = parse_dice();
		builder->has_speed = true;
	}
	else if (name == "ABIL") {
		builder->abilities = parse_list();
		builder->has_abilities = true;
	}
	else if (name == "HP") {
		builder->hp = parse_dice();
		builder->has_hp = true;
	}
	else if (name == "DAM") {
		builder->damage = parse_dice();
		builder->has_damage = true;
	}
	else if (name == "RRTY") {
		in >> builder->rarity;
		builder->has_rarity = true;
	}
	else if (name == "SYMB") {
		in >> builder->symbol;
		builder->has_symbol = true;
	}
	else {
		throw ParserError("Unknown field name " + name);
	}
}

Builder* MonsterParser::alloc_builder() {
	return new MonsterBuilder();
}

void MonsterBuilder::validate() {
	if (!has_name) throw ParserError("Monster is missing NAME field");
	if (!has_desc) throw ParserError("Monster is missing DESC field");
	if (!has_color) throw ParserError("Monster is missing COLOR field");
	if (!has_speed) throw ParserError("Monster is missing SPEED field");
	if (!has_abilities) throw ParserError("Monster is missing ABIL field");
	if (!has_hp) throw ParserError("Monster is missing  field");
	if (!has_damage) throw ParserError("Monster is missing DAM field");
	if (!has_rarity) throw ParserError("Monster is missing RRTY field");
	if (!has_symbol) throw ParserError("Monster is missing SYMB field");
}


// pick the appropriate parser for each field
void ObjectParser::parse_field(const std::string &name) {
	ObjectBuilder *builder = (ObjectBuilder*)$builder.get();

	if (name == "NAME") {
		// remove leading spaces
		while (isspace(in.peek())) in.get();

		std::getline(in, builder->name);
		builder->has_name = true;
	}
	else if (name == "DESC") {
		builder->desc = parse_text_block();
		builder->has_desc = true;
	}
	else if (name == "TYPE") {
		// remove leading spaces
		while (isspace(in.peek())) in.get();

		std::getline(in, builder->type);
		builder->has_type = true;
	}
	else if (name == "COLOR") {
		builder->color = parse_list();
		builder->has_color = true;
	}
	else if (name == "HIT") {
		builder->hit = parse_dice();
		builder->has_hit = true;
	}
	else if (name == "DAM") {
		builder->damage = parse_dice();
		builder->has_dam = true;
	}
	else if (name == "DODGE") {
		builder->dodge = parse_dice();
		builder->has_dodge = true;
	}
	else if (name == "DEF") {
		builder->defense = parse_dice();
		builder->has_def = true;
	}
	else if (name == "WEIGHT") {
		builder->weight = parse_dice();
		builder->has_weight = true;
	}
	else if (name == "SPEED") {
		builder->weight = parse_dice();
		builder->has_speed = true;
	}
	else if (name == "ATTR") {
		builder->attr = parse_dice();
		builder->has_attr = true;
	}
	else if (name == "VAL") {
		builder->value = parse_dice();
		builder->has_val = true;
	}
	else if (name == "ART") {
		std::string word;
		in >> word;

		builder->artifact = word == "TRUE";
		builder->has_art = true;
	}
	else if (name == "RRTY") {
		in >> builder->rarity;
		builder->has_rrty = true;
	}
	else {
		throw ParserError("Unknown field name " + name);
	}
}

Builder* ObjectParser::alloc_builder() {
	return new ObjectBuilder();
}

void ObjectBuilder::validate() {
	if (!has_name) throw ParserError("Object is missing name field");
	if (!has_desc) throw ParserError("Object is missing desc field");
	if (!has_type) throw ParserError("Object is missing type field");
	if (!has_color) throw ParserError("Object is missing color field");
	if (!has_hit) throw ParserError("Object is missing hit field");
	if (!has_dam) throw ParserError("Object is missing dam field");
	if (!has_dodge) throw ParserError("Object is missing dodge field");
	if (!has_def) throw ParserError("Object is missing def field");
	if (!has_weight) throw ParserError("Object is missing weight field");
	if (!has_speed) throw ParserError("Object is missing speed field");
	if (!has_attr) throw ParserError("Object is missing attr field");
	if (!has_val) throw ParserError("Object is missing val field");
	if (!has_art) throw ParserError("Object is missing art field");
	if (!has_rrty) throw ParserError("Object is missing rrty field");
}
