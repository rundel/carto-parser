//#define BOOST_SPIRIT_DEBUG

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

//#include <generate/generate_filter.hpp>
#include <parse/carto_grammar.hpp>
#include <parse/filter_grammar.hpp>
#include <parse/expression_grammar.hpp>
#include <parse/parse_tree.hpp>
#include <utility/environment.hpp>
#include <expression_eval.hpp>

#include <mapnik/position_iterator.hpp>

#include <iostream>
#include <string>



///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////
int main()
{

    using boost::spirit::ascii::space;
    using boost::spirit::utree;
    
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "Expression parser...\n\n";
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "Type an expression...or [q or Q] to quit\n\n";

    std::string str;
    while (std::getline(std::cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;
        
        typedef mapnik::position_iterator<std::string::const_iterator> iter;
        carto::parse_tree tree = carto::build_parse_tree< carto::expression_parser<iter> >(str);
        
        utree ut = tree.ast();
        
        std::cout << "AST: "<< ut << "\n";
        
        carto::expression exp(tree.ast(), tree.annotations(), carto::style_env());
        std::cout << "Result: " << exp.eval() << "\n\n";
    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}


