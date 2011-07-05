/*==============================================================================
    Copyright (c) 2010 Colin Rundel

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef LOAD_MSS_PARSER_H
#define LOAD_MSS_PARSER_H

#include <iosfwd>
#include <sstream>

#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>

#include <mapnik/map.hpp>
#include <mapnik/config_error.hpp>

#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/font_set.hpp>

#include <utree/utree_utility.hpp>
#include <utility/environment.hpp>

namespace carto {

using mapnik::config_error;

struct mss_parser {

    parse_tree tree;
    bool strict;
    std::string path;
    
    mss_parser(parse_tree const& pt, bool strict_ = false, std::string const& path_ = "./")
      : tree(pt),
        strict(strict_),
        path(path_) { }
      
    mss_parser(std::string const& in, bool strict_ = false, std::string const& path_ = "./")
      : strict(strict_),
        path(path_) 
    { 
        typedef position_iterator<std::string::const_iterator> it_type;
        tree = build_parse_tree< carto_parser<it_type> >(in),    
    }
    
    template<class T>
    T as(utree& ut)
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
    
    node_type get_node_type(utree const& ut)
    {   
        return( (node_type) tree.annotations(ut.tag()).second );
    }
    
    source_location get_location(utree const& ut)
    {    
        return tree.annotations()[ut.tag()].first;
    }

    void parse_stylesheet(mapnik::Map& map)
    {
        using spirit::utree_type;
        typedef utree::const_iterator iter;
        
        environment variables;
        environment mixins;
        
        utree root_node = tree.ast();
        
        iter it = root_node.begin(),
            end = root_node.end();
        
        for (; it != end; ++it) {
            try {
                switch(get_node_type(*it)) {
                    case carto_variable:
                        variables.define(as<std::string>((*it).front()), )
                        break;
                    case carto_mixin:
                    
                        break;
                    case carto_style,:
                        parse_style(map, *it);
                        break;
                    default:
                        throw config_error(std::string("Invalid node type: ")+get_node_type(*it));
                }
                

            } catch (std::exception& e) {
                std::cerr << "Error: " << e.what() << " at " << get_location(*it).get_string() << "\n";
                
                if (strict) break;
            } catch (...) {
                std::cerr << "Error: Unknown error at " << get_location(*it).get_string() << "\n";

                if (strict) break;
            }          
         }
    }
    
    void parse_style(mapnik::Map& map, utree const& node)
    {
        environment variables;
        environment mixins;
        
        std::string name = as<std::string>(node.front());
        
        typedef utree::iterator iter;
        iter it = node.begin()+1,
            end = node.end();
        
        if (name == "Map") {
            
            mapnik::parameters extra_attr;
            bool relative_to_xml = true;
            
            for (; it != end; ++it) {
                BOOST_ASSERT((*it).size()==2);

                std::string key = as<std::string>((*it).front());
                utree const& value = (*it).back();
                std::string base = "";

                if (key == "srs") {
                    map.set_srs(as<std::string>(value));
                } else if (key == "background-color") {
                    boost_assert(get_node_type(value) == carto_color);
                    map.set_background(as<mapnik::color>(value));
                } else if (key == "background-image") {
                    map.set_background_image(base+as<std::string>(value));
                } else if (key == "buffer-size") {
                    map.set_buffer_size(as<unsigned>(value));
                } else if (key == "base") {
                    base = as<std::string>(value); // FIXME - not sure this is correct
                } else if (key == "paths-from-xml") {
                    relative_to_xml = as<bool>(value);
                } else if (key == "minimum-version") {
                    std::string ver_str = as<std::string>(value);
                    extra_attr["minimum-version"] = ver_str;
                    
                    int min_ver = version_from_string(ver_str);
                    
                    if (min_ver == -1 && strict) {
                        throw config_error(std::string("Invalid version string") + ver_str);
                    } else if (min_ver > MAPNIK_VERSION) {
                        throw config_error(std::string("This map uses features only present in Mapnik version ") + ver_str + " and newer");
                    }
                } 
                else if (key == "font-directory") {
                    std::string dir = base+as<std::string>(value);
                    extra_attr["font-directory"] = dir;
                    freetype_engine::register_fonts( ensure_relative_to_xml(dir), false);
                } else {
                    throw config_error(std::string("Unknown attribute keyword: ")+key);
                }
            }
            
            map.set_extra_attributes(extra_attr);
            
            
        } else if (name[0] == '#') {
            
        } else if (name[0] == '.') {
            
        } else {
            throw config_error(std::string("Invalid style name: ")+name);
        }
    }
    
};




mss_parser load_mss(char const* filename, bool strict)
{
    std::ifstream file(filename, std::ios_base::in);

    if (!file)
        throw std::exception();

    std::string in;
    file.unsetf(std::ios::skipws);
    copy(std::istream_iterator<char>(file),
         std::istream_iterator<char>(),
         std::back_inserter(in));

    return mss_parser(in, strict, filename);
}

/*
mml_parser load_mml_string(std::string const& in, bool strict, std::string const& base_url)
{
    return mml_parser(in, strict, base_url);
}
*/

}
#endif 
