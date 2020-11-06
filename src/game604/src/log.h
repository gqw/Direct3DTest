#pragma once

#include <filesystem>
#include <sstream>
#include <memory>

#ifndef NOMINMAX
#	undef min
#	undef max
#endif

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/logger.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/bundled/printf.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace fs = std::filesystem;
/// spdlog wrap class
class logger final {
public:
	/// let logger like stream
	struct log_stream : public std::ostringstream
	{
	public:
		log_stream(const spdlog::source_loc& _loc, spdlog::level::level_enum _lvl, std::string_view _prefix)
			: loc(_loc)
			, lvl(_lvl)
			, prefix(_prefix)
		{
		}

		~log_stream()
		{
			flush();
		}

		void flush()
		{
			logger::get().log(loc, lvl, (prefix + str()).c_str());
		}

	private:
		spdlog::source_loc loc;
		spdlog::level::level_enum lvl = spdlog::level::info;
		std::string prefix;
	};

public:
	static logger& get() {
		static logger logger;
		return logger;
	}

	bool init(std::string_view log_file_path) {
		if (_is_inited) return true;
		try
		{
			// check log path and try to create log directory
			fs::path log_path(log_file_path);
			fs::path log_dir = log_path.parent_path();
			if (!fs::exists(log_path)) {
				fs::create_directories(log_dir);
			}
			// initialize spdlog
			constexpr std::size_t log_buffer_size = 32 * 1024; // 32kb
			// constexpr std::size_t max_file_size = 50 * 1024 * 1024; // 50mb
			spdlog::init_thread_pool(log_buffer_size, std::thread::hardware_concurrency());
			spdlog::set_level(_log_level);
			spdlog::flush_on(_log_level);
			spdlog::set_pattern("%s(%#): [%L %D %T.%e %P %t %!] %v");
			spdlog::set_default_logger(spdlog::daily_logger_mt("daily_logger", log_path.string(), false, 2));
		}
		catch (std::exception_ptr e)
		{
			assert(false);
			return false;
		}
		_is_inited = true;
		return true;
	}

	template <typename FormatString, typename... Args>
	void log(const spdlog::source_loc& loc, spdlog::level::level_enum lvl, const FormatString& fmt, const Args &... args)
	{
		spdlog::log(loc, lvl, fmt, args...);
	}

	template <typename... Args>
	void printf(const spdlog::source_loc& loc, spdlog::level::level_enum lvl, const char* fmt, const Args &... args)
	{
		spdlog::log(loc, lvl, fmt::sprintf(fmt, args...).c_str());
	}

	spdlog::level::level_enum level() {
		return _log_level;
	}

	void set_level(spdlog::level::level_enum lvl) {
		_log_level = lvl;
		spdlog::set_level(lvl);
		spdlog::flush_on(lvl);
	}

	static std::size_t get_filename_pos(std::string_view path) {
		if (path.empty())
			return 0;

		size_t pos = path.find_last_of("/\\");
		return (pos == path.npos) ? 0 : pos + 1;
	}

	static std::string wstr_to_utf8(std::wstring_view wstr)
	{
		std::string target;
		if (wstr.size() > static_cast<size_t>((std::numeric_limits<int>::max)()))
		{
			std::logic_error("UTF-16 string is too big to be converted to UTF-8");
		}

		int wstr_size = static_cast<int>(wstr.size());
		if (wstr_size == 0)
		{
			target.resize(0);
			return target;
		}

		int result_size = static_cast<int>(target.capacity());
		if ((wstr_size + 1) * 3 > result_size)
		{
			result_size = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr_size, NULL, 0, NULL, NULL);
		}

		if (result_size > 0)
		{
			target.resize(result_size);
			result_size = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr_size, (LPSTR)target.data(), result_size, NULL, NULL);

			if (result_size > 0)
			{
				target.resize(result_size);
				return target;
			}
		}

		throw std::logic_error(fmt::format("WideCharToMultiByte failed. Last error: {}", ::GetLastError()));
	}

