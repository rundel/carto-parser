#ifndef EXPRESSION_GRAMMAR_H
#define EXPRESSION_GRAMMAR_H

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_utree.hpp>

#include <mapnik/css_color_grammar.hpp>

#include <utility/color.hpp>
#include <parse/string_grammar.hpp>
#include <parse/error_handler.hpp>
#include <parse/annotator.hpp>

namespace carto {

namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;


enum expression_node_type
{
    exp_plus,
    exp_minus,
    exp_times,
    exp_divide,
    exp_neg,
    exp_function,
    exp_color,
    exp_var,
    exp_percentage
};

struct combine_impl
{
    template <typename T1, typename T2 = void>
    struct result { typedef void type; };

    void operator()(spirit::utree& expr, spirit::utree const& rhs) const
    {
        spirit::utree lhs;
        lhs.swap(expr);
        expr.push_back(lhs);
        expr.push_back(rhs);
    }
};

template<typename Iterator>
struct expression_parser : qi::grammar< Iterator, utree(), ascii::space_type>
{
    
    qi::rule<Iterator, utree(), ascii::space_type> expression, term, factor; 
    qi::rule<Iterator, utree::list_type(), ascii::space_type> function, color, start; 
    qi::rule<Iterator, boost::spirit::utf8_symbol_type()> name, var_name, function_name;
    
    typedef error_handler_impl<Iterator> error_handler_type;
    annotator<Iterator> annotate;
    
    mapnik::css_color_grammar<Iterator> css_color;
    phoenix::function<color_conv_impl> css_conv;
    
    phoenix::function<error_handler_type> const error;
    phoenix::function<combine_impl> const combine;
    
    expression_parser (std::string const& source, annotations_type& annotations)
      : expression_parser::base_type(expression),
        annotate(annotations),
        error(error_handler_type(source))
    {
        using qi::double_;
        using qi::_val;
        using qi::_1;
        using qi::lexeme;
        using qi::char_;
        
        
        name = char_("a-zA-Z_") > *char_("a-zA-Z0-9_-");
        var_name = lexeme["@" > name];
        
        expression = term[_val=_1]
            >> *(   ('+' > term[combine(_val, _1)] > annotate(_val, exp_plus))
                  | ('-' > term[combine(_val, _1)] > annotate(_val, exp_minus))
                );
        
        term = factor[_val=_1] 
            >> *(   ('*' > factor[combine(_val, _1)] > annotate(_val, exp_times))
                  | ('/' > factor[combine(_val, _1)] > annotate(_val, exp_divide))
                );  
        
        function = name >> "(" > expression % "," > ")";
        
        factor = ( double_[_val = _1] >> "%" > annotate(_val, exp_percentage) )
               | ( double_[_val = _1] )
               | ( css_color[_val = css_conv(_1)] > annotate(_val, exp_color) )
               | ( var_name[_val = _1] > annotate(_val, exp_var) )
               | ( function[_val = _1] > annotate(_val, exp_function) )
               | ( "(" > expression[_val = _1] > ")" )
               | ( "-" > factor[_val = _1] > annotate(_val, exp_neg) )
               ;
        
        BOOST_SPIRIT_DEBUG_NODE(expression);
        BOOST_SPIRIT_DEBUG_NODE(term);
        BOOST_SPIRIT_DEBUG_NODE(factor);
        BOOST_SPIRIT_DEBUG_NODE(function);
        BOOST_SPIRIT_DEBUG_NODE(var_name);
        BOOST_SPIRIT_DEBUG_NODE(function);
        
        qi::on_error<qi::fail>(expression, error(qi::_3, qi::_4));
    }
};


}

#endif
