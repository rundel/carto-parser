#include <utility/color.hpp>

namespace carto {

using boost::spirit::utree;

utree::list_type color_conv_impl::operator() (mapnik::color color) const
{
    utree::list_type u;
    u.push_back(color.red());
    u.push_back(color.green());
    u.push_back(color.blue());
    u.push_back(color.alpha());
    
    return u;
}

}

