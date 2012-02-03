#ifndef EXPRESSION_EVAL_H
#define EXPRESSION_EVAL_H

#include <math.h>

#include <utility/utree.hpp>
#include <utility/environment.hpp>
#include <utility/carto_functions.hpp>

namespace carto {

using mapnik::config_error;

struct expression {

    utree const& tree;
    annotations_type const& annotations;
    style_env const& env;
    
    expression(utree const& tree_, annotations_type const& annotations_, style_env const& env_)
      : tree(tree_),
        annotations(annotations_),
        env(env_) { }
    
    template<class T>
    T as(utree const& ut)
    {
        return detail::as<T>(ut);
    }
    
    int get_node_type(utree const& ut)
    {   
        return( annotations[ut.tag()].second );
    }
    
    source_location get_location(utree const& ut)
    {    
        return annotations[ut.tag()].first;
    }
    
    bool is_color(utree const& ut)
    {
        return get_node_type(ut) == exp_color;
    }
    
    bool is_double(utree const& ut)
    {
        return ut.which() == spirit::utree_type::double_type;
    }
    
    
    
    utree eval()
    {
        return eval_node(tree);
    }
    
    utree eval_var(utree const& node) {
        std::string key = as<std::string>(node);
        
        utree value = env.vars.lookup(key);
        
        if (value == utree::nil_type()) {
            std::stringstream err;
            err << "Unknown variable: @" << key
                << " at " << get_location(node).get_string(); 
            throw config_error(err.str());
        }
        
        return (get_node_type(value) == carto_variable) ? eval_var(value) : value;
    }
    
    utree eval_node(utree const& node)
    {
        
        //std::cout << node << " " << node.which() << " " << get_node_type(node) << "\n";
        
        if (node.which() == spirit::utree_type::double_type) {
            return node;
        } else if (node.which() == spirit::utree_type::list_type) {
            switch(get_node_type(node)) {
                case exp_plus:
                    BOOST_ASSERT(node.size()==2);
                    return eval_add( eval_node(node.front()), eval_node(node.back()) );
                    //return eval_node(node.front())+eval_node(node.back());
                case exp_minus:
                    BOOST_ASSERT(node.size()==2);
                    return eval_sub( eval_node(node.front()), eval_node(node.back()) );
                case exp_times:
                    BOOST_ASSERT(node.size()==2);
                    return eval_mult( eval_node(node.front()), eval_node(node.back()) );
                case exp_divide:
                    BOOST_ASSERT(node.size()==2);
                    return eval_div( eval_node(node.front()), eval_node(node.back()) );
                case exp_neg:
                    BOOST_ASSERT(node.size()==1);
                    return eval_mult( utree(-1.0), eval_node(node.back()) );
                case exp_function:
                    //std::cout << "Size: " << node.size() << "\n";
                    return eval_function(node);
                case exp_color:
                    BOOST_ASSERT(node.size()==4);
                    return node;
                case exp_var:
                    return eval_var(node);
                default:
                {
                    std::stringstream out;
                    out << "Invalid expression node type: " << get_node_type(node)
                        << " at " << get_location(node).get_string();
                    throw config_error(out.str());
                }
            }
        } else {
            std::cout << "Shouldn't be here!\n";
        }
        
        return utree();
    }
    
    utree eval_function(utree const& node)
    {
        typedef utree::const_iterator iter;
        
        iter it  = node.begin(),
             end = node.end();
        
        std::string func_name = as<std::string>(*it);
        ++it;
        
        if (func_name == "test") {
            utree color = eval_node(*it); ++it;
            return test(color);
        } else if (func_name == "hue") {
            utree color = eval_node(*it); ++it;
            return hue(color);
        } else if (func_name == "saturation") {
            utree color = eval_node(*it); ++it;
            return saturation(color);
        } else if (func_name == "lightness") {
            utree color = eval_node(*it); ++it;
            return lightness(color);
        } else if (func_name == "alpha") {
            utree color = eval_node(*it); ++it;
            return alpha(color);
        } else if (func_name == "saturate") {
            utree color = eval_node(*it); ++it;
            utree value = eval_node(*it); ++it;
            return saturate(color,value);    
        } else if (func_name == "desaturate") {
            utree color = eval_node(*it); ++it;
            utree value = eval_node(*it); ++it;
            return desaturate(color,value);
        } else if (func_name == "lighten") {
            utree color = eval_node(*it); ++it;
            utree value = eval_node(*it); ++it;
            return lighten(color,value);
        } else if (func_name == "darken") {
            utree color = eval_node(*it); ++it;
            utree value = eval_node(*it); ++it;
            return darken(color,value);
        } else if (func_name == "fadein") {
            utree color = eval_node(*it); ++it;
            utree value = eval_node(*it); ++it;
            return fadein(color,value);
        } else if (func_name == "fadeout") {
            utree color = eval_node(*it); ++it;
            utree value = eval_node(*it); ++it;
            return fadeout(color,value);
        } else if (func_name == "spin") {
            utree color = eval_node(*it); ++it;
            utree value = eval_node(*it); ++it;
            return spin(color,value);
        } else if (func_name == "mix") {    
            utree color1 = eval_node(*it); ++it;
            utree color2 = eval_node(*it); ++it;
            utree weight = eval_node(*it); ++it;
            return mix(color1, color2, weight);
        } else if (func_name == "greyscale") {
            utree color = eval_node(*it); ++it;
            return greyscale(color);
        } else {
            std::stringstream err;
            err << "Unknown function: " << func_name
                << " at " << get_location(node).get_string();
            throw config_error(err.str());
        }
    }
    
    utree fix_color_range(utree const& node) 
    {
        BOOST_ASSERT(is_color(node) == TRUE);
        BOOST_ASSERT(node.size() == 4);

        typedef utree::const_iterator iter;
        
        utree ut;
        for(iter it = node.begin(); it != node.end(); it++)
            ut.push_back( fmod(as<double>(*it), 256) );
        
        return ut;
    }

#define EVAL_OP(name, op)                                                            \
    utree eval_##name(utree const& lhs, utree const& rhs)                             \
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
