/*==============================================================================
    Copyright (c) 2010 Colin Rundel

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef LOAD_UTREE_H
#define LOAD_UTREE_H

#include <iosfwd>
#include <sstream>

#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>

#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/params.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>

#include "utree/utree_utility.hpp"

namespace carto {

using mapnik::config_error;

struct mml_parser {

    parse_tree tree;
    bool strict;
    std::string path;
    
    mml_parser(parse_tree const& pt, bool strict_ = false, std::string const& path_ = "./")
      : tree(pt),
        strict(strict_),
        path(path_) { }
      
    mml_parser(std::string const& in, bool strict_ = false, std::string const& path_ = "./")
      : tree(in),
        strict(strict_),
        path(path_) { }
    
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

    void parse_map(mapnik::Map& map)
    {
        using spirit::utree_type;
        typedef utree::iterator iter;
        
        
        utree root_node = tree.ast();
        
        BOOST_ASSERT(get_node_type(root_node) == json_object);
        
        iter it = root_node.front().begin(),
            end = root_node.front().end();
        
        for (; it != end; ++it) {
            try {
                BOOST_ASSERT((*it).size()==2);
            
                std::string key = as<std::string>((*it).front());
                utree& value = (*it).back();  
            
                if (key == "srs") {
                    map.set_srs( as<std::string>(value) );
                } else if (key == "Stylesheet") {
                
                } else if (key == "Layer") {
                    BOOST_ASSERT(get_node_type(value) == json_array);
                
                    iter lyr_it  = value.begin(), 
                         lyr_end = value.end();
                
                    for (; lyr_it != lyr_end; ++lyr_it) {
                        BOOST_ASSERT(get_node_type(*lyr_it) == json_object);
                        parse_layer(map, (*lyr_it).front()) ;
                    }
                } else {
                    throw config_error(std::string("Unknown keyword: ")+key);
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
    
    void parse_layer(mapnik::Map& map, utree node)
    {
        mapnik::layer lyr("","");
        
        typedef utree::iterator iter;
        iter it  = node.begin(), 
             end = node.end();
        
        for (; it != end; ++it) {
            try {
                BOOST_ASSERT((*it).size()==2);
            
                std::string key = as<std::string>((*it).front());
                utree& value = (*it).back();
            
                if (key == "id") {
                
                } else if (key == "name") {
                    lyr.set_name( as<std::string>(value) );
                } else if (key == "srs") {
                    lyr.set_srs( as<std::string>(value) );
                } else if (key == "status") {
                    lyr.setActive( as<bool>(value) );
                } else if (key == "title") {
                    lyr.set_title( as<std::string>(value) );
                } else if (key == "abstract") {
                    lyr.set_abstract( as<std::string>(value) );
                } else if (key == "minzoom") {
                    lyr.setMinZoom( value.get<double>() );
                } else if (key == "maxzoom") {
                    lyr.setMaxZoom( value.get<double>() );
                } else if (key == "queryable") {
                    lyr.setQueryable( value.get<bool>() );
                } else if (key == "Datasource") {
                    BOOST_ASSERT(get_node_type(value) == json_object);
                
                    parse_Datasource(lyr, value.front());
                
                } else {
                    throw config_error(std::string("Unknown keyword: ")+key);
                }
            } catch (std::exception& e) {
                throw config_error( e.what() );
            } catch (...) {
                throw config_error( "Unknown error" );
            }
        }
        
        map.addLayer(lyr);
    }


    void parse_Datasource(mapnik::layer& lyr, utree node)
    {
        mapnik::parameters params;
        
        typedef utree::iterator iter;
        iter it  = node.begin(), 
             end = node.end();
        
        for (; it != end; ++it)
        {
            BOOST_ASSERT((*it).size()==2);

            std::string name  = as<std::string>((*it).front());
            std::string value = as<std::string>((*it).back());
            
            params[name] = value;
        }

        //boost::optional<std::string> base_param = params.get<std::string>("base");
        //boost::optional<std::string> file_param = params.get<std::string>("file");
        //
        //if (base_param) {
        //    params["base"] = ensure_relative_to_xml(base_param);
        //} else if (file_param) {
        //    params["file"] = ensure_relative_to_xml(file_param);
        //}

        try {
            boost::shared_ptr<mapnik::datasource> ds =
                mapnik::datasource_cache::instance()->create(params);
            lyr.set_datasource(ds);
        } catch (const mapnik::config_error & ex ) {
            throw config_error( ex.what() );
        } catch (const mapnik::datasource_exception & ex ) {
            throw config_error( ex.what() );
        } catch (...) {
            throw config_error("Unknown exception");
        }
    }
    
};




mml_parser load_mml(char const* filename, bool strict)
{
    std::ifstream file(filename, std::ios_base::in);

    if (!file)
        throw std::exception();

    std::string in;
    file.unsetf(std::ios::skipws);
    copy(std::istream_iterator<char>(file),
         std::istream_iterator<char>(),
         std::back_inserter(in));

    return mml_parser(in, strict, filename);
}

/*
mml_parser load_mml_string(std::string const& in, bool strict, std::string const& base_url)
{
    return mml_parser(in, strict, base_url);
}
*/

}
#endif 
