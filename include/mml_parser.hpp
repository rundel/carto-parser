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

#include "utree/utree_utility.hpp"

namespace carto {

namespace detail {
    template<class T>
    T as(utree& ut) {
        return ut.get<T>();
    }
    
    template<>
    std::string as<std::string>(utree& ut) 
    {    
        return utree::visit(ut, utree_to_string());
    }
}

struct mml_parser {
    typedef void result_type;

    parse_tree tree;
    std::string path;
    
    mml_parser(parse_tree const& pt, std::string const& path_ = "./")
      : tree(pt),
        path(path_) { }
      
    mml_parser(std::string const& in, std::string const& path_ = "./")
      : tree(in),
        path(path_) { }
    
    template<class T>
    T as(utree& ut) {
        return detail::as<T>(ut);
    }
    
    parse_tree get_parse_tree() {
        return tree;
    }
    
    std::string get_path() {
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

    void parse_map(mapnik::Map& map, bool strict = false)
    {
        using spirit::utree_type;
        typedef utree::iterator iter;
        
        
        utree root_node = tree.ast();
        
        BOOST_ASSERT(get_node_type(root_node) == json_object);
        
        
        iter it, end;
        
        it  = root_node.front().begin(); 
        end = root_node.front().end();
        
        for (; it != end; ++it) {
            try {
                BOOST_ASSERT((*it).size()==2);
            
                utree& key = (*it).front();
                utree& value = (*it).back();  
            
                if (key == utree(spirit::utf8_symbol_type("srs"))) {

                    map.set_srs( as<std::string>(value) );
                
                } else if (key == utree(spirit::utf8_symbol_type("Stylesheet"))) {
                
                } else if (key == utree(spirit::utf8_symbol_type("Layer"))) {
                
                    BOOST_ASSERT(get_node_typevalue == json_array);
                
                    iter lyr_it  = value.begin(), 
                         lyr_end = value.end();
                
                    for (; lyr_it != lyr_end; ++lyr_it) {
                        BOOST_ASSERT(get_node_type(*lyr_it) == json_object);
                        parse_layer(map, (*lyr_it).front()) ;
                    }
                
                } else {
                    throw "Unknown keyword: "+as<std::string>(key);
                }
            } catch (std::exception& e) {
                std::cerr << "Error: " << e.what() << " at " << get_location(*it).get_string() << "\n";
                
                if (strict) break;
            } catch (std::string& s) {
                std::cerr << "Error: " << s << " at " << get_location(*it).get_string() << "\n";

                if (strict) break;
            } catch (...) {
                std::cerr << "Error: Unknown error at " << get_location(*it).get_string() << "\n";

                if (strict) break;
            }          
         }
    }
    
    void parse_layer(mapnik::Map& map, utree node)
    {
        using spirit::utree_type;
        
        mapnik::layer lyr("","");
        
        typedef utree::iterator iter;
        iter it  = node.begin(), 
             end = node.end();
        
        for (; it != end; ++it) {
            BOOST_ASSERT((*it).size()==2);
            
            utree& key   = (*it).front();
            utree& value = (*it).back();
            
            if (key == utree(spirit::utf8_symbol_type("id"))) {
                
            } else if (key == utree(spirit::utf8_symbol_type("Datasource"))) {

            } else if (key == utree(spirit::utf8_symbol_type("name"))) {
                
                lyr.set_name( as<std::string>(value) );
                
            } else if (key == utree(spirit::utf8_symbol_type("srs"))) {
                
                lyr.set_srs( as<std::string>(value) );
                
            } else if (key == utree(spirit::utf8_symbol_type("status"))) {
            
            } else if (key == utree(spirit::utf8_symbol_type("title"))) {
                
                lyr.set_title( as<std::string>(value) );
                
            } else if (key == utree(spirit::utf8_symbol_type("abstract"))) {
                                        
                lyr.set_abstract( as<std::string>(value) );
                
            } else if (key == utree(spirit::utf8_symbol_type("minzoom"))) {
                
                lyr.setMinZoom( value.get<double>() );
                
            } else if (key == utree(spirit::utf8_symbol_type("maxzoom"))) {
                
                lyr.setMaxZoom( value.get<double>() );
            
            } else if (key == utree(spirit::utf8_symbol_type("queryable"))) {
            
                lyr.setQueryable( value.get<bool>() );
            
            } else {
                throw "Unknown keyword: "+as<std::string>(key);
            }
        }
        
        map.addLayer(lyr);
    }
    
};

void load_mml(mapnik::Map& map, char const* filename, bool strict)
{
    std::ifstream file(filename, std::ios_base::in);

    if (!file)
        throw std::exception();

    std::string in;
    file.unsetf(std::ios::skipws);
    copy(std::istream_iterator<char>(file),
         std::istream_iterator<char>(),
         std::back_inserter(in));

    carto::mml_parser parser(in, filename);
    parser.parse_map(map, strict);
}

void load_mml_string(mapnik::Map& map, std::string const& in, bool strict, std::string const& base_url)
{
    carto::mml_parser parser(in, base_url);
    parser.parse_map(map, strict);
}

}
#endif 
