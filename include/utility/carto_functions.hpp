#ifndef CARTO_FUNCTIONS_H
#define CARTO_FUNCTIONS_H

#include <boost/spirit/include/support_utree.hpp>

#include <mapnik/color.hpp>

#include <utility/utree.hpp>
#include <utility/round.hpp>


namespace carto {

inline double clamp(double val);

struct hsl 
{    
    double h,s,l,a;
    unsigned tag;
    
    hsl(utree const& rgb);

    double hue(double h, double m1, double m2);

    utree to_rgb() ;
};

utree test(utree const& rgb);

utree hue(utree const& rgb);

utree saturation(utree const& rgb);

utree lightness(utree const& rgb);

utree alpha(utree const& rgb);

utree saturate(utree const& rgb, utree const& value) ;

utree desaturate(utree const& rgb, utree const& value);

utree lighten(utree const& rgb, utree const& value);

utree darken(utree const& rgb, utree const& value);

utree fadein(utree const& rgb, utree const& value);

utree fadeout(utree const& rgb, utree const& value);

utree spin(utree const& rgb, utree const& value);

//
// Copyright (c) 2006-2009 Hampton Catlin, Nathan Weizenbaum, and Chris Eppstein
// http://sass-lang.com
//
utree mix(utree const& col1, utree const& col2, utree const& weight);


utree greyscale(utree const& rgb);

}

#endif
