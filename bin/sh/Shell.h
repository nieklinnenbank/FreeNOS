/**
 * Copyright (C) 2009 Niek Linnenbank
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

#ifndef __SH_SHELL
#define __SH_SHELL

/**
 * Very basic command shell.
 */
class Shell
{
    public:

	/**
	 * Executes the Shell by entering an infinite loop.
	 * @return Never.
	 */
	int run();

    private:
    
	/**
	 * Fetch a command from standard input.
	 * @return Pointer to a command.
	 */
	char * getCommand();
    
	/**
	 * Output a prompt.
	 */
	void prompt();
    
	/**
	 * Simple builtin ps(1) equivalent.
	 */
	void ps();
	
	/**
	 * Uname(1) function.
	 */
	void uname();
	
	/**
	 * memstat function.
	 */
	void memstat();
	
	/**
	 * Displays program help.
	 */
	void help();
};

#endif /* __SH_SHELL */
