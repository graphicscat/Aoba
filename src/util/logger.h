#pragma once
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma warning(pop)
#include <memory>
#include <stdexcept>
class Logger {
   public:
	static void init();
	static void set_printer_mode();
	static void set_default_mode();
	inline static std::shared_ptr<spdlog::logger>& get_logger() { return s_logger; }

   private:
	static std::shared_ptr<spdlog::logger> s_logger;
};

//#define LUMEN_TRACE(...) Logger::get_logger()->trace(__VA_ARGS__)
// #ifdef _DEBUG
#define LOG_INFO(...) Logger::get_logger()->info(__VA_ARGS__)
#define LOG_WARN(...) Logger::get_logger()->warn(__VA_ARGS__)
#define LOG_TRACE(...) Logger::get_logger()->trace(__VA_ARGS__)
#define LOG_ERROR(...) Logger::get_logger()->error(__VA_ARGS__)