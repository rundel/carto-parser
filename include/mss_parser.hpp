/*==============================================================================
    Copyright (c) 2010 Colin Rundel

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef MSS_PARSER_H
#define MSS_PARSER_H

#include <iosfwd>
#include <sstream>

#include <boost/functional/hash.hpp>

#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>

#include <mapnik/map.hpp>
#include <mapnik/config_error.hpp>

#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/version.hpp>
#include <mapnik/rule.hpp>

#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_path_parser.hpp>

#include <agg_trans_affine.h>

#include <expression_eval.hpp>
#include <generate/generate_filter.hpp>
#include <utility/utree.hpp>
#include <utility/environment.hpp>
#include <utility/version.hpp>
#include <utility/round.hpp>


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
        typedef position_iterator<std::string::const_iterator> iter;
        tree = build_parse_tree< carto_parser<iter> >(in, path);    
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
    
    int get_node_type(utree const& ut)
    {   
        return( tree.annotations(ut.tag()).second );
    }
    
    source_location get_location(utree const& ut)
    {    
        return tree.annotations()[ut.tag()].first;
    }

    template<class symbolizer>
    symbolizer* find_symbolizer(mapnik::rule& rule) 
    {
        typedef mapnik::rule::symbolizers::iterator iter;
        iter it  = rule.begin(),
             end = rule.end();
        
        for(; it!=end; ++it) {
            if (symbolizer *sym = boost::get<symbolizer>(&(*it)))
                return(sym);
        }
        
        BOOST_ASSERT(it==end);
        rule.append(init_symbolizer<symbolizer>());
        
        symbolizer *sym = boost::get<symbolizer>(& (*(--rule.end())) );
        return(sym);
    }
    
    template<class symbolizer>
    symbolizer init_symbolizer() 
    {
        return symbolizer();
    }

    mapnik::transform_type create_transform(std::string const& str)
    {
        agg::trans_affine tr;
        if (!mapnik::svg::parse_transform(str,tr))
        {
            std::stringstream err;
            err << "Could not parse transform from '" << str 
                << "', expected string like: 'matrix(1, 0, 0, 1, 0, 0)'";
            if (strict)
                throw config_error(err.str()); // value_error here?
            else
                std::clog << "### WARNING: " << err << std::endl;         
        }
        mapnik::transform_type matrix;
        tr.store_to(&matrix[0]);
        
        return matrix;
    }
    
    void key_error(std::string const& key, utree const& node) {
        
        std::stringstream err;
        err << "Unknown variable: @" << key
            << " at " << get_location(node).get_string() ; 
        
        if (strict)
            throw config_error(err.str());
        else
            std::clog << "### WARNING: " << err.str() << "\n";    
    }
    
    void parse_stylesheet(mapnik::Map& map, style_env& env)
    {
        using spirit::utree_type;
        
        utree const& root_node = tree.ast();
        
        typedef utree::const_iterator iter;
        iter it = root_node.begin(),
            end = root_node.end();
        
        for (; it != end; ++it) {
            switch((carto_node_type) get_node_type(*it)) {
                case carto_variable:
                    parse_variable(*it,env);
                    break;
                case carto_map_style:
                    parse_map_style(map, *it, env);
                    break;
                case carto_style:
                    parse_style(map, *it, env);
                    break;
                case carto_mixin:
                case carto_comment:
                    break;
                default:
                {
                    std::stringstream out;
                    out << "Invalid stylesheet node type: " << get_node_type(*it)
                        << " at " << get_location(*it).get_string();
                    throw config_error(out.str());
                }
            }
          
         }
    }
    
    void parse_style(mapnik::Map& map, utree const& node, style_env const& parent_env, 
                     mapnik::rule const& parent_rule = mapnik::rule(), std::string const& parent_name = "")
    {
        
        BOOST_ASSERT(node.size()==2);
        
        typedef utree::const_iterator iter;
        iter style_it  = node.front().begin(),
             style_end = node.front().end();
        
        for (; style_it != style_end; ++style_it) {
            
            mapnik::feature_type_style* style;
            mapnik::rule rule(parent_rule);
            style_env env(parent_env);
            
            BOOST_ASSERT(*style_it.size() == 3);
            iter name_it  = (*style_it).begin(),
                 name_end = (*style_it).end();
            
            utree const& uname   = *name_it; name_it++;
            utree const& uattach = *name_it; name_it++;
            utree const& ufilter = *name_it;
            
            
            
            std::string name = parent_name + as<std::string>(uname);
            
            mapnik::Map::style_iterator map_it, map_end;
            
            map_it  = map.styles().find(name);                           
            map_end = map.styles().end();
            
            if (map_it == map_end) {
                map.insert_style(name, mapnik::feature_type_style());
                map_it = map.styles().find(name);
            }
            style = &map_it->second;
            
            
            if (uattach.size() != 0) {
                name += "-"+as<std::string>(uattach);
                
                map.insert_style(name, mapnik::feature_type_style(*style));
                map_it = map.styles().find(name);
                style = &map_it->second;
            }
            
            if (ufilter.size() != 0) {
                BOOST_ASSERT(get_node_type(ufilter) == carto_filter);
                parse_filter(map, ufilter, env, rule);
            }
            
            
            
            iter it  = node.back().begin(),
                 end = node.back().end();
        
            for (; it != end; ++it) {
                switch(get_node_type(*it)) {
                    case carto_variable:
                        parse_variable(*it,env);
                        break;
                    case carto_style:
                        parse_style(map, *it, env, rule, name);
                        break;
                    case carto_attribute:
                        parse_attribute(map, *it, env, rule);                    
                        break;
                    case carto_mixin:
                    case carto_comment:
                        break;
                    default:
                        std::stringstream out;
                        out << "Invalid style node type: " << get_node_type(*it)
                            << " at " << get_location(*it).get_string();
                        throw config_error(out.str());
                }
            }
            
            //if (rule.get_symbolizers().size() != 0 && !(rule == parent_rule)) {
            if (rule.get_symbolizers().size() != 0)
                style->add_rule(rule);
        }
    }
    
    void parse_filter(mapnik::Map& map, utree const& node, style_env const& env, mapnik::rule& rule)
    {
        if (node.size() == 0)
            return;
            
        typedef utree::const_iterator iter;
        iter it  = node.begin(),
             end = node.end();
        
        std::string out, cur_filter = mapnik::to_expression_string(rule.get_filter());
        
        if (cur_filter != "true") {
            out += "(" + cur_filter + ") and ";
        }
        
        for (; it != end; ++it) 
        {
            if (it != node.begin())
                out += " and ";
            
            std::string str;
            generate_filter(*it,tree.annotations(), env, str);
            
            out += str;
        }
        
        // FIXME
        rule.set_filter(mapnik::parse_expression(out,"utf8"));
        
        //std::string s = mapnik::to_expression_string(rule.get_filter());
    }
    
    utree eval_var(utree const& node, style_env const& env) {
        std::string key = as<std::string>(node.front());
        
        utree value = env.vars.lookup(key);
        
        if (value == utree::nil_type()) {
            std::stringstream err;
            err << "Unknown variable: @" << key
                << " at " << get_location(node).get_string(); 
            throw config_error(err.str());
        }
        
        return (get_node_type(value) == carto_variable) ? eval_var(value,env) : value;
    }
    
    utree parse_value(utree const& node, style_env const& env) 
    {
        if (get_node_type(node) == carto_variable) {
            return eval_var(node, env); // vars can point at other vars
        } else if (get_node_type(node) == carto_expression) {
            //BOOST_ASSERT(node.size()==1);
            expression exp(node.front().front(), tree.annotations(), env);
            return exp.eval();
        } else {
            if (node.size() == 1)
                return node.front();
            else 
                return node;
        }
    }
    
    void parse_attribute(mapnik::Map& map, utree const& node, style_env const& env, mapnik::rule& rule) 
    {
        BOOST_ASSERT(node.size()==2);
        
        std::string key = as<std::string>(node.front());
        utree value = parse_value(node.back(), env);
        
        if (key.substr(0,8) == "polygon-")
            parse_polygon(rule,key,value,env);
        else if (key.substr(0,5) == "line-")  
            parse_line(rule,key,value,env);
        else if (key.substr(0,7) == "marker-")
            parse_marker(rule,key,value,env);
        else if (key.substr(0,6) == "point-")
            parse_point(rule,key,value,env);
        else if (key.substr(0,13)== "line-pattern-")
            parse_line_pattern(rule,key,value,env);
        else if (key.substr(0,16)== "polygon-pattern-") 
            parse_polygon_pattern(rule,key,value,env);
        else if (key.substr(0,7) == "raster-")
            parse_raster(rule,key,value,env);
        else if (key.substr(0,9) == "building-")
            parse_building(rule,key,value,env);
        else if (key.substr(0,5) == "text-")
            parse_text(map,rule,key,value,env);
        else if (key.substr(0,7) == "shield-")
            parse_shield(rule,key,value,env);
        else 
            key_error(key, node);
    }
    
    bool parse_polygon(mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::polygon_symbolizer *s = find_symbolizer<mapnik::polygon_symbolizer>(rule);
        
        if (key == "polygon-fill") {
            s->set_fill(as<mapnik::color>(value));
        } else if (key == "polygon-gamma") {
            s->set_gamma(as<double>(value));
        } else if (key == "polygon-opacity") {
            s->set_opacity(as<double>(value));
        } else {
            return false;
        }
        return true;
    }
    
    bool parse_line(mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::line_symbolizer *s = find_symbolizer<mapnik::line_symbolizer>(rule);
        mapnik::stroke strk = s->get_stroke();
        
        if (key == "line-dasharray") {
            
            BOOST_ASSERT( (value.size()-1) % 2 == 0 );
            
            typedef utree::const_iterator iter;
            iter it = value.begin(),
                end = value.end();
            
            for(; it!=end;) {
                double dash = as<double>(*it); it++;
                double gap  = as<double>(*it); it++;
                
                strk.add_dash(dash,gap);
            }
        } else if (key == "line-color") {
            strk.set_color(as<mapnik::color>(value));
        } else if (key == "line-width") {
            strk.set_width(as<double>(value));
        } else if (key == "line-opacity") {
            strk.set_opacity(as<double>(value));
        } else if (key == "line-join") {
            mapnik::line_join_e en;
            en.from_string(as<std::string>(value));
            strk.set_line_join(en);
        } else if (key == "line-cap") {
            mapnik::line_cap_e en;
            en.from_string(as<std::string>(value));
            strk.set_line_cap(en);
        } else if (key == "line-gamma") {
            strk.set_gamma(as<double>(value));
        } else if (key == "line-dash-offset") {
            strk.set_dash_offset(as<double>(value));
        } else {
            return false;
        }
        s->set_stroke(strk);
        return true;
    }
    
    bool parse_marker(mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::markers_symbolizer *s = find_symbolizer<mapnik::markers_symbolizer>(rule); 
        mapnik::stroke stroke = s->get_stroke();

        if (key == "marker-file") {
            s->set_filename(mapnik::parse_path(as<std::string>(value)));
        } else if (key == "marker-opacity") {
            s->set_opacity(as<float>(value));
        } else if (key == "marker-line-color") {
            stroke.set_color(as<mapnik::color>(value));
        } else if (key == "marker-line-width") {
            stroke.set_width(as<double>(value));
        } else if (key == "marker-line-opacity") {
            stroke.set_opacity(as<double>(value));
        } else if (key == "marker-placement") {
            mapnik::marker_placement_e en;
            en.from_string(as<std::string>(value));
            s->set_marker_placement(en);
        } else if (key == "marker-type") {
            mapnik::marker_type_e en;
            en.from_string(as<std::string>(value));
            s->set_marker_type(en);
        } else if (key == "marker-width") {
            s->set_width(as<double>(value));
        } else if (key == "marker-height") {
            s->set_height(as<double>(value));
        } else if (key == "marker-fill") {
            s->set_fill(as<mapnik::color>(value));
        } else if (key == "marker-allow-overlap") {
            s->set_allow_overlap(as<bool>(value));
        } else if (key == "marker-spacing") {
            s->set_spacing(as<double>(value));
        } else if (key == "marker-max-error") {
            s->set_max_error(as<double>(value));
        } else if (key == "marker-transform") {
            s->set_transform(create_transform(as<std::string>(value)));
        } else {
            return false;
        }
        return true;
    }
    
    bool parse_point(mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::point_symbolizer *s = find_symbolizer<mapnik::point_symbolizer>(rule);
        
        if (key == "point-file") {
            s->set_filename(mapnik::parse_path(as<std::string>(value)));
        } else if (key == "point-allow-overlap") {
            s->set_allow_overlap(as<bool>(value));
        } else if (key == "point-ignore-placement") {
            s->set_ignore_placement(as<bool>(value));
        } else if (key == "point-opacity") {
            s->set_opacity(as<float>(value));
        } else if (key == "point-placement") {
            mapnik::point_placement_e en;
            en.from_string(as<std::string>(value));
            s->set_point_placement(en);
        } else if (key == "point-transform") {
            s->set_transform(create_transform(as<std::string>(value)));
        } else {
            return false;
        }
        return true;
    }
    
    bool parse_line_pattern(mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::line_pattern_symbolizer *s = find_symbolizer<mapnik::line_pattern_symbolizer>(rule);
        
        if (key == "line-pattern-file") {
            s->set_filename(mapnik::parse_path(as<std::string>(value)));
        } else {
            return false;
        }
        return true;
    }
    
    bool parse_polygon_pattern(mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::polygon_pattern_symbolizer *s = find_symbolizer<mapnik::polygon_pattern_symbolizer>(rule);
        
        if (key == "polygon-pattern-file") {
            s->set_filename(mapnik::parse_path(as<std::string>(value)));
        } else if (key == "polygon-pattern-alignment") {
            mapnik::pattern_alignment_e en;
            en.from_string(as<std::string>(value));
            s->set_alignment(en);
        } else {
            return false;
        }
        return true;
    }
    
    bool parse_raster(mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::raster_symbolizer *s = find_symbolizer<mapnik::raster_symbolizer>(rule);
        
        if (key == "raster-opacity") {
            s->set_opacity(as<float>(value));
        } else if (key == "raster-mode") {
            s->set_mode(as<std::string>(value));
        } else if (key == "raster-scaling") {
            s->set_scaling(as<std::string>(value));
        } else {
            return false;
        }
        return true;
    }
    
    bool parse_building(mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::building_symbolizer *s = find_symbolizer<mapnik::building_symbolizer>(rule);
        
        if (key == "building-fill") {
            s->set_fill(as<mapnik::color>(value));
        } else if (key == "building-fill-opacity") {
            s->set_opacity(as<double>(value));
        } else if (key == "building-height") {
            s->set_height(as<double>(value));
        } else {
            return false;
        }
        return true;
    }
    
    bool parse_text(mapnik::Map& map, mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::text_symbolizer *s = find_symbolizer<mapnik::text_symbolizer>(rule);
        
        using boost::spirit::utree_type;
        
        if (key == "text-face-name") {
            if (value.which() != utree_type::list_type) {
                s->set_face_name(as<std::string>(value));
            } else {
                typedef utree::const_iterator iter;
                iter it, end;
                
                it  = value.begin();
                end = value.end();
                
                std::size_t seed = 0;
                for( ; it!=end; ++it)
                    boost::hash_combine(seed, as<std::string>(*it));
                
                std::stringstream ss;
                ss << std::hex << seed;
                
                std::string name = ss.str();
                
                // FIXME - font_set does not have a/ set_name method so have to do this with two loops
                it  = value.begin();
                end = value.end();
                
                mapnik::font_set fs(name);
                for( ; it!=end; ++it)
                    fs.add_face_name(as<std::string>(*it));
                
                s->set_fontset(fs);
                s->set_face_name(std::string());
                map.insert_fontset(name, fs);
            }
        } else if (key == "text-name") {
            s->set_name(mapnik::parse_expression(as<std::string>(value)));
        } else if (key == "text-size") {
            s->set_text_size(round(as<double>(value)));
        } else if (key == "text-ratio") {
            s->set_text_ratio(round(as<double>(value)));
        } else if (key == "text-wrap-width") {
            s->set_wrap_width(round(as<double>(value)));
        } else if (key == "text-spacing") {
            s->set_label_spacing(round(as<double>(value)));
        } else if (key == "text-character-spacing") {
            s->set_character_spacing(round(as<double>(value)));
        } else if (key == "text-line-spacing") {
            s->set_line_spacing(round(as<double>(value)));
        } else if (key == "text-label-position-tolerance") {
            s->set_label_position_tolerance(round(as<double>(value)));
        } else if (key == "text-max-char-angle-delta") {
            s->set_max_char_angle_delta(as<double>(value));
        } else if (key == "text-fill") {
            s->set_fill(as<mapnik::color>(value));
        } else if (key == "text-opacity") {
            s->set_text_opacity(as<double>(value));
        } else if (key == "text-halo-fill") {
            s->set_halo_fill(as<mapnik::color>(value));
        } else if (key == "text-halo-radius") {
            s->set_halo_radius(as<double>(value));
        } else if (key == "text-dx") {
            double x = as<double>(value);
            double y = s->get_displacement().get<1>();
            s->set_displacement(x,y);
        } else if (key == "text-dy") {
            double x = s->get_displacement().get<0>();
            double y = as<double>(value);
            s->set_displacement(x,y);
        } else if (key == "text-vertical-alignment") {
            mapnik::vertical_alignment_e en;
            en.from_string(as<std::string>(value));
            s->set_vertical_alignment(en);
        } else if (key == "text-avoid-edges") {
            s->set_avoid_edges(as<bool>(value));
        } else if (key == "text-min-distance") {
            s->set_minimum_distance(as<double>(value));
        } else if (key == "text-min-padding") {
            s->set_minimum_padding(as<double>(value));
        } else if (key == "text-allow-overlap") {
            s->set_allow_overlap(as<bool>(value));
        } else if (key == "text-placement") {
            mapnik::label_placement_e en;
            en.from_string(as<std::string>(value));
            s->set_label_placement(en);
        } else if (key == "text-placement-type") {
            // FIXME
        } else if (key == "text-placements") {
            // FIXME
        } else if (key == "text-transform") {
            mapnik::text_transform_e en;
            en.from_string(as<std::string>(value));
            s->set_text_transform(en);
        } else {
            return false;
        }
        return true;
    }
    
    
    bool parse_shield(mapnik::rule& rule, std::string const& key, utree const& value, style_env const& env) 
    {
        mapnik::shield_symbolizer *s = find_symbolizer<mapnik::shield_symbolizer>(rule);
        
        if (key == "shield-name") {
            s->set_name(mapnik::parse_expression(as<std::string>(value)));
        } else if (key == "shield-face-name") {
            s->set_face_name(as<std::string>(value));
        } else if (key == "shield-size") {
            s->set_text_size(round(as<double>(value)));
        } else if (key == "shield-spacing") {
            s->set_label_spacing(round(as<double>(value)));
        } else if (key == "shield-character-spacing") {
            s->set_character_spacing(round(as<double>(value)));
        } else if (key == "shield-line-spacing") {
            s->set_line_spacing(round(as<double>(value)));
        } else if (key == "shield-fill") {
            s->set_fill(as<mapnik::color>(value));
        } else if (key == "shield-text-dx") {
            double x = as<double>(value);
            double y = s->get_displacement().get<1>();
            s->set_displacement(x,y);
        } else if (key == "shield-text-dy") {
            double x = s->get_displacement().get<0>();
            double y = as<double>(value);
            s->set_displacement(x,y);
        } else if (key == "shield-dx") {
            double x = as<double>(value);
            double y = s->get_shield_displacement().get<1>();
            s->set_shield_displacement(x,y);
        } else if (key == "shield-dy") {
            double x = s->get_shield_displacement().get<0>();
            double y = as<double>(value);
            s->set_shield_displacement(x,y);
        } else if (key == "shield-min-distance") {
            s->set_minimum_distance(as<double>(value));
        } else if (key == "shield-placement") {
            mapnik::label_placement_e en;
            en.from_string(as<std::string>(value));
            s->set_label_placement(en);
        } else {
            return false;
        }
        return true;
    }
    
    void parse_variable(utree const& node, style_env& env)
    {
        std::string name = as<std::string>(node.front());
        utree val = parse_value(node.back(), env);
        env.vars.define(name, val);
    }
    
    void parse_map_style(mapnik::Map& map, utree const& node, style_env& env) 
    {
        typedef utree::const_iterator iter;
        iter it = node.begin(),
            end = node.end();
        
        mapnik::parameters extra_attr;
        bool relative_to_xml = true;
        
        for (; it != end; ++it) {
            
            BOOST_ASSERT((*it).size()==2);
            
            if (get_node_type(*it) == carto_variable) {
                parse_variable(*it,env);
                break;
            }
            
            std::string key = as<std::string>((*it).front());
            utree const& value = parse_value((*it).back(),env);
            
            std::string base = "";

            if (key == "srs") {
                map.set_srs(as<std::string>(value));
            } else if (key == "background-color") {
                BOOST_ASSERT((carto_node_type) get_node_type(value) == carto_color);
                map.set_background(as<mapnik::color>(value));
            } else if (key == "background-image") {
                map.set_background_image(base+as<std::string>(value));
            } else if (key == "buffer-size") {
                map.set_buffer_size(round(as<double>(value)));
            } else if (key == "base") {
                base = as<std::string>(value); // FIXME - not sure this is correct
            } else if (key == "paths-from-xml") {
                relative_to_xml = as<bool>(value);
            } else if (key == "minimum-version") {
                std::string ver_str = as<std::string>(value);
                extra_attr["minimum-version"] = ver_str;
                
                int min_ver = version_from_string(ver_str);
                
                if (min_ver == -1 && strict) {
                    throw config_error(std::string("Invalid version string ") + ver_str);
                } else if (min_ver > MAPNIK_VERSION) {
                    throw config_error(std::string("This map uses features only present in Mapnik version ") + ver_str + " and newer");
                }
            } 
            else if (key == "font-directory") {
                std::string dir = base+as<std::string>(value);
                extra_attr["font-directory"] = dir;
                //freetype_engine::register_fonts( ensure_relative_to_xml(dir), false);
            } else {
                key_error(key,node);
            }
        }
        
        map.set_extra_attributes(extra_attr);

        return;
    }
    
};

template<>
mapnik::text_symbolizer mss_parser::init_symbolizer<mapnik::text_symbolizer>() 
{
    return mapnik::text_symbolizer(mapnik::expression_ptr(), "<no default>", 0, 
                                   mapnik::color(0,0,0) );
}

template<>
mapnik::shield_symbolizer mss_parser::init_symbolizer<mapnik::shield_symbolizer>() 
{
    return mapnik::shield_symbolizer(mapnik::expression_ptr(), "<no default>", 0, 
                                     mapnik::color(0,0,0), mapnik::path_expression_ptr());
}

template<>
mapnik::polygon_pattern_symbolizer mss_parser::init_symbolizer<mapnik::polygon_pattern_symbolizer>() 
{
    return mapnik::polygon_pattern_symbolizer(mapnik::parse_path(""));
}

template<>
mapnik::line_pattern_symbolizer mss_parser::init_symbolizer<mapnik::line_pattern_symbolizer>() 
{
    return mapnik::line_pattern_symbolizer(mapnik::parse_path(""));
}

mss_parser load_mss(std::string filename, bool strict)
{
    std::ifstream file(filename.c_str(), std::ios_base::in);

    if (!file)
        throw config_error(std::string("Cannot open input file: ")+filename);
    
    std::string in;
    file.unsetf(std::ios::skipws);
    copy(std::istream_iterator<char>(file),
         std::istream_iterator<char>(),
         std::back_inserter(in));

    return mss_parser(in, strict, filename);
}

}
#endif 
