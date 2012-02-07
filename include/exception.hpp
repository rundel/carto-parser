/*==============================================================================
    Copyright (c) 2011 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <sstream>
#include <exception>


#include <boost/spirit/home/support/info.hpp>
#include <boost/spirit/include/qi.hpp>

#include <position_iterator.hpp>

namespace carto {

///////////////////////////////////////////////////////////////////////////////
struct exception : std::exception
{
    std::string msg;
    
    exception ();
    
    exception (std::string const& source, source_location loc,
               std::string const& exception);
    
    void set (std::string const& source, source_location loc,
              std::string const& exception);
    
    const char* what () const throw();
    
    ~exception () throw();
};

///////////////////////////////////////////////////////////////////////////////
struct unexpected_character_data : carto::exception {
    unexpected_character_data(std::string const& source,
                              std::string e = "unexpected-character-data");
   
    ~unexpected_character_data() throw();
};

///////////////////////////////////////////////////////////////////////////////
template<class Out>
struct info_printer {
    typedef boost::spirit::utf8_string utf8;
    
    info_printer(Out& out) : out(out) {}
    
    void element(utf8 const& tag, utf8 const& value, int) const {
        if (value == "")
            out << tag;
        else
            out << "\"" << value << '"';
    }
    
    Out& out;
};

struct expected_component : carto::exception {
    static std::string make(boost::spirit::info const& w, std::string const& e);

    expected_component( std::string const& source,
                        source_location loc,
                        boost::spirit::info const& w,
                        std::string e = "expected-component");
  
    ~expected_component () throw();
};

///////////////////////////////////////////////////////////////////////////////
struct expected : carto::exception {
    template<class Got>
    static std::string make(Got const& got, 
                            std::string const& e)
    {
        std::ostringstream oss;
        oss << "(" << e << " " << got << ")"; 
        return oss.str();
    }
    
    template<class Expect, class Got>
    static std::string make(Expect const& expect, 
                            Got const& got,
                            std::string const& e)
    {
      std::ostringstream oss;
      oss << "(" << e << " " << expect << " " << got << ")"; 
      return oss.str();
    }
    
    template<class Got>
    expected (std::string e)
      : carto::exception("", source_location(), std::string("(") + e + ")") 
    { }
    
    template<class Got>
    expected (Got const& got, std::string e)
      : carto::exception("", source_location(), make(got, e))
    { }
    
    template<class Expect, class Got>
    expected (Expect const& expect, Got const& got, std::string e)
      : carto::exception("", source_location(), make(expect, got, e)) 
    { }
    
    virtual ~expected () throw();
};

}

#endif