private:
	logger() = default;
	~logger() = default;

	logger(const logger&) = delete;
	void operator=(const logger&) = delete;

private:
	std::atomic_bool _is_inited = false;
	spdlog::level::level_enum _log_level = spdlog::level::info;
};

class FunctionTracer
{
public:
	template<typename... Args>
	FunctionTracer(const char* file, int line, const char* func, const char* msg, const Args &... args)
		: file_(file)
		, line_(line)
		, func_(func)
	{
		startTime_ = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		std::string fmt = msg;
		if (fmt.empty())
		{
			fmt = "Enter function";
		}
		else
		{
			fmt = std::string("Enter function, ") + msg;
		}
		spdlog::log({ file_, line_, func_ }, spdlog::level::trace, fmt.c_str(), args...);
	}
	~FunctionTracer()
	{

		auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		spdlog::log({ file_, line_, func_ }, spdlog::level::trace, "Leave function, cost: {}", now - startTime_);
	}

	static size_t GetFileNamePos(const char* path)
	{
		if (path == nullptr)
			return 0;

		std::string file(path);
		size_t pos = file.find_last_of("/\\");
		return (pos == file.npos) ? 0 : pos + 1;
	}
private:
	const char* file_ = nullptr;
	int line_ = 0;
	const char* func_ = nullptr;
	uint64_t startTime_ = 0;
};

// got short filename(exlude file directory)
#define __FILENAME__ (__FILE__ + logger::get_filename_pos(__FILE__))

#ifdef PRINT_LOG_TRACE
#define  TRACE_FUNC()  FunctionTracer __log_function_tracer(__FILENAME__, __LINE__, __FUNCTION__, ""); 
#define  DEBUG_FUNC()  FunctionTracer __log_function_tracer(__FILENAME__, __LINE__, __FUNCTION__, ""); 
#define  TRACE_FUNC_EXT(msg, ...)  FunctionTracer __log_function_tracer(__FILENAME__, __LINE__, __FUNCTION__, msg, __VA_ARGS__); 
#else 
#define TRACE_FUNC()
#endif // PRINT_LOG_TRACE

// use fmt lib, e.g. LOG_WARN("warn log, {1}, {1}, {2}", 1, 2);
#define LOG_TRACE(msg,...) { if (logger::get().getLogLevel() == spdlog::level::trace) spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::trace, msg, ##__VA_ARGS__); };
#define LOG_DEBUG(msg,...) spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::debug, msg, ##__VA_ARGS__);
#define LOG_INFO(msg,...) spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::info, msg, ##__VA_ARGS__);
#define LOG_WARN(msg,...) spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::warn, msg, ##__VA_ARGS__);
#define LOG_ERROR(msg,...) spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::err, msg, ##__VA_ARGS__);
#define LOG_FATAL(msg,...) spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::critical, msg, ##__VA_ARGS__);

// use like sprintf, e.g. PRINT_WARN("warn log, %d-%d", 1, 2);
#define PRINT_TRACE(msg,...) logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::trace, msg, ##__VA_ARGS__);
#define PRINT_DEBUG(msg,...) logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::debug, msg, ##__VA_ARGS__);
#define PRINT_INFO(msg,...) logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::info, msg, ##__VA_ARGS__);
#define PRINT_WARN(msg,...) logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::warn, msg, ##__VA_ARGS__);
#define PRINT_ERROR(msg,...) logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::err, msg, ##__VA_ARGS__);
#define PRINT_FATAL(msg,...) logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::critical, msg, ##__VA_ARGS__);

// use like stream , e.g. STM_WARN() << "warn log: " << 1;
#define STM_TRACE() logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::trace, "")
#define STM_DEBUG() logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::debug, "")
#define STM_INFO()	logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::info, "")
#define STM_WARN()	logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::warn, "")
#define STM_ERROR() logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::err, "")
#define STM_FATAL() logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::critical, "")