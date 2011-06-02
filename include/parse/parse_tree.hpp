/*==============================================================================
    Copyright (c) 2010-2011 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/home/support/assert_msg.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include <position_iterator.hpp>
#include <parse/json_grammar.hpp>

namespace carto {

using boost::spirit::utree;

//template<typename parser_type>
class parse_tree {

private:
    utree _ast;    
    annotations_type _annotations;
    
    bool equal (parse_tree const& other) const {
        return    (_ast == other._ast)
               && (_annotations == other._annotations);
    }

public:
    parse_tree(void)
      : _ast(), 
        _annotations()
    { }

    parse_tree(std::string const& in)
      : _ast(),
        _annotations()
    { 
        typedef position_iterator<std::string::const_iterator> iterator_type;
        
        //FIXME
        json_parser<iterator_type> p("std::string", _annotations);

        iterator_type first(in.begin());
        iterator_type last(in.end());

        bool r = qi::phrase_parse(first, last, p, boost::spirit::ascii::space, _ast);
        if (!r)
            std::cout << "Parse failed\n";
    }   
    
    
    parse_tree& operator= (parse_tree const& other) {
        if (!equal(other)) {
            _ast = other._ast;
            _annotations = other._annotations;
        }
        return *this;
    }  

    utree& ast (void) {
        return _ast;
    }

    utree const& ast (void) const {
        return _ast;
    }

    annotations_type& annotations (void) {
        return _annotations;
    }

    annotations_type const& annotations (void) const {
        return _annotations;
    }

    annotation_type& annotations (int i) {
        return _annotations[i];
    }

    annotation_type const& annotations (int i) const {
        return _annotations[i];
    }

    bool operator== (parse_tree const& other) const {
        return equal(other);
    }

    bool operator!= (parse_tree const& other) const {
        return !equal(other);
    }

};

}

#endif
