
#include <boost/spirit/include/support_utree.hpp>

#include <mapnik/color.hpp>

#include <utility/utree.hpp>
#include <utility/round.hpp>


namespace carto {

inline double clamp(double val) {
    return std::min(1.0, std::max(0.0, val));
}

struct hsl {
    
    double h,s,l,a;
    
    hsl(utree const& rgb)
    {
        BOOST_ASSERT(rgb.size() == 3 || rgb.size() == 4);

        typedef utree::const_iterator iter;
        iter it  = rgb.begin(),
             end = rgb.end();

        double r = as<double>(*it) / 255; it++;
        double g = as<double>(*it) / 255; it++;
        double b = as<double>(*it) / 255; it++;
        
        double max = std::max(r,std::max(g,b)),
               min = std::min(r,std::min(g,b));

        h = (max + min) / 2;
        s = (max + min) / 2;
        l = (max + min) / 2;
        a = (it != end) ? as<double>(*it) : 255;

        double d = max - min;

        if (max == min) {
            h = 0;
            s = 0;
        } else {
            s = (l > 0.5) ? d / (2 - max - min) : d / (max + min);

            if (max == r) {
                h = (g - b) / d + (g < b ? 6 : 0);
            } else if (max == g) {
                h = (b - r) / d + 2;
            } else if (max == b) {
                h = (r - g) / d + 4;
            }
            h /= 6;
        }
    }
    
    inline double hue(double h, double m1, double m2) 
    {
        double tmp = h < 0 ? h + 1 : (h > 1 ? h - 1 : h);
        
        if      (tmp * 6 < 1) return m1 + (m2 - m1) * tmp * 6;
        else if (tmp * 2 < 1) return m2;
        else if (tmp * 3 < 2) return m1 + (m2 - m1) * (2/3 - tmp) * 6;
        else                  return m1;
    }
    
    utree to_rgb() 
    {
        double m2 = (l <= 0.5) ? l * (s + 1) : l + s - l * s;
        double m1 = l * 2 - m2;
        
        utree ut;
        ut.push_back( round(hue(h + 1/3,m1,m2) * 255) );
        ut.push_back( round(hue(h      ,m1,m2) * 255) );
        ut.push_back( round(hue(h - 1/3,m1,m1) * 255) );
        ut.push_back( round(a) );

        return ut;
    }
    
    template<class T>
    T as(utree const& ut)
    {
        return detail::as<T>(ut);
    }
};




utree hue(utree const& rgb)
{
    return utree( hsl(rgb).h );
}

utree saturation(utree const& rgb)
{
    return utree( hsl(rgb).s );
}

utree lightness(utree const& rgb)
{
    return utree( hsl(rgb).l );
}

utree alpha(utree const& rgb)
{
    BOOST_ASSERT(rgb.size() == 3 || rgb.size() == 4);
    utree::const_iterator it = ++(++(++rgb.begin()));
    
    return *it;
}


utree saturate(utree const& rgb, utree const& value) 
{
    hsl color(rgb);
    
    color.s += detail::as<double>(value) / 100;
    color.s = clamp(color.s);
    
    return color.to_rgb();
}

utree desaturate(utree const& rgb, utree const& value)
{
    hsl color(rgb);

    color.s -= detail::as<double>(value) / 100;
    color.s = clamp(color.s);
    
    return color.to_rgb();
}

utree lighten(utree const& rgb, utree const& value)
{
    hsl color(rgb);

    color.l += detail::as<double>(value) / 100;
    color.l = clamp(color.l);
    
    return color.to_rgb();
}

utree darken(utree const& rgb, utree const& value) 
{
    hsl color(rgb);

    color.l -= detail::as<double>(value) / 100;
    color.l = clamp(color.l);
    
    return color.to_rgb();
}

utree fadein(utree const& rgb, utree const& value) 
{
    hsl color(rgb);

    color.a += detail::as<double>(value) / 100;
    color.a = clamp(color.a);
    
    return color.to_rgb();
}

utree fadeout(utree const& rgb, utree const& value) {
    hsl color(rgb);

    color.a -= detail::as<double>(value) / 100;
    color.a = clamp(color.a);
    
    return color.to_rgb();
}

utree spin(utree const& rgb, utree const& value) {
    hsl color(rgb);
    double hue = fmod(color.h + detail::as<double>(value), 360);

    color.h = hue < 0 ? 360 + hue : hue;
    
    return color.to_rgb();
}


//
// Copyright (c) 2006-2009 Hampton Catlin, Nathan Weizenbaum, and Chris Eppstein
// http://sass-lang.com
//
utree mix(utree const& col1, utree const& col2, utree const& weight) 
{
    mapnik::color rgb1 = detail::as<mapnik::color>(col1);
    mapnik::color rgb2 = detail::as<mapnik::color>(col2);
    
    double p = detail::as<double>(weight) / 100.0;
    double w = p * 2 - 1;
    double a = rgb1.alpha() - rgb2.alpha();

    double w1 = (((w * a == -1) ? w : (w + a) / (1 + w * a)) + 1) / 2.0;
    double w2 = 1 - w1;
    
    utree ut;
    ut.push_back(rgb1.red()   * w1 + rgb2.red()   * w2     );
    ut.push_back(rgb1.green() * w1 + rgb2.green() * w2     );
    ut.push_back(rgb1.blue()  * w1 + rgb2.blue()  * w2     );
    ut.push_back(rgb1.alpha() * p  + rgb2.alpha() * (1 - p));
    
    return ut;
}


utree greyscale(utree const& rgb) {
    return desaturate(rgb, 100);
}

}