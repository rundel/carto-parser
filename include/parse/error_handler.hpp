/*==============================================================================
    Copyright (c) 2001-2010 Joel de Guzman
    Copyright (c) 2010      Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <exception.hpp>
#include <mapnik/position_iterator.hpp>

namespace carto {

namespace spirit = boost::spirit;

template<class Iterator>
struct error_handler_impl {
    
    template<class,class>
    struct result {
        typedef void type;
    };

    std::string const& source;

    error_handler_impl(std::string const& source_)
      : source(source_) { }

    void operator()(Iterator err_pos, spirit::info const& what) const {
        throw expected_component(source, mapnik::get_location(err_pos), what);
    }
};

}

#endif
