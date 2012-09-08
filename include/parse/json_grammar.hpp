/*==============================================================================
    Copyright (c) 2010 Object Modeling Designs
    Copyright (c) 2010 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef JSON_GRAMMAR_H
#define JSON_GRAMMAR_H

#include <limits>

#include <boost/spirit/include/phoenix.hpp>

#include <parse/error_handler.hpp>
#include <parse/annotator.hpp>

namespace carto {

namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

using boost::spirit::utf8_symbol_type;

enum node_type
{
    JSON_VALUE,
    JSON_PAIR,
    JSON_OBJECT,
    JSON_ARRAY
};

template<typename Iterator>
struct json_parser : qi::grammar< Iterator, utree(), ascii::space_type>
{
    qi::rule<Iterator, utree(), ascii::space_type> start, value;
    qi::rule<Iterator, utree::list_type(), ascii::space_type> member_pair, object, array;
    qi::rule<Iterator, utf8_symbol_type()> member, ustring;
    qi::rule<Iterator, utf8_symbol_type(), ascii::space_type> empty_object, empty_array;
    qi::rule<Iterator, utree::nil_type()> null;

    typedef error_handler_impl<Iterator> error_handler_type;
    phoenix::function<error_handler_type> const error;

    annotator<Iterator> annotate;

    json_parser (std::string const& source, annotations_type& annotations)
      : json_parser::base_type(start),
        error(error_handler_type(source)), 
        annotate(annotations)
    {
        using qi::char_;
        using qi::lexeme;
        using qi::on_error;
        using qi::fail;
        using qi::int_;
        using qi::bool_;
        using qi::lit;
        using qi::_val;

        qi::real_parser<double, qi::strict_real_policies<double> > real;
        
        qi::as<utf8_symbol_type> as_symbol;

        start = value.alias();

        value =   null
                | real
                | int_
                | bool_
                | ustring
                | object
                | array
                | empty_object
                | empty_array
                > annotate(_val, JSON_VALUE);
        
        null = "null" >> qi::attr(spirit::nil); 

        object %= '{' >> (member_pair % ',') > '}'
                > annotate(_val, JSON_OBJECT);

        member_pair %= '"' > as_symbol[member] > '"' > ':' > value
                     > annotate(_val, JSON_PAIR);//node_type::pair);
    
        array %= '[' >> (value % ',') > ']'
               > annotate(_val, JSON_ARRAY);

        std::string exclude = std::string(" {}[]:\"\x01-\x1f\x7f") + '\0';
        member = lexeme[+(~char_(exclude))];

        empty_object = char_('{') > char_('}');
        empty_array  = char_('[') > char_(']');

        ustring = lexeme['"' >> *(char_-'"')  > '"' ];

        std::string name = "mml";
 
        start.name(name);
        value.name(name + ":value");
        null.name(name + ":null");
        object.name(name + ":object");
        member_pair.name(name + ":member-pair");
        array.name(name + ":array");
        member.name(name + ":member");
        empty_object.name(name + ":empty-object");
        empty_array.name(name + ":empty-array");
 
        on_error<fail>(start, error(qi::_3, qi::_4));
        
        //BOOST_SPIRIT_DEBUG_NODE( start );
        //BOOST_SPIRIT_DEBUG_NODE( value );
        //BOOST_SPIRIT_DEBUG_NODE( object );
        //BOOST_SPIRIT_DEBUG_NODE( member_pair );   
    }
};


}

#endif
