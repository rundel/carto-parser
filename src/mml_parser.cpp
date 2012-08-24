/*==============================================================================
    Copyright (c) 2010 Colin Rundel

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <mml_parser.hpp>
#include <mss_parser.hpp>

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
  
mml_parser::mml_parser(parse_tree const& pt, bool strict_, std::string const& path_)
  : tree(pt),
    strict(strict_),
    path(path_) { }
  
mml_parser::mml_parser(std::string const& in, bool strict_, std::string const& path_)
  : strict(strict_),
    path(path_) 
{ 
    typedef mapnik::position_iterator<std::string::const_iterator> it_type;
    tree = build_parse_tree< json_parser<it_type> >(in, path);    
}

parse_tree mml_parser::get_parse_tree()
{
    return tree;
}

std::string mml_parser::get_path()
{
    return path;
}

node_type mml_parser::get_node_type(utree const& ut)
{   
    return( (node_type) tree.annotations(ut.tag()).second );
}

mapnik::source_location mml_parser::get_location(utree const& ut)
{    
    return tree.annotations()[ut.tag()].first;
}

void mml_parser::key_error(std::string const& key, utree const& node) {
    
    std::stringstream err;
    err << "Unknown keyword: " << key
        << " at " << mapnik::get_location(node).get_string(); 
    
    if (strict)
        throw config_error(err.str());
    else
        std::clog << "### WARNING: " << err.str() << "\n";    
}

void mml_parser::parse_map(mapnik::Map& map)
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
    
    typedef std::pair< std::string, std::vector<std::string> > style_pair;
    std::vector<style_pair> style_selectors;
    
    mapnik::Map::const_style_iterator s_it  =  map.begin_styles(),
                                      s_end =  map.end_styles();
    
    for(; s_it!=s_end; ++s_it) {
        std::string name = (*s_it).first;
        
        // remove attachment from style name
        size_t loc = name.find("::");
        if (loc != std::string::npos) {
            al::erase_tail(name, name.length()-loc);
        }
        
        al::trim_if(name,al::is_any_of(". "));
        std::vector<std::string> selectors;
        al::split(selectors, name, al::is_any_of(". "));
        
        style_selectors.push_back(style_pair((*s_it).first,selectors));
    }
    
    typedef std::vector< std::vector<std::string> >::const_iterator layer_it_type;
    layer_it_type layer_it  = layer_selectors.begin(),
                  layer_end = layer_selectors.end();
    
    for(size_t i=0; layer_it != layer_end; ++layer_it, ++i) {
        
        typedef std::vector<style_pair>::const_iterator style_it_type;
        style_it_type style_it  = style_selectors.begin(),
                      style_end = style_selectors.end();
        
        for(; style_it != style_end; ++style_it) {
            typedef std::vector<std::string>::const_iterator selector_it_type;
        
            selector_it_type lselect_it  = (*layer_it).begin(),
                             lselect_end = (*layer_it).end(),
                             sselect_it  = (*style_it).second.begin(),
                             sselect_end = (*style_it).second.end();
            
            for(; lselect_it != lselect_end; ++lselect_it) {
                while(sselect_it != sselect_end && *lselect_it != *sselect_it) {
                    ++sselect_it;
                }
                
                if (sselect_it == sselect_end) break;
            }
            
            if (lselect_it == lselect_end && sselect_it != sselect_end)
                map.getLayer(i).add_style((*style_it).first);
        }
    }
}

void mml_parser::parse_stylesheet(mapnik::Map& map, utree const& node)
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

void mml_parser::parse_layer(mapnik::Map& map, utree const& node)
{
    mapnik::layer lyr("");
    
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
            lyr_class = as<std::string>(value);
        } else if (key == "name") {
            lyr_name = as<std::string>(value);
            lyr.set_name( lyr_name );
        } else if (key == "srs") {
            std::string srs = as<std::string>(value);
            if (srs != "") lyr.set_srs( srs );
        } else if (key == "status") {
            lyr.set_active( as<bool>(value) );
        } else if (key == "minzoom") {
            lyr.set_min_zoom( value.get<double>() );
        } else if (key == "maxzoom") {
            lyr.set_max_zoom( value.get<double>() );
        } else if (key == "queryable") {
        lyr.set_queryable( value.get<bool>() );
        } else if (key == "Datasource") {
            BOOST_ASSERT(get_node_type(value) == json_object);
            parse_Datasource(lyr, value.front());
        } else {
            key_error(key, *it);
        }
    }
    
    map.addLayer(lyr);
    
    layer_selectors.push_back( std::vector<std::string>() );
    int i = layer_selectors.size()-1;
    
    BOOST_ASSERT(layer_selectors.size() == map.layer_count());
    
    if (!lyr_id.empty())
        layer_selectors[i].push_back(lyr_id);
    if (!lyr_class.empty())
        al::split(layer_selectors[i], lyr_class, al::is_any_of(". "), al::token_compress_on);
}


void mml_parser::parse_Datasource(mapnik::layer& lyr, utree const& node)
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
            << e.what() << ") at " << mapnik::get_location(node).get_string()
            << " " << node;
        if (strict)
            throw config_error(err.str());
        else
            std::clog << "### WARNING: " << err.str() << "\n";
    }
}

std::string mml_parser::ensure_relative_to_xml( boost::optional<std::string> opt_path )
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

