/*==============================================================================
    Copyright (c) 2001-2010 Joel de Guzman
    Copyright (c) 2010      Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef ANNOTATOR_H
#define ANNOTATOR_H

#include <limits>

#include <utility/utree.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <utility/position_iterator.hpp>

namespace carto {

typedef std::pair<source_location, int> annotation_type;
typedef std::vector< annotation_type > annotations_type;

namespace qi = boost::spirit::qi;
using qi::raw;
using boost::spirit::utree;

struct push_annotation_impl {
    
    template<typename T1,typename T2,typename T3>
    struct result {
        typedef void type;
    };
    
    annotations_type& annotations;

    push_annotation_impl(annotations_type& annotations_);

    template<class RangeIter>
    void operator() (utree& ast, int type, RangeIter const& rng) const {
        
        annotations.push_back( annotation_type(get_location(rng.begin()), type) );
        std::size_t n = annotations.size() - 1;

        BOOST_ASSERT(n <= (std::numeric_limits<short>::max)());
        ast.tag(n);
    }
};

template<class Iterator>
struct annotator : qi::grammar<Iterator, void(utree&, int)> {

    qi::rule<Iterator, void(utree&, int)> start;
    boost::phoenix::function<push_annotation_impl> const push;

    annotator (annotations_type& annotations)
      : annotator::base_type(start), 
        push(push_annotation_impl(annotations))
    {
        using qi::omit;
        using qi::raw;
        using qi::eps;
        using qi::_1;
        using qi::_r1;
        using qi::_r2;

        start = omit[raw[eps] [push(_r1, _r2, _1)]];
    }
};

}

#endif





