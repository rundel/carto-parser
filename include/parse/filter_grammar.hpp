#ifndef FILTER_GRAMMAR_H
#define FILTER_GRAMMAR_H

#include <boost/spirit/include/phoenix.hpp>

#include <parse/string_grammar.hpp>
#include <parse/expression_grammar.hpp>
#include <parse/error_handler.hpp>
#include <parse/annotator.hpp>


namespace carto {

namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

using ascii::space_type;

enum filter_node_type
{
    filter_and,
    filter_or,
    filter_not,
    filter_eq,
    filter_neq,
    filter_le,
    filter_lt,
    filter_ge,
    filter_gt,
    filter_match,
    filter_replace,
    filter_attribute,
    filter_expression,
    filter_var
};


template<typename Iterator>
struct filter_parser : qi::grammar< Iterator, utree(), space_type>
{
    qi::rule<Iterator, utree(), space_type> logical_expr, not_expr, cond_expr, 
                                            equality_expr, lhs_expr, rhs_expr, 
                                            regex_match_expr, regex_replace_expr, 
                                            ustring, null, var_name; 
    
    utf8_string_parser<Iterator> utf8;
    //expression_parser<Iterator> expression;
    typedef error_handler_impl<Iterator> error_handler_type;
    phoenix::function<error_handler_type> const error;
    annotator<Iterator> annotate;
    phoenix::function<combine_impl> const combine;
    
    
    filter_parser (std::string const& source, annotations_type& annotations)
      : filter_parser::base_type(logical_expr),
        utf8(source),
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
        
        logical_expr = not_expr [_val = _1] 
            >> *(   ( (lit("and") | lit("&&")) >> not_expr[combine(_val, _1)] > annotate(_val, filter_and) )
                  | ( (lit("or")  | lit("||")) >> not_expr[combine(_val, _1)] > annotate(_val, filter_or)  )
                );

        not_expr =   ( (lit("not") | lit('!')) >> cond_expr[_val = _1] > annotate(_val, filter_or) )
                   | cond_expr [_val = _1 ];
                   
        cond_expr =   equality_expr[_val = _1] 
                    | rhs_expr[_val = _1]
                    | '(' > logical_expr[_val = _1] > ')';
                    
        equality_expr = lhs_expr[_val = _1]
            >> *(   ( (lit("==") | "eq" | "is" | "=") > rhs_expr[combine(_val, _1)] > annotate(_val, filter_eq) )
                  | ( (lit("!=") | "<>" | "neq")      > rhs_expr[combine(_val, _1)] > annotate(_val, filter_neq) )
                  | ( (lit("<=") | "le")              > rhs_expr[combine(_val, _1)] > annotate(_val, filter_le) )
                  | ( (lit('<')  | "lt")              > rhs_expr[combine(_val, _1)] > annotate(_val, filter_lt) )
                  | ( (lit(">=") | "ge")              > rhs_expr[combine(_val, _1)] > annotate(_val, filter_ge) )
                  | ( (lit('>')  | "gt")              > rhs_expr[combine(_val, _1)] > annotate(_val, filter_gt) )
                  | (                           regex_match_expr[combine(_val, _1)] > annotate(_val, filter_match) )
                  | (                         regex_replace_expr[combine(_val, _1)] > annotate(_val, filter_replace) )
                );
        
        regex_match_expr = lit(".match") > '(' > ustring[_val = _1] > ')' > annotate(_val, filter_match);
        
        regex_replace_expr = lit(".replace") > '(' > ustring[_val = _1] > ',' 
                                                   > ustring[combine(_val, _1)] > ')'
                                                   > annotate(_val, filter_replace);
        
        qi::alpha_type alpha;
        
        std::string exclude = std::string(" {}[]:\"\x01-\x1f\x7f") + '\0';
        
        var_name = lexeme["@" > alpha >> *(~char_(exclude))] > annotate(_val, filter_var);
        
        ustring = '\'' >> lexeme[*(char_-'\'')] >> '\'';
        
        null = lit("null")[_val = utree::nil_type()];
        
        rhs_expr =   double_
                   | bool_
                   | null
                   | ustring
                   | var_name;
                   //| (expression[_val = _1] > annotate(_val, filter_expression))
                   //| lhs_expr[_val = _1];
        
        lhs_expr = (   '[' >> +(char_ - ']') >> ']'
                     | +(~char_(".<>=!()|&"+exclude))
                     | var_name
                   ) > annotate(_val, filter_attribute);
        
        
        //BOOST_SPIRIT_DEBUG_NODE(logical_expr);
        //BOOST_SPIRIT_DEBUG_NODE(not_expr);
        //BOOST_SPIRIT_DEBUG_NODE(cond_expr);
        //BOOST_SPIRIT_DEBUG_NODE(equality_expr);
        //BOOST_SPIRIT_DEBUG_NODE(lhs_expr);
        //BOOST_SPIRIT_DEBUG_NODE(rhs_expr);
        //BOOST_SPIRIT_DEBUG_NODE(regex_match_expr);
        //BOOST_SPIRIT_DEBUG_NODE(regex_replace_expr);
        //BOOST_SPIRIT_DEBUG_NODE(ustring);
        
        
        qi::on_error<qi::fail>(logical_expr, error(qi::_3, qi::_4));
    }
};


}

#endif
