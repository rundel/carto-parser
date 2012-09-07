#include <exception.hpp>

#include <sstream>
#include <exception>


#include <boost/spirit/home/support/info.hpp>
#include <boost/spirit/include/qi.hpp>

#include <utility/position_iterator.hpp>

namespace carto {

exception::exception () {
    msg = "(unknown-exception)";
}
    
exception::exception (std::string const& source, source_location loc,
                      std::string const& exception)
{ 
    set(source, loc, exception);
}

void exception::set (std::string const& source, source_location loc,
                     std::string const& exception)
{
    msg = "Error in ";
    
    msg += "\"" + source + "\"";
    
    if (loc.valid())
        msg += " " + loc.get_string();
    
    msg += " " + exception;
}

const char* exception::what () const throw() {
    return msg.c_str();
}

exception::~exception () throw() { }

unexpected_character_data::unexpected_character_data(std::string const& source,
                                                     std::string e)
  : carto::exception(source, source_location(),
                     std::string("(") + e + ")") 
{ }

unexpected_character_data::~unexpected_character_data() throw() { }

std::string expected_component::make(boost::spirit::info const& w, std::string const& e) {
    std::ostringstream oss;
    oss << "(" << e << " ";
    
    carto::info_printer<std::ostringstream> pr(oss);
    
    boost::spirit::basic_info_walker< info_printer<std::ostringstream> > walker(pr, w.tag, 0);
    boost::apply_visitor(walker, w.value);
    
    oss << ")";
    
    return oss.str();
}

expected_component::expected_component( std::string const& source,
                                        source_location loc,
                                        boost::spirit::info const& w,
                                        std::string e)
  : carto::exception(source, loc, make(w, e))
{ }

expected_component::~expected_component () throw() { }

expected::~expected () throw() { }

}

