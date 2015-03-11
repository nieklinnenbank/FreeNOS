/**
 * Logging utilities and definitions.
 *
 * @author Niek Linnenbank
 * @date 5 march 2015
 */

#ifndef _PRIV_EXEC_LOG_H
#define _PRIV_EXEC_LOG_H

#ifndef __HOST__
#include "Log.h"

/**
 * Log using PrivExec().
 */
class PrivExecLog : public Log
{
  public:

    /**
     * Constructor
     */
    PrivExecLog();

    /**
     * Write using PrivExec()
     */
    virtual void write(const char *str);
};

#endif /* __HOST__ */
#endif /* _PRIV_EXEC_LOG_H */
