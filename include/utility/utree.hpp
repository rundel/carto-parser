/*==============================================================================
    Copyright (c) 2001-2011 Joel de Guzman
    Copyright (c) 2010      Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef UTREE_UTILITY_H
#define UTREE_UTILITY_H

#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/support_utree.hpp>

namespace carto {

namespace spirit = boost::spirit;
using spirit::utree;

struct utree_to_string {
    typedef std::string result_type;

    template<class T>
    std::string operator() (T val){
        return boost::lexical_cast<std::string>(val);
    }
    
    std::string operator() (spirit::utf8_string_range_type const& str)
    {
        typedef spirit::utf8_string_range_type::const_iterator iterator;
        iterator it = str.begin(), end = str.end();
        
        std::string out;
        for (; it != end; ++it)
            out += *it;
        
        return out;
    }

    std::string operator() (spirit::utf8_symbol_range_type const& str)
    {
        typedef spirit::utf8_symbol_range_type::const_iterator iterator;
        iterator it = str.begin(), end = str.end();

        std::string out;
        for (; it != end; ++it)
            out += *it;
        
        return out;
    }
    
    template<typename Iterator>
    std::string operator() (boost::iterator_range<Iterator> const& range){
        BOOST_ASSERT(false);
        
        return std::string();
    }

    std::string operator() (utree::invalid_type ut){
        return (*this)("[invalid]");
    }

    std::string operator() (utree::nil_type ut){
        return (*this)("[null]");
    }

    std::string operator() (spirit::binary_range_type const& str){
        return (*this)("[binary]");
    }

    std::string operator() (spirit::any_ptr const& p){
        return (*this)("[pointer]");
    }

    std::string operator() (spirit::function_base const& pf){
        return (*this)("[function]");
    }
};

namespace detail {

    template<class T>
    T as(utree& ut) {
        return ut.get<T>();
    }

    template<class T>
    T as(utree const& ut) {
        return ut.get<T>();
    }

    template<>
    std::string as<std::string>(utree const& ut) 
    {    
        return utree::visit(ut, utree_to_string());
    }

    template<>
    mapnik::color as<mapnik::color>(utree const& ut) 
    {    
        BOOST_ASSERT(ut.size()==4);
        
        utree::const_iterator it = ut.begin();
        
        int r = as<int>(*it++),
            g = as<int>(*it++),
            b = as<int>(*it++),
            a = as<int>(*it++);
        
        return mapnik::color(r,g,b,a);
    }
}

}

#endif