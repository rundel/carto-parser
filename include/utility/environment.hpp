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
    typedef boost::unordered_map<std::string, boost::spirit::utree> map_type;
    map_type definitions;

public:
    environment(void);

    environment(environment const& parent_);

    boost::spirit::utree lookup (std::string const& name) const;
    
    void define (std::string const& name, boost::spirit::utree const& val);

    bool defined (std::string const& name) const;
    
    bool locally_defined (std::string const& name) const;
};


struct style_env {
    environment vars;
    environment mixins;
    
    style_env();
        
    //style_env(environment vars_, environment mixins_);
        
    style_env(style_env const& env);
};

}
#endif
