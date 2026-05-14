// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

// Cross-platform Linux syslog-compatible log levels (RFC 5424)
// On Linux these match <syslog.h> LOG_EMERG...LOG_DEBUG values.
// Defined here so the widget is self-contained and portable on Windows/macOS.

enum class LogLevel : int {
    Emergency = 0,  // LOG_EMERG   – system is unusable
    Alert     = 1,  // LOG_ALERT   – action must be taken immediately
    Critical  = 2,  // LOG_CRIT    – critical conditions
    Error     = 3,  // LOG_ERR     – error conditions
    Warning   = 4,  // LOG_WARNING – warning conditions
    Notice    = 5,  // LOG_NOTICE  – normal but significant condition
    Info      = 6,  // LOG_INFO    – informational messages
    Debug     = 7,  // LOG_DEBUG   – debug-level messages
};

inline const char* logLevelName(LogLevel level) {
    switch (level) {
        case LogLevel::Emergency: return "EMERG";
        case LogLevel::Alert:     return "ALERT";
        case LogLevel::Critical:  return "CRIT";
        case LogLevel::Error:     return "ERROR";
        case LogLevel::Warning:   return "WARN";
        case LogLevel::Notice:    return "NOTICE";
        case LogLevel::Info:      return "INFO";
        case LogLevel::Debug:     return "DEBUG";
        default:                  return "UNKNOWN";
    }
}

inline const char* logLevelName(int level)
{
  return logLevelName(static_cast<LogLevel>(level));
}
