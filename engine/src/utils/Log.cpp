#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"


std::shared_ptr<spdlog::logger> granite::Log::c_Logger;

void granite::Log::Init()
{
    spdlog::set_pattern("%^ %n [%T] : %v	%$");
    c_Logger = spdlog::stdout_color_mt("Granite");
    c_Logger->set_level(spdlog::level::trace);
}
