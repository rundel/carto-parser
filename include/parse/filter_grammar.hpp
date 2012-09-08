#ifndef FILTER_GRAMMAR_H
#define FILTER_GRAMMAR_H

#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <parse/expression_grammar.hpp>
#include <parse/error_handler.hpp>
#include <parse/annotator.hpp>

#include <utility/position_iterator.hpp>

namespace carto {

namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

using boost::spirit::utree;
using boost::spirit::utf8_symbol_type;

enum filter_node_type
{
    FILTER_AND,
    FILTER_OR,
    FILTER_NOT,
    FILTER_EQ,
    FILTER_NEQ,
    FILTER_LE,
    FILTER_LT,
    FILTER_GE,
    FILTER_GT,
    FILTER_MATCH,
    FILTER_REPLACE,
    FILTER_ATTRIBUTE,
    FILTER_EXPRESSION,
    FILTER_VAR,
    FILTER_VAR_ATTR
};


template<typename Iterator>
struct filter_parser : qi::grammar< Iterator, utree(), ascii::space_type>
{
    
    qi::rule<Iterator, utree(), ascii::space_type> logical_expr, not_expr, cond_expr, 
                                            equality_expr, lhs_expr, rhs_expr, 
                                            regex_match_expr, regex_replace_expr, 
                                            null, var_attr, var,
                                            attr, sq_attr; 
    
    qi::rule<Iterator, utf8_symbol_type()> name, var_name, ustring;
    
    //expression_parser<Iterator> expression;
    typedef error_handler_impl<Iterator> error_handler_type;
    phoenix::function<error_handler_type> const error;
    annotator<Iterator> annotate;
    phoenix::function<combine_impl> const combine;
    
    filter_parser (std::string const& source, annotations_type& annotations)
      : filter_parser::base_type(logical_expr),
        //expression(source,annotations),
        error(error_handler_type(source)),
        annotate(annotations)
    {
        using qi::double_;
        using qi::bool_;
        using qi::_val;
        using qi::_1;
        using qi::lexeme;
        using qi::char_;
        using qi::lit;
        
        using boost::spirit::utf8_symbol_type;
        
        logical_expr = not_expr [_val = _1] 
            >> *(   ( (lit("and") | lit("&&")) >> not_expr[combine(_val, _1)] > annotate(_val, FILTER_AND) )
                  | ( (lit("or")  | lit("||")) >> not_expr[combine(_val, _1)] > annotate(_val, FILTER_OR)  )
                );

        not_expr =   ( (lit("not") | lit('!')) >> cond_expr[_val = _1] > annotate(_val, FILTER_NOT) )
                   | cond_expr [_val = _1 ];

        cond_expr =   equality_expr[_val = _1] 
                    | rhs_expr[_val = _1]
                    | ('(' > logical_expr[_val = _1] > ')');

        equality_expr = lhs_expr[_val = _1]
            >> *(   ( (lit("==") | "eq" | "is" | "=") > rhs_expr[combine(_val, _1)] > annotate(_val, FILTER_EQ) )
                  | ( (lit("!=") | "<>" | "neq")      > rhs_expr[combine(_val, _1)] > annotate(_val, FILTER_NEQ) )
                  | ( (lit("<=") | "le")              > rhs_expr[combine(_val, _1)] > annotate(_val, FILTER_LE) )
                  | ( (lit('<')  | "lt")              > rhs_expr[combine(_val, _1)] > annotate(_val, FILTER_LT) )
                  | ( (lit(">=") | "ge")              > rhs_expr[combine(_val, _1)] > annotate(_val, FILTER_GE) )
                  | ( (lit('>')  | "gt")              > rhs_expr[combine(_val, _1)] > annotate(_val, FILTER_GT) )
                  | (                           regex_match_expr[combine(_val, _1)] > annotate(_val, FILTER_MATCH) )
                  | (                         regex_replace_expr[combine(_val, _1)] > annotate(_val, FILTER_REPLACE) )
                );

        regex_match_expr = lit(".match") > '(' > ustring[_val = _1] > ')' > annotate(_val, FILTER_MATCH);

        regex_replace_expr = lit(".replace") > '(' > ustring[_val = _1] > ',' 
                                                   > ustring[combine(_val, _1)] > ')'
                                                   > annotate(_val, FILTER_REPLACE);
        
        
        name = char_("a-zA-Z_") >> *char_("a-zA-Z0-9_");
        ustring = lexeme[char_("'") > *(char_-'\'') > char_("'")];
        
        null = lit("null")[_val = utree::nil_type()];

        attr = lexeme[name] > annotate(_val, FILTER_ATTRIBUTE);
        sq_attr = '[' >> attr > ']';
        
        var_name = lexeme["@" > name];
        var_attr = var_name > annotate(_val, FILTER_VAR_ATTR);
        var      = var_name > annotate(_val, FILTER_VAR);
        
        lhs_expr =   attr
                   | sq_attr
                   | var_attr;
        
        rhs_expr =   double_
                   | bool_
                   | null
                   | ustring
                   | var;
                   //| (expression[_val = _1] > annotate(_val, filter_expression))
                   //| lhs_expr[_val = _1];

        qi::on_error<qi::fail>(logical_expr, error(qi::_3, qi::_4));

        //BOOST_SPIRIT_DEBUG_NODE(logical_expr);
        //BOOST_SPIRIT_DEBUG_NODE(not_expr);
        //BOOST_SPIRIT_DEBUG_NODE(cond_expr);
        //BOOST_SPIRIT_DEBUG_NODE(equality_expr);
        //BOOST_SPIRIT_DEBUG_NODE(lhs_expr);
        //BOOST_SPIRIT_DEBUG_NODE(rhs_expr);
        //BOOST_SPIRIT_DEBUG_NODE(regex_match_expr);
        //BOOST_SPIRIT_DEBUG_NODE(regex_replace_expr);
        //BOOST_SPIRIT_DEBUG_NODE(ustring);
        //BOOST_SPIRIT_DEBUG_NODE(attr);
        //BOOST_SPIRIT_DEBUG_NODE(sq_attr);
        //BOOST_SPIRIT_DEBUG_NODE(var_name);
    }
    
};


}

#endif
