#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>

namespace logger {
	enum log_level {
		LEVEL_ERROR,
		LEVEL_INFO,
		LEVEL_WARNING
	};

	struct message {
		std::string text;
		log_level level;

		std::chrono::time_point<std::chrono::system_clock> timestamp;

		message(const std::string& text_, const log_level& level_)
			: text(text_), level(level_), timestamp(std::chrono::system_clock::now())
		{}
	};

	struct logger_impl {

		template<typename ... Args>
		void emplace_back(Args&& ... args) {
			log_messages.emplace_back(std::forward<Args>(args)...);
		}

		typename std::vector<logger::message>::const_iterator begin() const {
			return log_messages.begin();
		}

		typename std::vector<logger::message>::const_iterator end() const {
			return log_messages.end();
		}

		const message& at(std::size_t index) const {
			return log_messages.at(index);
		}

		std::size_t size() const {
			return log_messages.size();
		}

	private:
		std::vector<message> log_messages;
	};

	logger_impl& instance() {
		static logger_impl log;
		return log;
	}

	namespace data {
		std::vector<message> log_messages;
	}
}

#define main_logger logger::instance()

template<typename ... Args>
void print_error(Args&& ... args) {
	std::stringstream ss("");
	(ss << ... << args);

	main_logger.emplace_back(ss.str(), logger::LEVEL_ERROR);

	std::cerr << "ERROR: " << ss.str() << '\n';
}

template<typename ... Args>
void print_info(Args&& ... args) {
	std::stringstream ss("");
	(ss << ... << args);

	main_logger.emplace_back(ss.str(), logger::LEVEL_INFO);

	std::cerr << "INFO: " << ss.str() << '\n';
}

template<typename ... Args>
void print_warning(Args&& ... args) {
	std::stringstream ss("");
	(ss << ... << args);

	main_logger.emplace_back(ss.str(), logger::LEVEL_WARNING);

	std::cerr << "WARNING: " << ss.str() << '\n';
}