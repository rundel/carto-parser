/*==============================================================================
    Copyright (c) 2010 Colin Rundel

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef MSS_PARSER_H
#define MSS_PARSER_H

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
#include <mapnik/filter_factory.hpp>
#include <mapnik/version.hpp>
#include <mapnik/rule.hpp>

//#include <base_parser.hpp>
#include <utree/utree_utility.hpp>
#include <utility/environment.hpp>
#include <utility/version.hpp>


namespace carto {

using mapnik::config_error;

struct style_env {
    environment vars;
    environment mixins;
    
    style_env() 
      : vars(),
        mixins() { }
        
    //style_env(environment vars_, environment mixins_)
    //  : vars(vars_),
    //    mixins(mixins_) { }
        
    style_env(style_env const& env)
      : vars(env.vars),
        mixins(env.mixins) { }
};

struct mss_parser {//}: public base_parser {

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

    //parse_tree tree;
    //bool strict;
    //std::string path;
    //
    mss_parser(parse_tree const& pt, bool strict_ = false, std::string const& path_ = "./")
      : tree(pt),
        strict(strict_),
        path(path_) { }
      
    mss_parser(std::string const& in, bool strict_ = false, std::string const& path_ = "./")
      : strict(strict_),
        path(path_) 
    { 
        typedef position_iterator<std::string::const_iterator> iter;
        tree = build_parse_tree< carto_parser<iter> >(in);    
    }
    
    void parse_stylesheet(mapnik::Map& map)
    {
        using spirit::utree_type;
        
        style_env env;
        
        utree const root_node = tree.ast();
        
        typedef utree::const_iterator iter;
        iter it = root_node.begin(),
            end = root_node.end();
        
        for (; it != end; ++it) {
            try {
                switch((carto_node_type) get_node_type(*it)) {
                    case carto_variable:
                        env.vars.define(as<std::string>((*it).front()), (*it).back());
                        break;
                    case carto_mixin:
                    
                        break;
                    case carto_style:
                        parse_style(map, *it, env);
                        break;
                    default:
                    {
                        std::stringstream out;
                        out << "Invalid stylesheet node type: " << get_node_type(*it) << "\n";
                        throw config_error(out.str());
                    }
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
    
    void parse_style(mapnik::Map& map, utree const& node, style_env const& env_, mapnik::rule const& rule_ = mapnik::rule())
    {
        mapnik::rule rule(rule_);
        style_env env(env_);
        
        std::string name = as<std::string>(node.front());
        
        if (name == "Map") {
            parse_map_style(map, node, env);
        } else if (name[0] == '#' || name[0] == '.' || name == "") {
            rule.set_name(name);
            
            parse_rule(map, node, env, rule);
            
            mapnik::feature_type_style style;
            style.add_rule(rule);
            map.insert_style(name, style);
            
        } else {
            throw config_error(std::string("Invalid style name: ")+name);
        }
    }
    
    void parse_rule(mapnik::Map& map, utree const& node, style_env& env, mapnik::rule& rule) 
    {
        
        typedef utree::const_iterator iter;
        iter it = ++node.begin(),
            end = node.end();
        
        for (; it != end; ++it) {
            switch((carto_node_type) get_node_type(*it)) {
                case carto_variable:
                    env.vars.define(as<std::string>((*it).front()), (*it).back());
                    break;
                case carto_filter:
                    //rule.set_filter(mapnik::parse_expression(as<std::string>((*it)),"utf8"));
                    break;
                case carto_mixin:
                
                    break;
                case carto_style:
                    parse_style(map, *it, env, rule);
                    break;
                case carto_attribute:
                    parse_rule_attribute(map, *it, env, rule);                    
                    break;
                default:
                    std::stringstream out;
                    out << "Invalid style node type: " << get_node_type(*it) << "\n";
                    throw config_error(out.str());
            }
        }
    }
    
    void parse_rule_attribute(mapnik::Map& map, utree const& node, style_env& env, mapnik::rule& rule) 
    {
        std::string key = as<std::string>(node.front());
        utree const& value = node.back();
        
        if (key.substr(0,8) == "polygon-")
            parse_polygon(rule,key,value);
        /*
        else if (key.substr(0,5) == "line-")  
            parse_line(rule,value,env);
        else if (key.substr(0,7) == "marker-")
            parse_marker(rule,value,env);
        else if (key.substr(0,7) == "shield-")
            parse_shield(rule,value,env);
        else if (key.substr(0,13)== "line-pattern-")
            parse_line_pattern(rule,value,env);
        else if (key.substr(0,16)== "polygon-pattern-") 
            parse_polygon_pattern(rule,value,env);
        else if (key.substr(0,7) == "raster-")
            parse_raster(rule,value,env);
        else if (key.substr(0,6) == "point-")
            parse_point(rule,value,env);
        else if (key.substr(0,5) == "text-")
            parse_text(rule,value,env);
        else if (key.substr(0,9) == "building-")
            parse_building(rule,value,env);
        else 
            config_error(std::string("Unknown attribute type: ")+key);
        */
    }
        
    void parse_polygon(mapnik::rule& rule, std::string const& key, utree const& value) 
    {
        typedef mapnik::rule::symbolizers::iterator iter;
        iter it  = rule.begin(),
             end = rule.end();
        
        mapnik::polygon_symbolizer *s;
        for(; it!=end; ++it) {
            if (mapnik::polygon_symbolizer *sym = boost::get<mapnik::polygon_symbolizer>(&(*it))) {
                s = sym;
                break;
            }
        }
        
        if (it==end) {
            rule.append(mapnik::polygon_symbolizer());
            
            mapnik::polygon_symbolizer *sym = boost::get<mapnik::polygon_symbolizer>(& (*(--rule.end())) );
            s = sym;
        }
        
        if (key == "polygon-fill") {
            s->set_fill(as<mapnik::color>(value));
        } else if (key == "polygon-gamma") {
            s->set_gamma(as<double>(value));
        } else if (key == "polygon-opacity") {
            s->set_opacity(as<double>(value));
        } else {
            throw config_error(std::string("Unknown attribute keyword: ")+key);
        }
        
    }
    
    void parse_map_style(mapnik::Map& map, utree const& node, style_env& env) 
    {
        typedef utree::const_iterator iter;
        iter it = ++node.begin(),
            end = node.end();
        
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
                BOOST_ASSERT((carto_node_type) get_node_type(value) == carto_color);
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
                //freetype_engine::register_fonts( ensure_relative_to_xml(dir), false);
            } else {
                throw config_error(std::string("Unknown attribute keyword: ")+key);
            }
        }
        
        map.set_extra_attributes(extra_attr);

        return;
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

}
#endif 
