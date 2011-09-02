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

struct mml_parser {

    parse_tree tree;
    bool strict;
    std::string path;
    std::map<std::string, int> layer_lookup;
    
    mml_parser(parse_tree const& pt, bool strict_ = false, std::string const& path_ = "./")
      : tree(pt),
        strict(strict_),
        path(path_) { }
      
    mml_parser(std::string const& in, bool strict_ = false, std::string const& path_ = "./")
      : strict(strict_),
        path(path_) 
    { 
        typedef position_iterator<std::string::const_iterator> it_type;
        tree = build_parse_tree< json_parser<it_type> >(in, path);    
    }
    
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
    
    node_type get_node_type(utree const& ut)
    {   
        return( (node_type) tree.annotations(ut.tag()).second );
    }
    
    source_location get_location(utree const& ut)
    {    
        return tree.annotations()[ut.tag()].first;
    }
    
    void key_error(std::string const& key, utree const& node) {
        
        std::stringstream err;
        err << "Unknown keyword: " << key
            << " at " << get_location(node).get_string(); 
        
        if (strict)
            throw config_error(err.str());
        else
            std::clog << "### WARNING: " << err.str() << "\n";    
    }
    
    void parse_map(mapnik::Map& map)
    {
        //using spirit::utree_type;
        typedef utree::const_iterator iter;
        
        
        utree const& root_node = tree.ast();
        
        BOOST_ASSERT(get_node_type(root_node) == json_object);
        
        iter it = root_node.front().begin(),
            end = root_node.front().end();
        
        for (; it != end; ++it) {
            BOOST_ASSERT((*it).size()==2);
        
            std::string key = as<std::string>((*it).front());
            utree const& value = (*it).back();  
        
            if (key == "srs") {
                map.set_srs( as<std::string>(value) );
            } else if (key == "Stylesheet") {
                BOOST_ASSERT(get_node_type(value) == json_array);
                parse_stylesheet(map, value);
            } else if (key == "Layer") {
                BOOST_ASSERT(get_node_type(value) == json_array);
            
                iter lyr_it  = value.begin(), 
                     lyr_end = value.end();
            
                for (; lyr_it != lyr_end; ++lyr_it) {
                    BOOST_ASSERT(get_node_type(*lyr_it) == json_object);
                    parse_layer(map, (*lyr_it).front()) ;
                }
            } else {
                key_error(key, *it);
            }        
        }
        
        mapnik::Map::const_style_iterator s_it  =  map.begin_styles(),
                                          s_end =  map.end_styles();
        
        for(; s_it!=s_end; ++s_it) {
            std::string name   = (*s_it).first,
                        prefix = (*s_it).first;
            
            size_t loc = prefix.find(".",1);
            
            if (loc != std::string::npos)
                prefix.replace(loc,prefix.length(),"");
            
            if (prefix != "") {
                
                typedef std::map<std::string, int> lookup_type;
                
                lookup_type::const_iterator l_it  = layer_lookup.find(prefix),
                                            l_end = layer_lookup.end();
                if (l_it != l_end) {
                    map.getLayer(l_it->second).add_style(name);
                } else {
                    
                }
            }
        }
    }
    
    void parse_stylesheet(mapnik::Map& map, utree const& node)
    {
        namespace fs = boost::filesystem;
        
        typedef utree::const_iterator iter;
        
        iter it  = node.begin(), 
             end = node.end();
        
        fs::path parent_dir = fs::path(path).parent_path();
        
        style_env env;
        for (; it != end; ++it) {
            std::string data( as<std::string>(*it) );
            fs::path abs_path( data ),
                rel_path = parent_dir / abs_path;
            
            if (fs::exists(abs_path)) {
                mss_parser parser = load_mss(abs_path.string(), strict);
                parser.parse_stylesheet(map, env);
            } else if (fs::exists(rel_path)) {
                mss_parser parser = load_mss(rel_path.string(), strict);
                parser.parse_stylesheet(map, env);
            } else {
                mss_parser parser(data, strict, path);
                parser.parse_stylesheet(map, env);
            }
        }
    }
    
