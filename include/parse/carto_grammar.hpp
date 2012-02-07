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


#include <utility/color.hpp>
#include <parse/string_grammar.hpp>
#include <parse/filter_grammar.hpp>
#include <parse/expression_grammar.hpp>
#include <parse/error_handler.hpp>
#include <parse/annotator.hpp>

namespace carto {

namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

using boost::spirit::utf8_symbol_type;

enum carto_node_type
{
    carto_undefined,
    carto_variable,
    carto_mixin,
    carto_style,
    carto_map_style,
    carto_filter,
    carto_expression,
    carto_attribute,
    carto_color,
    carto_function,
    carto_comment
};

template<typename Iterator>
struct carto_parser : qi::grammar< Iterator, utree::list_type(), ascii::space_type>
{

    qi::rule<Iterator, utree(), ascii::space_type> value, element, filter, //expression, 
                                                   var_val, expr_val,
                                                   comment, attachment;
                                                   
    qi::rule<Iterator, utree::list_type(), ascii::space_type> start, variable, attribute, map_style,
                                                       style_prefix, style, name_list, element_list,
                                                       color, mixin, filter_list, expression;
    
    qi::rule<Iterator, utf8_symbol_type()> name, style_name, var_name, enum_val, ustring;
    qi::rule<Iterator, utree::nil_type()> null;

    mapnik::css_color_grammar<Iterator> css_color;
    phoenix::function<color_conv_impl> css_conv;

    utf8_string_parser<Iterator> utf8;
    filter_parser<Iterator> filter_text;
    expression_parser<Iterator> expression_text;
        
    typedef error_handler_impl<Iterator> error_handler_type;
    phoenix::function<error_handler_type> const error;
    annotator<Iterator> annotate;

    carto_parser (std::string const& source, annotations_type& annotations)
      : carto_parser::base_type(start),
        utf8(source),
        filter_text(source, annotations),
        expression_text(source, annotations),
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
        
        start = +(comment | variable | map_style | style);
        
        comment = as_symbol["/*" >> *(char_ - "*/") > "*/"]
                  > annotate(_val, carto_comment);
        
        element =   comment
                  | variable
                  | attribute
                  | style;
              
        name = char_("a-zA-Z_") > *char_("a-zA-Z0-9_-");
        var_name = lexeme["@" > name];
        
        // Map style grammar
        map_style = lit("Map") > "{" > *(variable | attribute) > "}" > annotate(_val, carto_map_style);
        
        
        // Style grammar
        style_name = -lexeme[char_("#.") >> name >> *(char_(".") >> name)];
        attachment = -("::" >> name);
        filter = lit("[") > filter_text > "]";
        filter_list = *filter > annotate(_val, carto_filter);;
        
        style_prefix = style_name >> attachment >> filter_list;
        
        name_list = style_prefix % ",";
        element_list = "{" > *element > "}";
        
        style = name_list >> element_list > annotate(_val, carto_style);
        
        
        attribute = as_symbol[name] > ":" >> value //(value % ",") > ";"
                    > annotate(_val, carto_attribute);
        
        variable = as_symbol[var_name] > ":" >> value //(value % ",") > ";"
                   > annotate(_val, carto_variable);

        value =   ( null          >> ";" )
                | ( color         >> ";" )
                | ( double_ % "," >> ";" )
                | ( bool_         >> ";" )
                //| ( utf8 % ","    >> ";" )
                | ( var_val       >> ";" )
                | ( ustring % "," >> ";" )
                | ( expr_val      >> ";" )
                | ( enum_val      >> ";" );
        
        ustring =   lexeme['\'' >> *(char_-'\'') > '\'']
                  | lexeme['"'  >> *(char_-'"')  > '"' ];
                
        null = "null" >> qi::attr(spirit::nil); 
        color =   css_color[_val = css_conv(qi::_1)] > annotate(_val, carto_color);
        enum_val = lexeme[+(char_("a-zA-Z_-"))];
        
        var_val = var_name > annotate(_val, carto_variable);
        expr_val = expression > annotate(_val, carto_expression) ;
        
        expression = expression_text;// 
        
        
        //mixin = style_name > -("(" > name > ")") > ";" > annotate(_val, carto_mixin);
        
        qi::on_error<qi::fail>(start, error(qi::_3, qi::_4));
        
        /*
        BOOST_SPIRIT_DEBUG_NODE(element);
        BOOST_SPIRIT_DEBUG_NODE(attribute);
        BOOST_SPIRIT_DEBUG_NODE(value);
        BOOST_SPIRIT_DEBUG_NODE(expression);
        
        BOOST_SPIRIT_DEBUG_NODE(var_ref);
        BOOST_SPIRIT_DEBUG_NODE(ustring);
        BOOST_SPIRIT_DEBUG_NODE(enum_val);
        */
        
        //BOOST_SPIRIT_DEBUG_NODE(start);
        //BOOST_SPIRIT_DEBUG_NODE(variable);
        //BOOST_SPIRIT_DEBUG_NODE(map_style);
        //BOOST_SPIRIT_DEBUG_NODE(style);
        //BOOST_SPIRIT_DEBUG_NODE(comment);
        //BOOST_SPIRIT_DEBUG_NODE(attribute);
        //BOOST_SPIRIT_DEBUG_NODE(style_name_list);
        //BOOST_SPIRIT_DEBUG_NODE(style_element_list);
        //BOOST_SPIRIT_DEBUG_NODE(style_prefix);
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
