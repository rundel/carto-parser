/*==============================================================================
    Copyright (c) 2010 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef GENERATE_JSON_H
#define GENERATE_JSON_H

#include <iosfwd>
#include <sstream>

#include <config.hpp>
#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>

namespace carto {

template<class Out>
struct json_printer {
    typedef void result_type;

    Out& out;
    annotations_type annotations;

    json_printer (Out& out_, annotations_type const& annotations_)
      : out(out_), 
        annotations(annotations_) { }

    void print_object (utree const& ut) const {
        typedef utree::const_iterator iterator;

        iterator it = ut.front().begin(), end = ut.front().end();

        (*this)('{');

        (*this)(*it); ++it;

        for (; it != end; ++it) {
          out << ", ";
          (*this)(*it);
        }

        (*this)('}');
    }

    void print_member_pair (utree const& ut) const {
        utree::const_iterator it = ut.begin();
        print_key(*it); ++it;
        out << ':';
        (*this)(*it);
    }

    void print_key (utree const& ut) const {
        typedef spirit::utf8_symbol_range_type::const_iterator iterator;

        spirit::utf8_symbol_range_type range
          = ut.get<spirit::utf8_symbol_range_type>();

        iterator it = range.begin(), end = range.end();

        out << '"';

        for (; it != end; ++it)
          out << *it;

        out << '"';
    } 

    void print_array (utree const& ut) const {
        typedef utree::const_iterator iterator;

        iterator it = ut.begin(), end = ut.end();

        (*this)('[');

        (*this)(*it); ++it;

        for (; it != end; ++it) {
            out << ", ";
            (*this)(*it);
        }

        (*this)(']');
    }

    void operator() (utree::invalid_type) const {
        out << "\"invalid\"";
    }

    void operator() (utree::nil_type) const {
        out << "null";
    }

    template<class T>
    void operator() (T val) const {
        out << val;
    }

    void operator() (bool b) const {
        out << (b ? "true" : "false");
    }

    void operator() (spirit::binary_range_type const& str) const {
        out << "\"binary\"";
    }

    void operator() (spirit::utf8_string_range_type const& str) const {
        
        typedef spirit::utf8_string_range_type::const_iterator iterator;
        iterator it = str.begin(), end = str.end();

        out << '"';

        for (; it != end; ++it)
            out << *it;

        out << '"';
    }

    void operator() (spirit::utf8_symbol_range_type const& str) const {
        
        typedef spirit::utf8_symbol_range_type::const_iterator iterator;
        iterator it = str.begin(), end = str.end();

        for (; it != end; ++it)
            out << *it;
    }

    template<typename Iterator>
    void operator() (boost::iterator_range<Iterator> const& range) const {
        BOOST_ASSERT(false);
    }

    void operator() (utree const& ut) const {
        
        using spirit::utree_type;

        switch (ut.which()) {
            case utree_type::reference_type:
                return (*this)(ut.deref());

            case utree_type::range_type:
            case utree_type::list_type:
                if (annotations[ut.tag()].second == json_object) {
                    print_object(ut);
                    return;
                }
                else if (annotations[ut.tag()].second == json_array) {
                    print_array(ut);
                    return;
                }
                else if (annotations[ut.tag()].second == json_pair) {
                    print_member_pair(ut);
                    return;
                }

                default:
                    break;
        }

        utree::visit(ut, *this);
    }

    void operator() (spirit::any_ptr const& p) const {
        return (*this)("pointer");
    }

    void operator() (spirit::function_base const& pf) const {
        return (*this)("function");
    }
};


template<class Char>
bool generate_json (parse_tree const& in,
                    std::basic_ostream<Char>& out)
{
    json_printer<std::basic_ostream<Char> > print(out, in.annotations());
    print(in.ast());
    return true; 
}

template<class Char>
bool generate_json (parse_tree const& in,
                    std::basic_string<Char>& out)
{
    std::basic_stringstream<Char> oss;
    json_printer<std::basic_stringstream<Char> > print(oss, in.annotations());
    print(in.ast());
    out = oss.str();
    return true; 
}

}
#endif 
