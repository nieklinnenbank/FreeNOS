/*
 * Copyright (C) 2009 Coen Bijlsma
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

#ifndef __LIBPARSE_COMMANDLINE_H
#define __LIBPARSE_COMMANDLINE_H

#include <String.h>
#include <Vector.h>

#include "CommandLineOption.h"

/**
 * Represents a commandline
 */
class CommandLine
{
    public:

	/**
	 * Constructor
	 */
	CommandLine(String& line);
	
	/**
	 * Constructor
	 */
	CommandLine(char* line);
	
	/**
	 * Destructor
	 */
	virtual ~CommandLine();
	
	/**
	 * Returns the entered options
	 */
	Vector< CommandLineOption<String, String> > getOptions();
	
	/**
	 * Returns the CommandLineOption that has the given name
	 */
	CommandLineOption<String, String> getOption(String& name);
	
	/**
	 * Returns the CommandLineOption that has the given name
	 */
	CommandLineOption<String, String> getOption(char* name);

    private:

	char* _line;
	Vector< CommandLineOption<String, String> > _options;
	
	void _parse();
};

#endif
#endif /* __LIBPARSE_COMMANDLINE_H */