    void parse_layer(mapnik::Map& map, utree const& node)
    {
        mapnik::layer lyr("","");
        // FIXME - seems to be the default from carto js
        lyr.set_srs( "+proj=longlat +ellps=WGS84 +towgs84=0,0,0,0,0,0,0 +no_defs" );
        
        std::string lyr_name, lyr_id, lyr_class;
        
        typedef utree::const_iterator iter;
        iter it  = node.begin(), 
             end = node.end();
        
        for (; it != end; ++it) {
            BOOST_ASSERT((*it).size()==2);
        
            std::string key = as<std::string>((*it).front());
            utree const& value = (*it).back();
        
            if (key == "id") {
                lyr_id = "#"+as<std::string>(value);
            } else if (key == "class") {
                lyr_class = "."+as<std::string>(value);
            } else if (key == "name") {
                lyr_name = as<std::string>(value);
                lyr.set_name( lyr_name );
            } else if (key == "srs") {
                std::string srs = as<std::string>(value);
                if (srs != "") lyr.set_srs( srs );
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
                key_error(key, *it);
            }
        }
        
        map.addLayer(lyr);
    
        boost::algorithm::to_lower(lyr_name);
        boost::algorithm::replace_all(lyr_name, " ", "_");
    
        if (lyr_id.empty())
            lyr_id = "#"+lyr_name;
        if (lyr_class.empty())
            lyr_class = "."+lyr_name;
        
        typedef std::pair<std::string, mapnik::layer*> pair_type;
        
        //layer_lookup.insert( pair_type(lyr_id,    &map.layers().back()) );
        //layer_lookup.insert( pair_type(lyr_class, &map.layers().back()) );
        
        layer_lookup[lyr_id]    = map.layers().size()-1;
        layer_lookup[lyr_class] = map.layers().size()-1;
    }


    void parse_Datasource(mapnik::layer& lyr, utree const& node)
    {
        mapnik::parameters params;
        
        typedef utree::const_iterator iter;
        iter it  = node.begin(), 
             end = node.end();
        
        for (; it != end; ++it)
        {
            BOOST_ASSERT((*it).size()==2);

            std::string name  = as<std::string>((*it).front());
            std::string value = as<std::string>((*it).back());
            
            params[name] = value;
        }

        boost::optional<std::string> base_param = params.get<std::string>("base");
        boost::optional<std::string> file_param = params.get<std::string>("file");
        
        if (base_param) {
            params["base"] = ensure_relative_to_xml(base_param);
        } else if (file_param) {
            params["file"] = ensure_relative_to_xml(file_param);
        }
        
        try {
            boost::shared_ptr<mapnik::datasource> ds = mapnik::datasource_cache::instance()->create(params,false);
            lyr.set_datasource(ds);
        } catch (std::exception& e) {
            
            std::stringstream err;
            err << "Datasource creation issue ("
                << e.what() << ") at " << get_location(node).get_string(); 

            if (strict)
                throw config_error(err.str());
            else
                std::clog << "### WARNING: " << err.str() << "\n";
        }
    }

    std::string ensure_relative_to_xml( boost::optional<std::string> opt_path )
    {
        boost::filesystem::path mml_path = boost::filesystem::path(path);
        boost::filesystem::path rel_path = *opt_path;
        if ( !rel_path.has_root_path() ) 
        {
            #if (BOOST_FILESYSTEM_VERSION == 3)
            // TODO - normalize is now deprecated, use make_preferred?
            boost::filesystem::path full = boost::filesystem::absolute(mml_path.parent_path()/rel_path);
            #else // v2
            boost::filesystem::path full = boost::filesystem::complete(mml_path.branch_path()/rel_path).normalize();
            #endif
            return full.string();
        }
        return *opt_path;
    }
    
};




mml_parser load_mml(std::string filename, bool strict)
{
    std::ifstream file(filename.c_str(), std::ios_base::in);

    if (!file)
        throw config_error(std::string("Cannot open input file: ")+filename);
        
    
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
