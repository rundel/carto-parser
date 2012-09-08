#ifndef EXPRESSION_EVAL_H
#define EXPRESSION_EVAL_H

#include <math.h>

#include <utility/utree.hpp>
#include <utility/environment.hpp>
#include <utility/carto_functions.hpp>

#include <parse/expression_grammar.hpp>

#include <utility/position_iterator.hpp>

namespace carto {

struct expression {

    utree const& tree;
    annotations_type const& annotations;
    style_env const& env;
    
    expression(utree const& tree_, annotations_type const& annotations_, style_env const& env_)
      : tree(tree_),
        annotations(annotations_),
        env(env_) { }
    
    inline int get_node_type(utree const& ut)
    {   
        return annotations[ut.tag()].second;
    }

    inline source_location get_location(utree const& ut)
    {    
        return annotations[ut.tag()].first;
    }

    inline bool is_color(utree const& ut)
    {
        return get_node_type(ut) == CARTO_EXP_COLOR;
    }
    
    inline bool is_double(utree const& ut)
    {
        return ut.which() == spirit::utree_type::double_type;
    }
  
    inline utree eval()
    {
        return eval_node(tree);
    }

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
            BOOST_ASSERT(lhs.size()==rhs.size());                                    \
            BOOST_ASSERT(lhs.size()==4);                                             \
                                                                                     \
            iter lhs_it  = lhs.begin(),                                              \
                 rhs_it  = rhs.begin();                                              \
                                                                                     \
            ut.tag(lhs.tag());                                                       \
            for(int i = 0; i != 3; i++, lhs_it++, rhs_it++) {                        \
                double res = as<double>(*lhs_it) op as<double>(*rhs_it);             \
                ut.push_back( fmod(res, 256) );                                      \
            }                                                                        \
            ut.push_back( (as<double>(*lhs_it) + as<double>(*rhs_it)) / 2 );         \
                                                                                     \
        } else if ( is_double(lhs) && is_color(rhs) ) {                              \
            BOOST_ASSERT(rhs.size()==4);                                             \
            iter rhs_it  = rhs.begin();                                              \
            double d = as<double>(lhs);                                              \
                                                                                     \
            ut.tag(rhs.tag());                                                       \
            for(int i = 0; i != 3; i++, rhs_it++) {                                  \
                double res = d op as<double>(*rhs_it);                               \
                ut.push_back( fmod(res, 256) );                                      \
            }                                                                        \
            ut.push_back( as<double>(*rhs_it) );                                     \
                                                                                     \
        } else if ( is_color(lhs) && is_double(rhs) ) {                              \
            BOOST_ASSERT(lhs.size()==4);                                             \
            iter lhs_it  = lhs.begin();                                              \
            double d = as<double>(rhs);                                              \
                                                                                     \
            ut.tag(lhs.tag());                                                       \
            for(int i = 0; i != 3; i++, lhs_it++) {                                  \
                double res = d op as<double>(*lhs_it);                               \
                ut.push_back( fmod(res,256) );                                       \
            }                                                                        \
            ut.push_back( as<double>(*lhs_it) );                                     \
                                                                                     \
        } else {                                                                     \
            ut = lhs op rhs;                                                         \
        }                                                                            \
                                                                                     \
        return ut;                                                                   \
    }

    EVAL_OP(add, +)
    EVAL_OP(sub, -)
    EVAL_OP(mult, *)
    EVAL_OP(div, /)

};

}
#endif 
