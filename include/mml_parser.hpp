/*==============================================================================
    Copyright (c) 2010 Colin Rundel

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef LOAD_MML_PARSER_H
#define LOAD_MML_PARSER_H

#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>

#include <boost/optional.hpp>

#include <parse/parse_tree.hpp>
#include <utility/utree.hpp>

namespace carto {

namespace al = boost::algorithm;

struct mml_parser {

    parse_tree tree;
    bool strict;
    std::string path;
    std::vector< std::vector<std::string> > layer_selectors;
    
    mml_parser(parse_tree  const& pt, std::string const& path_, bool strict_ = false);
    mml_parser(std::string const& in, std::string const& path_, bool strict_ = false);
    mml_parser(std::string const& filename, bool strict_ = false);
    
    parse_tree get_parse_tree();
    std::string get_path();
    
    node_type get_node_type(utree const& ut);
    source_location get_location(utree const& ut);
    
    void key_error(std::string const& key, utree const& node);
    
    void parse(mapnik::Map& map);
    void parse_map(mapnik::Map& map);
    void parse_stylesheet(mapnik::Map& map, utree const& node);
    void parse_layer(mapnik::Map& map, utree const& node);
    void parse_Datasource(mapnik::layer& lyr, utree const& node);

    std::string ensure_relative_to_xml( boost::optional<std::string> opt_path );
    
};




mml_parser load_mml(std::string filename, bool strict);

/*
mml_parser load_mml_string(std::string const& in, bool strict, std::string const& base_url);
*/

}
#endif 
