#ifndef EXPRESSION_EVAL_H
#define EXPRESSION_EVAL_H

#include <math.h>

#include <utility/utree.hpp>
#include <utility/environment.hpp>
#include <utility/carto_functions.hpp>

#include <parse/expression_grammar.hpp>

#include <position_iterator.hpp>

namespace carto {

struct expression {

    utree const& tree;
    annotations_type const& annotations;
    style_env const& env;
    
    expression(utree const& tree_, annotations_type const& annotations_, style_env const& env_);
    
    template<class T>
    T as(utree const& ut)
    {
        return detail::as<T>(ut);
    }
    
    int get_node_type(utree const& ut);
    
    source_location get_location(utree const& ut);
    
    inline bool is_color(utree const& ut)
    {
        return get_node_type(ut) == exp_color;
    }
    
    inline bool is_double(utree const& ut)
    {
        return ut.which() == spirit::utree_type::double_type;
    }
  
    utree eval();

    utree eval_var(utree const& node);
    
    utree eval_node(utree const& node);
    
    utree eval_function(utree const& node);
    
    utree fix_color_range(utree const& node);

#define EVAL_OP(name, op)                                                            \
    utree eval_##name(utree const& lhs, utree const& rhs)                            \
    {                                                                                \
        typedef utree::const_iterator iter;                                          \
        utree ut;                                                                    \
                                                                                     \
        if ( is_color(lhs) && is_color(rhs) ) {                                      \
            iter lhs_it  = lhs.begin(),                                              \
                 rhs_it  = rhs.begin();                                              \
                                                                                     \
            ut.tag(lhs.tag());                                                       \
            for(; lhs_it != lhs.end() && rhs_it != rhs.end(); lhs_it++, rhs_it++)    \
                ut.push_back( as<double>(*lhs_it) op as<double>(*rhs_it) );          \
                                                                                     \
        } else if ( is_double(lhs) && is_color(rhs) ) {                              \
            iter rhs_it  = rhs.begin();                                              \
            double d = as<double>(lhs);                                              \
                                                                                     \
            ut.tag(rhs.tag());                                                       \
            for(; rhs_it != rhs.end(); rhs_it++)                                     \
                ut.push_back( d op as<double>(*rhs_it) );                            \
                                                                                     \
        } else if ( is_color(lhs) && is_double(rhs) ) {                              \
            iter lhs_it  = lhs.begin();                                              \
            double d = as<double>(rhs);                                              \
                                                                                     \
            ut.tag(lhs.tag());                                                       \
            for(; lhs_it != lhs.end(); lhs_it++)                                     \
                ut.push_back( d op as<double>(*lhs_it) );                            \
                                                                                     \
        } else {                                                                     \
            ut = lhs op rhs;                                                         \
        }                                                                            \
                                                                                     \
        return (get_node_type(ut) == exp_color) ? fix_color_range(ut) : ut;          \
    }                                                                                \
    /***/

    EVAL_OP(add, +);
    EVAL_OP(sub, -);
    EVAL_OP(mult, *);
    EVAL_OP(div, /);

};

}
#endif 
