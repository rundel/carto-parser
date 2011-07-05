////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2010 Bryce Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <boost/spirit/include/support_utree.hpp>
#include <boost/unordered_map.hpp>

namespace carto {

struct environment {

private:
    environment const* parent;
    boost::unordered_map<std::string, utree const* > definitions;


    environment(void): parent(), definitions() { }

    environment(environment const& parent_)
      : parent(parent_), 
        definitions() { }

    utree const* lookup (std::string const& name) {
        typename map_type::iterator it = definitions.find(name);

        if (it == definitions.end()) {
            if (parent)
                return parent->lookup(name);

            return utree::nil_type();
        }

        return it->second; 
    }
    
    void define (std::string const& name, uteee const& val) {
        //BOOST_ASSERT(!definitions.count(name));
        definitions[name] = &val;
    }

    bool defined (std::string const& name) const {
        if (!definitions.count(name)) {
            if (parent)
                return parent->defined(name);

            return false;
        }

        return true; 
    }
    
    bool locally_defined (key_type const& name) const {
        return definitions.count(name); 
    } 
};
}
#endif