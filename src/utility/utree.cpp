#include <utility/utree.hpp>
#include <mapnik/color.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/support_utree.hpp>

namespace carto {

namespace spirit = boost::spirit;
using spirit::utree;

std::string utree_to_string::operator() (spirit::utf8_string_range_type const& str)
{
    typedef spirit::utf8_string_range_type::const_iterator iterator;
    iterator it = str.begin(), end = str.end();
    
    std::string out;
    for (; it != end; ++it)
        out += *it;
    
    return out;
}

std::string utree_to_string::operator() (spirit::utf8_symbol_range_type const& str)
{
    typedef spirit::utf8_symbol_range_type::const_iterator iterator;
    iterator it = str.begin(), end = str.end();

    std::string out;
    for (; it != end; ++it)
        out += *it;
    
    return out;
}
    
std::string utree_to_string::operator() (utree::invalid_type ut){
    return (*this)("[invalid]");
}

std::string utree_to_string::operator() (utree::nil_type ut){
    return (*this)("[null]");
}

std::string utree_to_string::operator() (spirit::binary_range_type const& str){
    return (*this)("[binary]");
}

std::string utree_to_string::operator() (spirit::any_ptr const& p){
    return (*this)("[pointer]");
}

std::string utree_to_string::operator() (spirit::function_base const& pf){
    return (*this)("[function]");
}

template<>
std::string as<std::string>(utree const& ut) 
{    
    return utree::visit(ut, utree_to_string());
}

template<>
mapnik::color as<mapnik::color>(utree const& ut) 
{    
    BOOST_ASSERT(ut.size()==4);
    
    utree::const_iterator it = ut.begin();
    
    int r = as<int>(*it++),
        g = as<int>(*it++),
        b = as<int>(*it++),
        a = as<int>(*it++);
    
    return mapnik::color(r,g,b,a);
}

}

