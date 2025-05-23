#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <stdio.h>
#include <memory>
#include <iostream>

namespace granite
{
	class Log{
		public:
			static void Init();

			inline static std::shared_ptr<spdlog::logger>& GetLogger(){
				return c_Logger;
			}
		private:
			static std::shared_ptr<spdlog::logger> c_Logger;
	};
}