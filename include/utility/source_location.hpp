/*==============================================================================
    Copyright (c) 2001-2011 Joel de Guzman
    Copyright (c) 2010      Bryce Lelbach
 
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <string>

#ifndef MAPNIK_SOURCE_LOCATION_H
#define MAPNIK_SOURCE_LOCATION_H
 
namespace carto {
 
struct source_location
{ 
    int line;
    int column;
 
    source_location (int l, int c);
 
    source_location ();
 
    bool valid();
    
    std::string get_string();
    
    bool operator==(source_location const& other) const;   
};
 
}
 
#endif
