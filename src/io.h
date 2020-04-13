#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace io {
static std::shared_ptr<spdlog::logger> logger;

void init(){
  spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  sink->set_pattern("[+] %^%v%$");

  logger = std::make_shared<spdlog::logger>("server", sink);
  spdlog::register_logger(logger);
}

auto get() { return std::move(logger); }
};  // namespace io