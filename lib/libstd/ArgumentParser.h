/*
 * Copyright (C) 2015 Niek Linnenbank
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIBSTD_ARGUMENTPARSER_H
#define __LIBSTD_ARGUMENTPARSER_H

#include "Types.h"
#include "Macros.h"
#include "String.h"
#include "HashTable.h"
#include "HashIterator.h"
#include "Vector.h"
#include "Argument.h"
#include "ArgumentContainer.h"

/**
 * Generic command-line argument parser.
 */
class ArgumentParser
{
  public:

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        InvalidArgument,
        NotFound,
        AlreadyExists
    };

    /**
     * Constructor
     */
    ArgumentParser();

    /**
     * Destructor
     */
    virtual ~ArgumentParser();

    /**
     * Get program usage.
     *
     * @return Program usage string reference.
     */
    String & getUsage();

    /**
     * Set program name.
     *
     * @param name Program name string.
     */
    void setName(const char *name);

    /**
     * Set program description.
     *
     * @param desc Program description string.
     */
    void setDescription(const char *desc);

    /**
     * Register a flag Argument
     *
     * @return Result code.
     */
    Result registerFlag(char arg,
                        const char *name,
                        const char *description);

    /**
     * Register a positional argument.
     *
     * @return Result code.
     */
    Result registerPositional(const char *name,
                              const char *description,
                              Size count=1);

    /**
     * Parse input arguments.
     *
     * This function parses the given arguments and
     * fills the argument container with parsed arguments.
     *
     * @param argc Input argument count.
     * @param argv Input argument values.
     * @param output Filled with parsed arguments on output.
     * @return Result code.
     */
    Result parse(int argc,
                 char **argv,
                 ArgumentContainer & output);

  private:

    /** Contains all registered flag arguments. */
    HashTable<String, Argument *> m_flags;

    /** Contains all registered positional arguments. */
    Vector<Argument *> m_positionals;

    /** Program name */
    String m_name;

    /** Program description */
    String m_description;

    /** Program usage */
    String m_usage;
};

#endif /* __LIBSTD_ARGUMENTPARSER_H */
