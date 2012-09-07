#include <sstream>
#include <iostream>
#include <utility/carto_error.hpp>

namespace carto {

carto_error::carto_error(std::string const& what)
  : what_(what), loc_(), file_(), msg_()
{ }

carto_error::carto_error(std::string const& what, source_location const& loc)
  : what_(what), loc_(loc), file_(), msg_()
{ }

carto_error::carto_error(std::string const& what, source_location const& loc, std::string const& filename)
  : what_(what), loc_(loc), file_(filename), msg_()
{ }

char const* carto_error::what() const throw()
{
    std::stringstream s;
    s << what_;
    s << " at " << loc_.get_string();
    if (!file_.empty()) s << " in '" << file_ << "'";
    
    msg_ = s.str(); //Avoid returning pointer to dead object
    return msg_.c_str();
}

void carto_error::append_context(std::string const& ctx)
{
    what_ += " " + ctx;
}

void carto_error::set_location(source_location const& loc)
{
    loc_ = loc;
}

void carto_error::set_filename(std::string const& filename)
{
    file_ = filename;
}

void warn(carto_error const& err) 
{
    std::clog << "### WARNING: " << err.what() << std::endl;
}

}
