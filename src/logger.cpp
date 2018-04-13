#include <logger.hpp>
#include <const.hpp>
#include <info.hpp>

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#endif

Logger *Logger::logger = nullptr;

// get/create the logger instance
Logger* Logger::inst() {
	if (!logger) {
		logger = new Logger();
	}

	return logger;
}

// add a message to the log
std::weak_ptr<LoggerMsg> Logger::log(std::string msg) {
	std::shared_ptr<LoggerMsg> ptr(new LoggerMsg(msg));

	// move to the new message
	index = messages.size();

	messages.push_back(ptr);
	print_most_recent();

	return ptr;
}

// move to the next unread message
void Logger::scroll(int32_t direction) {
	index += direction;

	if (index < 0) index = 0;
	if (index >= messages.size()) index = messages.size() - 1;
}

// print the most recent message and the count
void Logger::print_most_recent() {
	move(0, 0);
	clrtoeol();

	if (!messages.empty()) {
		attron(COLOR_PAIR(1));
		mvprintw(0, 0, "%s", messages[index]->msg.c_str());
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(5));
		mvprintw(0, DUNGEON_X - (digit_count(index + 1) + digit_count(messages.size())), "%d/%d", index + 1, messages.size());
		attroff(COLOR_PAIR(5));
	}
}

// remove all messages bound to this input loop
void Logger::clear_messages(std::stack<std::weak_ptr<LoggerMsg>> life_time) {
	while (!life_time.empty()) {
		if (auto msg = life_time.top().lock()) {
			msg->hide();
		}

		life_time.pop();
	}

	Logger::inst()->print_most_recent();
}

// hide a message
void LoggerMsg::hide() {
	if (active > 0) {
		if (--active == 0) {
			size_t i = 0;
			auto &msgs = Logger::inst()->messages;

			// find this message
			while (i < msgs.size() && msgs[i].get() != this) ++i;

			if (i == msgs.size()) return;

			// remove the message
			msgs.erase(msgs.begin() + i);

			if (msgs.size() == Logger::inst()->index) --Logger::inst()->index;
		}
	}
}

// open a window with the log message
void Logger::open_log_window() {
	std::vector<std::string> raw_messages;

	for (auto &msgs : messages) {
		raw_messages.push_back(msgs->msg);
	}

	open_list_window("Logs", raw_messages, nullptr);
}