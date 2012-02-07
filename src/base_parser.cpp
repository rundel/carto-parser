#include <base_parser.hpp>

#include <parse/parse_tree.hpp>

namespace carto {
    
parse_tree base_parser::get_parse_tree()
{
    return tree;
}

std::string base_parser::get_path()
{
    return path;
}

int base_parser::get_node_type(utree const& ut)
{   
    return( tree.annotations(ut.tag()).second );
}

source_location base_parser::get_location(utree const& ut)
{    
    return tree.annotations()[ut.tag()].first;
}

}

