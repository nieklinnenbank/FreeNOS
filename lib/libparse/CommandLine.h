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
#include "Delimeter.h"

/**
 * Represents a commandline
 */
class CommandLine
{
    public:

	/**
	 * Constructor
	 * @param line The original commandline
	 * @param delimeters
	 */
	CommandLine(String* line, Vector<Delimeter>* delimeters);
	
	/**
	 * Constructor
	 */
	CommandLine(char* line, Vector<Delimeter>* delimeters);
	
	/**
	 * Destructor
	 */
	virtual ~CommandLine() {};
	
	/**
	 * Returns the entered options
	 */
	Vector< CommandLineOption<String, String> > getOptions();
	
	/**
	 * Returns the CommandLineOption that has the given name
	 */
	CommandLineOption<String, String>* getOption(String& name);
	
	/**
	 * Returns the CommandLineOption that has the given name
	 */
	CommandLineOption<String, String>* getOption(char* name);

    private:

	char* line;
	Vector< Delimeter >* delimeters;
	Vector< CommandLineOption<String, String> > options;
	
	void parse();
};

#endif /* __LIBPARSE_COMMANDLINE_H */
