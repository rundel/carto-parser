/*==============================================================================
    Copyright (c) 2010 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef GENERATE_JSON_DOT_H
#define GENERATE_JSON_DOT_H

#include <iosfwd>
#include <sstream>

#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>

namespace carto {

template<class Out>
struct json_dot_printer {
    typedef void result_type;

    Out& out;
    annotations_type annotations;
    int n_id,cur_id;
    std::string prefix;

    json_dot_printer (Out& out_, annotations_type const& annotations_)
      : out(out_), 
        annotations(annotations_),
        n_id(0),
        cur_id(0),
        prefix("node")
    { }

    void print(utree const& ut) {
        out << "digraph simpleDotFile {\n";
        (*this)(ut);
        out << "}";
    }

    void operator() (utree const& ut){
        
        using spirit::utree_type;
        typedef utree::const_iterator iterator;
        
        int id = cur_id;
        n_id++;
        
        switch (ut.which()) {
            case utree_type::reference_type:
                out << prefix << id << " [label=\"[reference]\"];\n"; 
                out << prefix << id << " -> " << prefix << cur_id << ";\n";
                (*this)(ut.deref());
                return;

            case utree_type::range_type:
            case utree_type::list_type:
            {    
                iterator it, end;
                
                int start_id = n_id;
                
                if (annotations[ut.tag()].second == json_object) {
                    out << prefix << id << " [label=\"[object]\"];\n"; 
                    it    = ut.front().begin();
                    end   = ut.front().end();
                    n_id += ut.front().size();
                } else {
                    if (annotations[ut.tag()].second == json_array) {
                        out << prefix << id << " [label=\"[array]\"];\n"; 
                    } else if (annotations[ut.tag()].second == json_pair) {
                        out << prefix << id << " [label=\"[pair]\"];\n"; 
                    } else {
                        BOOST_ASSERT(false);
                        return;
                    }
                    it    = ut.begin();
                    end   = ut.end();
                    n_id += ut.size();
                }
                
                for (int i=0; it != end; ++it, ++i) {
                    
                    cur_id = start_id+i; 
                    out << prefix << id << " -> " << prefix << cur_id << ";\n";
                    (*this)(*it);   
                }
                
                return;
            }    
            default:
                break;
        }

        utree::visit(ut, *this);
    }

    template<class T>
    void operator() (T val){
        out << prefix << cur_id << " [label=";
        out << val;
        out << "];\n";
    }

    void operator() (bool b){
        out << prefix << cur_id << " [label=";
        out << (b ? "true" : "false");
        out << "];\n";
    }

    void operator() (spirit::utf8_string_range_type const& str){
        
        typedef spirit::utf8_string_range_type::const_iterator iterator;
        iterator it = str.begin(), end = str.end();

        out << prefix << cur_id << " [label=\"";
        for (; it != end; ++it)  out << *it;
        out << "\"];\n";
    }

    void operator() (spirit::utf8_symbol_range_type const& str){
        
        typedef spirit::utf8_symbol_range_type::const_iterator iterator;
        iterator it = str.begin(), end = str.end();

        out << prefix << cur_id << " [label=\"";
        for (; it != end; ++it)
            out << *it;
        out << "\"];\n";
    }

    template<typename Iterator>
    void operator() (boost::iterator_range<Iterator> const& range){
        BOOST_ASSERT(false);
    }

    void operator() (utree::invalid_type ut){
        return (*this)("[invalid]");
    }

    void operator() (utree::nil_type ut){
        return (*this)("[null]");
    }

    void operator() (spirit::binary_range_type const& str){
        return (*this)("[binary]");
    }

    void operator() (spirit::any_ptr const& p){
        return (*this)("[pointer]");
    }

    void operator() (spirit::function_base const& pf){
        return (*this)("[function]");
    }
};


template<class Char>
bool generate_dot( parse_tree const& in,
                   std::basic_ostream<Char>& out)
{
    json_dot_printer<std::basic_ostream<Char> > printer(out, in.annotations());
    printer.print(in.ast());
    
    return true; 
}

template<class Char>
bool generate_dot( parse_tree const& in,
                   std::basic_string<Char>& out)
{
    std::basic_stringstream<Char> oss;
    json_dot_printer<std::basic_stringstream<Char> > printer(oss, in.annotations());
    printer.print(in.ast());
    
    out = oss.str();
    return true; 
}

}
#endif 
