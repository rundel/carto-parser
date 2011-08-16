/*==============================================================================
    Copyright (c) 2010 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef GENERATE_FILTER_H
#define GENERATE_FILTER_H

#include <iosfwd>
#include <sstream>

#include <parse/parse_tree.hpp>
#include <parse/filter_grammar.hpp>

#include <utility/environment.hpp>
#include <utility/utree.hpp>


namespace carto {

template<class Out>
struct filter_printer {
    typedef void result_type;

    Out& out;
    annotations_type const& annotations;
    style_env const& env;

    filter_printer (Out& out_, annotations_type const& annotations_, style_env const& env_)
      : out(out_), 
        annotations(annotations_),
        env(env_)
    {}

    void print(utree const& ut) {
        (*this)(ut);
    }

    void operator() (utree const& ut){
        
        //std::cout << ut << " " << ut.which() << " " << ut.tag() << " " << annotations[ut.tag()].second << "\n";
        
        using spirit::utree_type;
        
        if (ut.tag() == 0) {
            if (ut.which() == utree_type::list_type) {
                utree::const_iterator it = ut.begin(), end = ut.end();
                for (; it != end; ++it) 
                    (*this)(*it);
            } else {
                utree::visit(ut, *this);
            }
            
        } else {
            typedef utree::const_iterator iter;
            iter it = ut.begin(),
                end = ut.end();
            
            std::string key;
            
            switch (annotations[ut.tag()].second) {
                case filter_var:
                    //FIXME
                    for (; it != end; ++it)
                        key += detail::as<std::string>(*it);
                    
                    (*this)(*(env.vars.lookup(key)));
                    break;
                case filter_var_attr:
                    //FIXME
                    for (; it != end; ++it)
                        key += detail::as<std::string>(*it);
                    
                    out << "[";
                    (*this)(*(env.vars.lookup(key)));
                    out << "]";
                    break;
                
                case filter_and:
                    BOOST_ASSERT(ut.size()==2);
                    out << "(";
                    (*this)(*it++);
                    out << " and ";
                    (*this)(*it);
                    out << ")";
                    break;
                case filter_or:
                    BOOST_ASSERT(ut.size()==2);
                    out << "(";
                    (*this)(*it++);
                    out << " or ";
                    (*this)(*it);
                    out << ")";
                    break;
                case filter_not:
                    BOOST_ASSERT(ut.size()==2);
                    out << "(not (";
                    (*this)(*it);
                    out << "))";
                    break;
                case filter_eq:
                    BOOST_ASSERT(ut.size()==2);
                    out << "(";
                    (*this)(*it++);
                    out << " = ";
                    (*this)(*it);
                    out << ")";
                    break;
                case filter_neq:
                    BOOST_ASSERT(ut.size()==2);
                    out << "(";
                    (*this)(*it++);
                    out << " != ";
                    (*this)(*it);
                    out << ")";
                    break;
                case filter_le:
                    BOOST_ASSERT(ut.size()==2);
                    out << "(";
                    (*this)(*it++);
                    out << " <= ";
                    (*this)(*it);
                    out << ")";
                    break;
                case filter_lt:
                    BOOST_ASSERT(ut.size()==2);
                    out << "(";
                    (*this)(*it++);
                    out << " < ";
                    (*this)(*it);
                    out << ")";
                    break;
                case filter_ge:
                    BOOST_ASSERT(ut.size()==2);
                    out << "(";
                    (*this)(*it++);
                    out << " >= ";
                    (*this)(*it);
                    out << ")";
                    break;
                case filter_gt:
                    BOOST_ASSERT(ut.size()==2);
                    out << "(";
                    (*this)(*it++);
                    out << " > ";
                    (*this)(*it);
                    out << ")";
                    break;
                case filter_match:
                    BOOST_ASSERT(ut.size()==2);
                    (*this)(*it++);
                    out << ".match('";
                    (*this)(*it);
                    out << "')";
                    break;
                case filter_replace:
                    BOOST_ASSERT(ut.size()==3);
                    (*this)(*it++);
                    out << ".replace('";
                    (*this)(*it++);
                    out << "','";
                    (*this)(*it);
                    out << "')";
                    break;
                case filter_attribute:
                    //BOOST_ASSERT(ut.size()==1);
                    out << "[";
                    for (; it != end; ++it)
                        (*this)(*it);
                    out << "]";
                    break;
                case filter_expression:
                    break;
            }
        }
    }

    template<class T>
    void operator() (T val){
        out << val;
    }

    void operator() (bool b){
        out << (b ? "true" : "false");
    }

    void operator() (spirit::utf8_string_range_type const& str){
        
        typedef spirit::utf8_string_range_type::const_iterator iterator;
        iterator it = str.begin(), end = str.end();

        for (; it != end; ++it)  out << *it;
    }

    void operator() (spirit::utf8_symbol_range_type const& str){
        
        typedef spirit::utf8_symbol_range_type::const_iterator iterator;
        iterator it = str.begin(), end = str.end();

        for (; it != end; ++it) out << *it;
    }

    template<typename Iterator>
    void operator() (boost::iterator_range<Iterator> const& range){
        BOOST_ASSERT(false);
    }

    void operator() (utree::invalid_type ut){
        BOOST_ASSERT(false);
    }

    void operator() (utree::nil_type ut){
        BOOST_ASSERT(false);
    }

    void operator() (spirit::binary_range_type const& str){
        BOOST_ASSERT(false);
    }

    void operator() (spirit::any_ptr const& p){
        BOOST_ASSERT(false);
    }

    void operator() (spirit::function_base const& pf){
        BOOST_ASSERT(false);
    }
};

//template<class Char>
//bool generate_filter( parse_tree const& in, std::basic_ostream<Char>& out)
//{
//    filter_printer<std::basic_ostream<Char> > printer(out, in.annotations());
//    printer.print(in.ast());
//    
//    return true; 
//}
//
//template<class Char>
//bool generate_filter( parse_tree const& in, std::basic_string<Char>& out)
//{
//    std::basic_stringstream<Char> oss;
//    filter_printer<std::basic_stringstream<Char> > printer(oss, in.annotations());
//    printer.print(in.ast());
//    
//    out = oss.str();
//    return true; 
//}


template<class Char>
bool generate_filter( utree const& ast, annotations_type const& annotations, 
                      style_env const& env, std::basic_ostream<Char>& out)
{
    filter_printer<std::basic_ostream<Char> > printer(out, annotations, env);
    printer.print(ast);
    
    return true; 
}

template<class Char>
bool generate_filter( utree const& ast, annotations_type const& annotations, 
                      style_env const& env, std::basic_string<Char>& out)
{
    std::basic_stringstream<Char> oss;
    filter_printer<std::basic_stringstream<Char> > printer(oss, annotations, env);
    printer.print(ast);
    
    out = oss.str();
    return true; 
}

}
#endif 
