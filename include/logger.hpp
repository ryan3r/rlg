#pragma once
#include <sstream>
#include <vector>
#include <stack>
#include <memory>
#include <stdint.h>

class LoggerStream;

class LoggerMsg {
public:
	std::string msg;
	int active = 2;

	LoggerMsg(std::string m) : msg{ m } {}

	// hide/destroy this message
	void hide();
};

class Logger {
private:
	static Logger *logger;

	int32_t index = -1;
	std::vector<std::shared_ptr<LoggerMsg>> messages;

public:
	static Logger *inst();

	static void del() {
		if (logger) {
			logger = nullptr;
			delete logger;
		}
	}
	
	// log messages to be removed at the end of this turn
	std::stack<std::weak_ptr<LoggerMsg>> life_time_turn;
	std::stack<std::weak_ptr<LoggerMsg>> life_time_key;

	static void clear_messages(std::stack<std::weak_ptr<LoggerMsg>> life_time);
	std::weak_ptr<LoggerMsg> log(std::string);
	void _scroll_(int32_t);
	void open_log_window();
	void print_most_recent();

	friend LoggerMsg;
};

class LoggerStream : public std::stringstream {
private:
	std::weak_ptr<LoggerMsg> *log_msg_target = nullptr;
	std::stack<std::weak_ptr<LoggerMsg>> *log_stack_target = nullptr;

public:
	LoggerStream(std::weak_ptr<LoggerMsg> &m): log_msg_target{&m} {}
	LoggerStream(std::stack<std::weak_ptr<LoggerMsg>> &s): log_stack_target{&s} {}

	~LoggerStream() {
		if (str().size() > 0) {
			auto log_msg = Logger::inst()->log(str());
			
			if (log_msg_target) {
				*log_msg_target = log_msg;
			}
			else if(log_stack_target) {
				log_stack_target->push(log_msg);
			}
		}
	}
};
