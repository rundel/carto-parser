#ifndef EXPRESSION_GRAMMAR_H
#define EXPRESSION_GRAMMAR_H

#include <boost/spirit/include/phoenix.hpp>

#include <parse/string_grammar.hpp>
#include <parse/error_handler.hpp>
#include <parse/annotator.hpp>

namespace carto {

namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

using ascii::space_type;

enum expression_node_type
{
    exp_plus,
    exp_minus,
    exp_times,
    exp_divide,
    exp_neg,
    exp_function
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
struct expression_parser : qi::grammar< Iterator, utree(), space_type>
{
    qi::rule<Iterator, utree(), space_type> expression, term, factor, var_name; 
    
    typedef error_handler_impl<Iterator> error_handler_type;
    annotator<Iterator> annotate;
    
    phoenix::function<error_handler_type> const error;
    phoenix::function<combine_impl> const combine;
    
    expression_parser (std::string const& source, annotations_type& annotations)
      : expression_parser::base_type(expression),
        error(error_handler_type(source)), 
        annotate(annotations)
    {
        using qi::double_;
        using qi::_val;
        using qi::_1;
        using qi::lexeme;
        using qi::char_;
        
        qi::alpha_type alpha;      
        std::string exclude = std::string(" {}[]:\"\x01-\x1f\x7f") + '\0';
        
        var_name = lexeme["@" > alpha >> *(~char_(exclude))];
        
        expression = term[_val=_1]
            >> *(   ('+' >> term[combine(_val, _1)] > annotate(_val, exp_plus))
                  | ('-' >> term[combine(_val, _1)] > annotate(_val, exp_minus))
                );
        
        term = factor[_val=_1] 
            >> *(   ('*' > factor[combine(_val, _1)] > annotate(_val, exp_times))
                  | ('/' > factor[combine(_val, _1)] > annotate(_val, exp_divide))
                );  
        
        factor =   double_[_val=_1]
                 | var_name[_val=_1]
                 | ( '(' > expression[_val=_1] > ')' )
                 | ( '-' > factor[_val=_1] > annotate(_val, exp_neg) );
        
        BOOST_SPIRIT_DEBUG_NODE(expression);
        BOOST_SPIRIT_DEBUG_NODE(term);
        BOOST_SPIRIT_DEBUG_NODE(factor);
        
        qi::on_error<qi::fail>(expression, error(qi::_3, qi::_4));
    }
};


}

#endif
