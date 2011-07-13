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
    carto_filter,
    carto_function,
    carto_attribute,
    carto_color
};


struct css_conv_impl
{
    template <typename T>
    struct result
    {
        typedef utree::list_type type;
    };
    
    utree::list_type operator() (mapnik::css color) const
    {
        utree::list_type u;
        u.push_back(color.r);
        u.push_back(color.g);
        u.push_back(color.b);
        u.push_back(color.a);
        
        return u;
    }
};


template<typename Iterator>
struct carto_parser : qi::grammar< Iterator, utree::list_type(), space_type>
{

    qi::rule<Iterator, utree(), space_type> value, element, filter;
    qi::rule<Iterator, utree::list_type(), space_type> start, variable, attribute, style, color, mixin, function;
    qi::rule<Iterator, utf8_symbol_type()> name, var_name, style_name, filter_text, enum_val;
    qi::rule<Iterator, utree::nil_type()> null;

    mapnik::css_color_grammar<Iterator> css_color;

    utf8_string_parser<Iterator> utf8;
    
    phoenix::function<css_conv_impl> css_conv;

    typedef error_handler_impl<Iterator> error_handler_type;
    phoenix::function<error_handler_type> const error;
    annotator<Iterator> annotate;

    carto_parser (std::string const& source, annotations_type& annotations)
      : carto_parser::base_type(start),
        utf8(source), 
        error(error_handler_type(source)), 
        annotate(annotations)
    {
        using qi::char_;
        using qi::lexeme;
        using qi::int_;
        using qi::bool_;
        using qi::lit;
        using qi::_val;
        using qi::lit;

        qi::as<utf8_symbol_type> as_symbol;
        
        qi::alpha_type alpha;
        //qi::alnum_type alnum;

        start = +(variable | style);
        
        element =   variable
                  | attribute
                  | style;
              
        std::string exclude = std::string(" {}[]:\"\x01-\x1f\x7f") + '\0';
        name = alpha >> *(~char_(exclude));
        
        var_name = lexeme["@" > name];
        
        variable %= as_symbol[var_name] > ":" > (value % ",") > ";"
                    > annotate(_val, carto_variable);
        
        attribute %= as_symbol[name] > ":" > (value % ",") > ";"
                     > annotate(_val, carto_attribute);
        
        style_name = lexeme[-char_("#.") >> name];
        
        filter_text = lexeme[*(char_-"]")];
        filter %= lit("[") >> filter_text > "]"
                  > annotate(_val, carto_filter);
        
        style %= as_symbol[-style_name] >> -filter >> "{" >> (*element) >> "}"
                 > annotate(_val, carto_style);
        
        enum_val = lexeme[+(alpha|"_")];
        
        qi::real_parser<double, qi::strict_real_policies<double> > real;
        value =   null
                | color
                | real
                | int_
                | bool_
                | utf8
                | enum_val
                | function
                | mixin
                | var_name;
                
        mixin %=   style_name > -("(" > name > ")") > ";"
                 > annotate(_val, carto_mixin);
        
        function %=   name > "(" > (value % ",") > ")"
                    > annotate(_val, carto_function);
        
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
        
        //BOOST_SPIRIT_DEBUG_NODE( start );
        //BOOST_SPIRIT_DEBUG_NODE( variable );
        //BOOST_SPIRIT_DEBUG_NODE( style );
        //BOOST_SPIRIT_DEBUG_NODE( style_name );
        //BOOST_SPIRIT_DEBUG_NODE( filter );
        //BOOST_SPIRIT_DEBUG_NODE( element );
        //BOOST_SPIRIT_DEBUG_NODE( attribute );
        //BOOST_SPIRIT_DEBUG_NODE( value );
        //BOOST_SPIRIT_DEBUG_NODE( name );
        
    }
};


}

#endif
