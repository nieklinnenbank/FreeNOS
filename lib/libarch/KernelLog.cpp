/**
 * Logging utilities and definitions.
 *
 * @author Niek Linnenbank
 * @date 5 march 2015
 */

#ifndef __HOST__
#include <FreeNOS/System.h>
#include "KernelLog.h"

KernelLog::KernelLog() : Log()
{
}

void KernelLog::write(const char *str)
{
    PrivExec(WriteConsole, (Address) str);
}

#endif /* __HOST__ */
