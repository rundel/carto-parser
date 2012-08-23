/*==============================================================================
    Copyright (c) 2010 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef GENERATE_MSS_DOT_H
#define GENERATE_MSS_DOT_H

#include <iosfwd>
#include <sstream>

#include <parse/parse_tree.hpp>
#include <parse/carto_grammar.hpp>
#include <generate/generate_filter.hpp>

namespace carto {

template<class Out>
struct mss_dot_printer {
    typedef void result_type;

    Out& out;
    annotations_type annotations;
    int n_id,cur_id;
    std::string prefix;

    mss_dot_printer (Out& out_, annotations_type const& annotations_)
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
                carto_node_type nope_type = annotations[ut.tag()].second;

                /*if (annotations[ut.tag()].second == json_object) {
                    out << prefix << id << " [label=\"[object]\"];\n"; 
                    it    = ut.front().begin();
                    end   = ut.front().end();
                    n_id += ut.front().size();
                }*/
                
                out << prefix << id << " [label=\"[";
                switch(node_type) {
                    case CARTO_UNDEFINED:
                        out << "";
                        break;
                    case CARTO_VARIABLE:
                        out << "variable";
                        break;
                    case CARTO_MIXIN:
                        OUT << "mixin";
                        break;
                    case CARTO_STYLE:
                        out << "style";
                        break;
                    case CARTO_FUNCTION:
                        out << "function";
                        break;
                    case CARTO_ATTRIBUTE:
                        out << "attribute";
                        break;
                    case CARTO_COLOR:
                        out << "color";
                        break;
                    case CARTO_FILTER:
                        out << "filter";
                        break;
                    default:
                        std::cout << node_type << std::endl;
                        BOOST_ASSERT(false);
                        //return;
                }
                out << "]\"];\n";
                
                it    = ut.begin();
                end   = ut.end();
                n_id += ut.size();
                
                if (node_type == CARTO_FILTER) {
                    for (int i=0; it != end; ++it, ++i) {
                    
                        cur_id = start_id+i;
                        out << prefix << id << " -> " << prefix << cur_id << ";\n";
                    
                        out << prefix << cur_id << " [label=\"";
                        generate_filter(*it,annotations,style_env(),out);
                        out << "\"];\n";
                    }
                    
                } else {                
                    for (int i=0; it != end; ++it, ++i) {
                    
                        cur_id = start_id+i; 
                        out << prefix << id << " -> " << prefix << cur_id << ";\n";
                        (*this)(*it);   
                    }
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


/*template<class Char>
bool generate_mss_dot( utree const& ast, annotations_type const& annotations,
                       std::basic_ostream<Char>& out)
{
    mss_dot_printer<std::basic_ostream<Char> > printer(out, annotations);
    printer.print(ast);
    
    return true; 
}

template<class Char>
bool generate_mss_dot( utree const& ast, annotations_type const& annotations,
                       std::basic_string<Char>& out)
{
    std::basic_stringstream<Char> oss;
    mss_dot_printer<std::basic_stringstream<Char> > printer(oss, annotations);
    printer.print(ast);
    
    out = oss.str();
    return true; 
}*/

template<class Char>
bool generate_mss_dot( parse_tree const& in,
                   std::basic_ostream<Char>& out)
{
    mss_dot_printer<std::basic_ostream<Char> > printer(out, in.annotations());
    printer.print(in.ast());
    
    return true; 
}

template<class Char>
bool generate_mss_dot( parse_tree const& in,
                   std::basic_string<Char>& out)
{
    std::basic_stringstream<Char> oss;
    mss_dot_printer<std::basic_stringstream<Char> > printer(oss, in.annotations());
    printer.print(in.ast());
    
    out = oss.str();
    return true; 
}

}
#endif 
