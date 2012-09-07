/*==============================================================================
    Copyright (c) 2010 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef GENERATE_FILTER_H
#define GENERATE_FILTER_H

#include <iosfwd>
#include <sstream>

#include <parse/parse_tree.hpp>
#include <parse/filter_grammar.hpp>

#include <utility/environment.hpp>
#include <utility/utree.hpp>

#include <mapnik/rule.hpp>

namespace carto {

struct filter_printer {
    typedef std::string result_type;

    utree const& tree;
    annotations_type const& annotations;
    style_env const& env;
    mapnik::rule& rule;

    filter_printer(utree const& tree_, annotations_type const& annotations_, 
                   style_env const& env_, mapnik::rule& rule_);
    
    std::string print();
    
    utree parse_var(utree const& ut);
    
    double parse_zoom_value(utree const& ut);
    
    std::string operator() (utree const& ut);

    template<class T>
    std::string operator() (T val)
    {
        std::stringstream out;
        out << val;
        
        return out.str();
    }

    std::string operator() (bool b);

    std::string operator() (spirit::utf8_string_range_type const& str);

    std::string operator() (spirit::utf8_symbol_range_type const& str);
};

}
#endif 
