/**
 * Logging utilities and definitions.
 *
 * @author Niek Linnenbank
 * @date 5 march 2015
 */

#ifndef __HOST__
#include <FreeNOS/System.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "StdioLog.h"

StdioLog::StdioLog() : Log()
{
}

void StdioLog::write(const char *str)
{
    ::write(1, str, strlen(str));
}

#endif /* __HOST__ */
