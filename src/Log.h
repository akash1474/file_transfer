#pragma once

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <memory>
#pragma warning(pop)

namespace FTransfer {
	class Log
	{
	public:
		static void Init();
		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
	};
};

// Core log macros
#ifdef FT_DEBUG
	#define FT_TRACE(...)    ::FTransfer::Log::GetCoreLogger()->trace(__VA_ARGS__)
	#define FT_INFO(...)     ::FTransfer::Log::GetCoreLogger()->info(__VA_ARGS__)
	#define FT_WARN(...)     ::FTransfer::Log::GetCoreLogger()->warn(__VA_ARGS__)
	#define FT_ERROR(...)    ::FTransfer::Log::GetCoreLogger()->error(__VA_ARGS__)
	#define FT_CRITICAL(...) ::FTransfer::Log::GetCoreLogger()->critical(__VA_ARGS__)
#endif

#ifndef FT_DEBUG
	#define FT_TRACE
	#define FT_INFO
	#define FT_WARN
	#define FT_ERROR
	#define FT_CRITICAL
#endif


