/*
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

#ifndef __INIT_H
#define __INIT_H

#ifdef __HOST__
#error "Init.h is not supported for the host platform."
#endif

#include "Types.h"
#include "Macros.h"

/**
 * Defines a class function for initialization.
 * @param class The class to use.
 * @param func Target function.
 * @param section Where to place the function pointer.
 */
#define _DEFINE_CLASSFUNC(class,func,section) \
    static Address __initcall_##class##func \
    SECTION((section)) USED = (Address) class::func;

/**
 * Declares a function for initialization.
 * @param func Function to use as init function.
 * @param section Where to place the function pointer.
 */
#define _DEFINE_FUNC(func,section) \
    static Address __initcall_##func \
    SECTION((section)) USED = (Address) func;

/**
 * Declares an initialization class function
 * @param func The class function to declare as "init" function.
 * @param level Allows ordering of "init" functions.
 */
#define INITCLASS(class,func,level) \
    _DEFINE_CLASSFUNC(class,func,".init" level)

/**
 * Declares a function for initialization.
 * @param func Function to declare as "init" function.
 * @param level The ordering of the function.
 */
#define INITFUNC(func,level) \
    _DEFINE_FUNC(func,".init" level)

/**
 * Initializes an object, assigning it to a variable.
 * @param class Name of the class to instantiate.
 * @param name Variable name.
 * @param level The ordering of the instantiation.
 */
#define INITOBJ(class,name,level) \
    class *name; \
    \
    void __mk_##class##name () \
    { \
	name = class::instance(); \
    } \
    INITFUNC(__mk_##class##name, level)

/**
 * Execute a range of initialization functions.
 * @param from Points to the first InitHandler to execute.
 * @param to Points to the last InitHandler plus one.
 */
#define INITRUN(from,to) \
    { \
	Address *i; \
	\
	for (i = (from); i < (to); i++) \
	{ \
    	    (*(InitHandler **) i)(); \
	} \
    }

/**
 * Function executed at initialization time.
 */
typedef void InitHandler();

#endif /* __INIT_H */
