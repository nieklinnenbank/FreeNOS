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

void Log::append(const char *str)
{
    m_outputBuffer << str;

    if (m_outputBuffer.endsWith("\n"))
    {
	write(*m_outputBuffer);
	m_outputBuffer = "";
    }
}

Log & operator << (Log &log, const char *str)
{
    log.append(str);
    return log;
}

Log & operator << (Log &log, int number)
{
    String s = number;
    log.append(*s);
    return log;
}

Log & operator << (Log &log, unsigned number)
{
// TODO: #warning: string does not have unsigned conversions yet
    String s = number;
    log.append(*s);
    return log;
}

Log & operator << (Log &log, unsigned long number)
{
    String s = number;
    log.append(*s);
    return log;
}

Log & operator << (Log &log, void *ptr)
{
    String s;
    s << Number::Hex << (long ) ptr;
    log.append(*s);
    return log;
}
