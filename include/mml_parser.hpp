/*==============================================================================
    Copyright (c) 2010 Colin Rundel

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef LOAD_MML_PARSER_H
#define LOAD_MML_PARSER_H

#include <iosfwd>
#include <sstream>

#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/params.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <mss_parser.hpp>
#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>
#include <utility/utree.hpp>

namespace carto {

using mapnik::config_error;
namespace al = boost::algorithm;

struct mml_parser {

    parse_tree tree;
    bool strict;
    std::string path;
    std::vector< std::vector<std::string> > layer_selectors;
    
    mml_parser(parse_tree const& pt, bool strict_ = false, std::string const& path_ = "./");
      
    mml_parser(std::string const& in, bool strict_ = false, std::string const& path_ = "./");
    
    template<class T>
    T as(utree const& ut)
    {
        return detail::as<T>(ut);
    }
    
    parse_tree get_parse_tree();

    std::string get_path();
    
    node_type get_node_type(utree const& ut);
    
    source_location get_location(utree const& ut);
    
    void key_error(std::string const& key, utree const& node);
    
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
