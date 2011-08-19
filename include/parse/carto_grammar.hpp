/*==============================================================================
    Copyright (c) 2010 Object Modeling Designs
    Copyright (c) 2010 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef CARTO_GRAMMAR_H
#define CARTO_GRAMMAR_H

#include <limits>

#include <boost/spirit/include/phoenix.hpp>

#include <mapnik/css_color_grammar.hpp>

#include <parse/string_grammar.hpp>
#include <parse/filter_grammar.hpp>
#include <parse/error_handler.hpp>
#include <parse/annotator.hpp>

namespace carto {

namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

using ascii::space_type;
using boost::spirit::utf8_symbol_type;

enum carto_node_type
{
    carto_undefined,
    carto_variable,
    carto_mixin,
    carto_style,
    carto_map_style,
    carto_filter,
    carto_function,
    carto_attribute,
    carto_color
};


struct color_conv_impl
{
    template <typename T>
    struct result
    {
        typedef utree::list_type type;
    };
    
    utree::list_type operator() (mapnik::color color) const
    {
        utree::list_type u;
        u.push_back(color.red());
        u.push_back(color.green());
        u.push_back(color.blue());
        u.push_back(color.alpha());
        
        return u;
    }
};


template<typename Iterator>
struct carto_parser : qi::grammar< Iterator, utree::list_type(), space_type>
{

    qi::rule<Iterator, utree(), space_type> value, element, filter, ustring, var_value;
    qi::rule<Iterator, utree::list_type(), space_type> start, variable, attribute, map_style,
                                                       style, style_name_list, style_name_filter, style_element_list,
                                                       color, mixin, function, filter_list;
    qi::rule<Iterator, utf8_symbol_type()> name, var_name, style_name, enum_val;
    qi::rule<Iterator, utree::nil_type()> null;

    mapnik::css_color_grammar<Iterator> css_color;

    utf8_string_parser<Iterator> utf8;
    filter_parser<Iterator> filter_text;
    
    phoenix::function<color_conv_impl> css_conv;

    typedef error_handler_impl<Iterator> error_handler_type;
    phoenix::function<error_handler_type> const error;
    annotator<Iterator> annotate;

    carto_parser (std::string const& source, annotations_type& annotations)
      : carto_parser::base_type(start),
        utf8(source),
        filter_text(source, annotations),
        error(error_handler_type(source)),
        annotate(annotations)
    {
        using qi::char_;
        using qi::lexeme;
        using qi::double_;
        using qi::bool_;
        using qi::lit;
        using qi::_val;
        using qi::lit;
        using qi::omit;

        qi::as<utf8_symbol_type> as_symbol;
        
        start = +(variable | map_style | style);
        
        element = omit["/*" >> *(char_ - "*/") >> "*/"]
                  | variable
                  | attribute
                  | style;
              
        //std::string exclude = std::string(" {}[]:\"\x01-\x1f\x7f") + '\0';
        name = char_("a-zA-Z_") > *char_("a-zA-Z0-9_-");
        
        var_name = lexeme["@" > name];
        
        variable %= as_symbol[var_name] > ":" > (value % ",") > ";"
                    > annotate(_val, carto_variable);
        
        attribute %= as_symbol[name] > ":" > (value % ",") > ";"
                     > annotate(_val, carto_attribute);
        
        
        //filter_text = lexeme[*(char_-"]")];
        filter = lit("[") > filter_text > "]";
        filter_list = *filter > annotate(_val, carto_filter);;
        
        style_name = lexeme[-char_("#.") >> name];
        style_name_filter = as_symbol[-style_name] >> filter_list;
        style_name_list = style_name_filter % ",";
        
        style_element_list = "{" > *element > "}";
        style = style_name_list >> style_element_list > annotate(_val, carto_style);
        //style = style_name_list >> "{" > *element > "}" > annotate(_val, carto_style);
        
        
        map_style = lit("Map") > "{" > *(variable | attribute) > "}" > annotate(_val, carto_map_style);
        
        enum_val = lexeme[+(char_("a-zA-Z_-"))];
        ustring = lexeme[char_("'") > *(char_-'\'') > char_("'")];
        
        var_value = var_name > annotate(_val, carto_variable);
        
        value =   null
                | color
                | double_
                | bool_
                | utf8
                | ustring
                | enum_val
                | function
                | mixin
                | var_value;
                
        mixin = style_name > -("(" > name > ")") > ";" > annotate(_val, carto_mixin);
        
        function = name > "(" > (value % ",") > ")" > annotate(_val, carto_function);
        
        color =   css_color[_val = css_conv(qi::_1)]
                > annotate(_val, carto_color);
             
        null = "null" >> qi::attr(spirit::nil); 


        std::string base = "mss";

        start.name(base+":start");
        value.name(base+":value");
        element.name(base+":element");
        filter.name(base+":filter");
        filter_text.name(base+":filter_text");
        variable.name(base+":variable");
        attribute.name(base+":attribute");
        style.name(base+":style");
        color.name(base+":color");
        mixin.name(base+":mixin");
        function.name(base+":function");
        name.name(base+":name");
        var_name.name(base+":var_name");
        style_name.name(base+":style_name");
        null.name(base+":null");

 
        qi::on_error<qi::fail>(start, error(qi::_3, qi::_4));
        
        //BOOST_SPIRIT_DEBUG_NODE(start);
        //BOOST_SPIRIT_DEBUG_NODE(variable);
        //BOOST_SPIRIT_DEBUG_NODE(attribute);
        //BOOST_SPIRIT_DEBUG_NODE(map_style);
        //BOOST_SPIRIT_DEBUG_NODE(style);
        //BOOST_SPIRIT_DEBUG_NODE(style_name_list);
        //BOOST_SPIRIT_DEBUG_NODE(style_element_list);
        //BOOST_SPIRIT_DEBUG_NODE(style_name_filter);
        //BOOST_SPIRIT_DEBUG_NODE(color);
        //BOOST_SPIRIT_DEBUG_NODE(mixin);
        //BOOST_SPIRIT_DEBUG_NODE(function);
        //BOOST_SPIRIT_DEBUG_NODE(filter_list);
        //BOOST_SPIRIT_DEBUG_NODE(name);
        //BOOST_SPIRIT_DEBUG_NODE(var_name);
        //BOOST_SPIRIT_DEBUG_NODE(style_name);
    }
};
}
#endif
