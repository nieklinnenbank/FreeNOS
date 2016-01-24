/**
 * Logging utilities and definitions.
 *
 * @author Niek Linnenbank
 * @date 5 march 2015
 */

#ifndef __LIBARCH_STDIOLOG_H
#define __LIBARCH_STDIOLOG_H

#ifndef __HOST__
#include "Log.h"

/**
 * Log to standard output.
 */
class StdioLog : public Log
{
  public:

    /**
     * Constructor
     */
    StdioLog();

    /**
     * Write to the standard output.
     */
    virtual void write(const char *str);
};

#endif /* __HOST__ */
#endif /* __LIBARCH_STDIOLOG_H */
