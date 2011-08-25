#ifndef COLOR_H
#define COLOR_H

#include <boost/spirit/include/support_utree.hpp>

namespace carto {

using boost::spirit::utree;

struct color_conv_impl
{
    template <typename T>
    struct result
    {
        typedef utree::list_type type;
    };
    
    utree::list_type operator() (mapnik::color color) const
    {
        utree::list_type u;
        u.push_back(color.red());
        u.push_back(color.green());
        u.push_back(color.blue());
        u.push_back(color.alpha());
        
        return u;
    }
};

}

#endif