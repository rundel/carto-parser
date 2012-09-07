/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef CARTO_ERROR_HPP
#define CARTO_ERROR_HPP

#include <utility/source_location.hpp>

namespace carto {

class carto_error : public std::exception
{
public:
    carto_error(std::string const& what);
    carto_error(std::string const& what, source_location const& loc);
    carto_error(std::string const& what, source_location const& loc, std::string const& filename);
    virtual ~carto_error() throw() {}

    virtual const char * what() const throw();

    void append_context(std::string const& ctx);
    void set_location(source_location const& node);
    void set_filename(std::string const& node);
protected:
    mutable std::string what_;
    mutable source_location loc_;
    mutable std::string file_;
    mutable std::string msg_;
};

void warn(carto_error const& err);

}

#endif // CARTO_ERROR_HPP
