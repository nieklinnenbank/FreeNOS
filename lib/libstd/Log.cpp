/**
 * Logging utilities and definitions.
 *
 * @author Niek Linnenbank
 * @date 12 June 2014
 */

#include "Log.h"
#include "String.h"

Log::Log() : Singleton<Log>(this)
{
    setMinimumLogLevel(Debug);
}

Log::~Log()
{
}

Log::Level Log::getMinimumLogLevel()
{
    return m_minimumLogLevel;
}

void Log::setMinimumLogLevel(Log::Level level)
{
    m_minimumLogLevel = level;
}

const char * Log::getIdent()
{
    return (m_ident);
}

void Log::setIdent(const char *ident)
{
    m_ident = ident;
}

Log & operator << (Log &log, const char *str)
{
    log.write(str);
    return log;
}

Log & operator << (Log &log, int number)
{
    char buf[16];
    String::format(buf, sizeof(buf), "%d", number);
    log.write(buf);
    return log;
}

Log & operator << (Log &log, unsigned number)
{
    char buf[16];
    String::format(buf, sizeof(buf), "%u", number);
    log.write(buf);
    return log;
}

Log & operator << (Log &log, unsigned long number)
{
    char buf[16];
    String::format(buf, sizeof(buf), "%lu", number);
    log.write(buf);
    return log;
}

Log & operator << (Log &log, void *ptr)
{
    char buf[16];
    String::format(buf, sizeof(buf), "%p", ptr);
    log.write(buf);
    return log;
}

