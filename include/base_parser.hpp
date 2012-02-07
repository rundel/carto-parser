
#ifndef BASE_PARSER_H
#define BASE_PARSER_H


#include <parse/parse_tree.hpp>
#include <utility/utree.hpp>

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
    
    parse_tree get_parse_tree();
    
    std::string get_path();
    
    int get_node_type(utree const& ut);
    
    source_location get_location(utree const& ut);
};

}
#endif
