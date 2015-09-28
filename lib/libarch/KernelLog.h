/**
 * Logging utilities and definitions.
 *
 * @author Niek Linnenbank
 * @date 5 march 2015
 */

#ifndef __LIBARCH_KERNELLOG_H
#define __LIBARCH_KERNELLOG_H

#ifndef __HOST__
#include "Log.h"

/**
 * Log using PrivExec().
 */
class KernelLog : public Log
{
  public:

    /**
     * Constructor
     */
    KernelLog();

    /**
     * Write using PrivExec()
     */
    virtual void write(const char *str);
};

#endif /* __HOST__ */
#endif /* __LIBARCH_KERNELLOG_H */
