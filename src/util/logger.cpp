#include <util/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
std::shared_ptr<spdlog::logger> Logger::s_logger;
void Logger::init() {
	std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Aoba.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_logger = std::make_shared<spdlog::logger>("Aoba", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_logger);
		s_logger->set_level(spdlog::level::trace);
		s_logger->flush_on(spdlog::level::trace);
}

void Logger::set_printer_mode() { s_logger->set_pattern("%v%$"); }

void Logger::set_default_mode() { s_logger->set_pattern("%n: %v%$"); }
