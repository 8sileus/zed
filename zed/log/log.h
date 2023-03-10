#ifndef ZED_LOG_LOG_H_
#define ZED_LOG_LOG_H_

#include <memory>
#include <sstream>
#include <string_view>

#include "zed/comm/thread.h"
#include "zed/log/log_appender.h"
#include "zed/log/log_buffer.h"
#include "zed/log/log_file.h"
#include "zed/util/noncopyable.h"
#include "zed/util/singleton.hpp"

#define LOG_EVENT(level)                  \
    if (zed::Logger::GetLevel() <= level) \
    zed::LogEvent(level, __FILE__, __LINE__, __func__).getStringStream()

#define LOG_DEBUG LOG_EVENT(zed::LogLevel::DEBUG)
#define LOG_INFO LOG_EVENT(zed::LogLevel::INFO)
#define LOG_WARN LOG_EVENT(zed::LogLevel::WARN)
#define LOG_ERROR LOG_EVENT(zed::LogLevel::ERROR)
#define LOG_FATAL LOG_EVENT(zed::LogLevel::FATAL)

namespace zed {

class LogLevel {
public:
    enum Level { UNKNOWN = 1, DEBUG, INFO, WARN, ERROR, FATAL };

    static std::string_view Tostring(LogLevel::Level level);
    static LogLevel::Level  Fromstring(std::string& str);
};

class LogEvent {
public:
    using Ptr = std::shared_ptr<LogEvent>;

    LogEvent(LogLevel::Level level, const char* file_name, int32_t line, const char* func_name);
    ~LogEvent();

    std::stringstream& getStringStream() { return m_ss; };

private:
    void addFormattedTime();
    void addLevel() { m_ss << "[" << LogLevel::Tostring(m_level) << "]\t"; }
    void addTid() { m_ss << '[' << this_thread::GetId() << "]\t"; }
    void addFileInformation();
    void setColor();

private:
    LogLevel::Level   m_level {};
    const char*       m_file_name {nullptr};
    int32_t           m_line {0};
    const char*       m_func_name {nullptr};
    std::stringstream m_ss {};
};

class Logger : util::Noncopyable {
public:
    using Ptr = std::shared_ptr<Logger>;

    Logger();
    ~Logger() = default;

    void log(std::string&& msg);

    void setAppender(std::unique_ptr<LogAppender> appender) { m_appender = std::move(appender); }

public:
    static void            SetLevel(LogLevel::Level level);
    static LogLevel::Level GetLevel();

private:
    static LogLevel::Level g_level;

private:
    std::unique_ptr<LogAppender> m_appender;
};

using LoggerManager = util::Singleton<Logger>;

} // namespace zed

#endif // ZED_LOG_LOG_H_