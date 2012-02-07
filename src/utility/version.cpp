#ifndef VERSION_H
#define VERSION_H

#include <iostream>

#include <utility/version.hpp>
#include <mapnik/version.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>


bool version_from_string(std::string min_version_string)
{
    boost::char_separator<char> sep(".");
    boost::tokenizer<boost::char_separator<char> > tokens(min_version_string,sep);
    unsigned i = 0;
    bool success = false;
    int n[3];
    for (boost::tokenizer<boost::char_separator<char> >::iterator beg=tokens.begin(); 
         beg!=tokens.end();
         ++beg)
    {
        try 
        {
            n[i] = boost::lexical_cast<int>(boost::trim_copy(*beg));
        }
        catch (boost::bad_lexical_cast & ex)
        {
            std::clog << *beg << " : " << ex.what() << "\n";
            break;
        }
        if (i==2) 
        {
            success = true;
            break;
        }
        ++i;
    }
    return (success) ? (n[0] * 100000) + (n[1] * 100) + (n[2]) : -1;

}

#endif
    
