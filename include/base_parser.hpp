
#ifndef BASE_PARSER_H
#define BASE_PARSER_H

#include <parse/parse_tree.hpp>

namespace carto {

struct base_parser {

    parse_tree tree;
    bool strict;
    std::string path;
    
    template<class T>
    T as(utree const& ut)
    {
        return detail::as<T>(ut);
    }
    
    parse_tree get_parse_tree()
    {
        return tree;
    }
    
    std::string get_path()
    {
        return path;
    }
    
    int get_node_type(utree const& ut)
    {   
        return( tree.annotations(ut.tag()).second );
    }
    
    source_location get_location(utree const& ut)
    {    
        return tree.annotations()[ut.tag()].first;
    }
};

#endif