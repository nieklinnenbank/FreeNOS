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

#ifndef __LIBSTD_ARGUMENT_H
#define __LIBSTD_ARGUMENT_H

#include "Types.h"
#include "String.h"

class Argument
{
  public:

    Argument(const char *name);

    char getIdentifier() const;

    const String & getName() const;

    const String & getDescription() const;

    const String & getValue() const;

    Size getCount() const;

    void setIdentifier(char id);

    void setName(const char *name);

    void setDescription(const char *description);

    void setValue(const char *value);

    void setCount(Size count);

    bool operator == (const Argument & arg) const;

    bool operator != (const Argument & arg) const;

  private:

    char m_id;
    
    String m_name;

    String m_description;

    String m_value;

    Size m_count;

};

#endif /* __LIBSTD_ARGUMENT_H */
