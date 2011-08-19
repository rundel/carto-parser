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
    typedef boost::unordered_map<std::string, utree const* > map_type;
    map_type definitions;

public:
    environment(void): parent(), definitions() { }

    environment(environment const& parent_)
      : parent(&parent_), 
        definitions() { }

    utree const* lookup (std::string const& name) const {
        map_type::const_iterator it = definitions.find(name);

        if (it == definitions.end()) {
            if (parent)
                return (parent->lookup(name));

            return NULL;
        }

        return it->second; 
    }
    
    void define (std::string const& name, utree const& val) {
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
    
    bool locally_defined (std::string const& name) const {
        return definitions.count(name); 
    } 
};


struct style_env {
    environment vars;
    environment mixins;
    
    style_env() 
      : vars(),
        mixins() { }
        
    //style_env(environment vars_, environment mixins_)
    //  : vars(vars_),
    //    mixins(mixins_) { }
        
    style_env(style_env const& env)
      : vars(env.vars),
        mixins(env.mixins) { }
};

}
#endif