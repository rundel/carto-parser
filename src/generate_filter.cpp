/*==============================================================================
    Copyright (c) 2010 Bryce Lelbach

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <generate/generate_filter.hpp>

#include <iosfwd>
#include <sstream>

#include <parse/parse_tree.hpp>
#include <parse/filter_grammar.hpp>

#include <utility/environment.hpp>
#include <utility/utree.hpp>


namespace carto {

static 
double const zoom_ranges[] = { 1000000000, 500000000, 200000000, 100000000,
                                 50000000,  25000000,  12500000,   6500000,
                                  3000000,   1500000,    750000,    400000,
                                   200000,    100000,     50000,     25000,
                                    12500,      5000,      2500,      1500,
                                      750,       500,       250,       100};

filter_printer::filter_printer(utree const& tree_, annotations_type const& annotations_, 
                               style_env const& env_, mapnik::rule& rule_)
  : tree(tree_),
    annotations(annotations_),
    env(env_),
    rule(rule_)
{}
    
std::string filter_printer::print()
{
    return (*this)(tree);
}
    
utree filter_printer::parse_var(utree const& ut)
{
    BOOST_ASSERT(ut.size()==1);
    BOOST_ASSERT(    annotations[ut.tag()].second == filter_var 
                  || annotations[ut.tag()].second == filter_var_attr);
    
    std::string key = detail::as<std::string>(ut);
    
    //for (; it != end; ++it)
    //    key += detail::as<std::string>(*it);
        
    utree value = env.vars.lookup(key);
    
    if (value == utree::nil_type()) {
        std::string err = std::string("Unknown variable: @")+key; 
        throw config_error(err);
    }
    
    return value;
}
    
double filter_printer::parse_zoom_value(utree const& ut)
{
    if (ut.tag() != 0){
        int node_type = annotations[ut.tag()].second;
    
        if (node_type == filter_var_attr) {
            return as<double>(parse_var(ut));
        } else {
            source_location loc = annotations[ut.tag()].first;
            
            std::stringstream out;
            out << "Invalid node type: " << node_type
                << " at " << loc.get_string();
            throw config_error(out.str());
        }
        
    } else {
        return as<double>(ut);
    }
}
    
std::string filter_printer::operator() (utree const& ut)
{
    using spirit::utree_type;
    
    std::string out;
    
    if (ut.tag() == 0) {
        if (ut.which() == utree_type::list_type) {
            utree::const_iterator it = ut.begin(), end = ut.end();
            for (; it != end; ++it) 
                out += (*this)(*it);
        } else {
            out += as<std::string>(ut);
        }
        
        return out;
    }
    
    int const node_type = annotations[ut.tag()].second;
    
    typedef utree::const_iterator iter;
    iter it = ut.begin(),
        end = ut.end();
    
    if (node_type == filter_var || node_type == filter_var_attr) {
            
        utree value = parse_var(ut);
        
        if (node_type == filter_var_attr) 
            out += "[" + (*this)(value) + "]";
        else
            out += (*this)(value);
    } else if (node_type == filter_and) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        std::string b = (*this)(*it);
        
        if (a == "" || b == "")
            out += (a=="") ? a : b;
        else
            out += "(" + a + " and " + b + ")";

    } else if (node_type ==  filter_or) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        std::string b = (*this)(*it);
        
        if (a == "" || b == "")
            out += (a=="") ? a : b;
        else
            out += "(" + a + " or " + b + ")";
            
    } else if (node_type == filter_not) {
        BOOST_ASSERT(ut.size()==1);
        
        std::string a = (*this)(*it);
        
        if (a != "")
            out += "(not " + a + ")";
    } else if (node_type == filter_match) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        std::string b = (*this)(*it);
        
        out += a + ".match('" + b + "')";
        
    } else if (node_type == filter_replace) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        std::string b = (*this)(*it); it++;
        std::string c = (*this)(*it);
        
        out += a + ".replace('" + b + ", " + c + "')";
        
    } else if (node_type == filter_attribute) {
        //BOOST_ASSERT(ut.size()==1);
        out += "[" + (*this)(*it) + "]";
        
    } else if (node_type == filter_expression) {
        
    } else if (node_type == filter_eq) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        
        if (a == "[zoom]") {
            int b = round(parse_zoom_value(*it));
            rule.set_min_scale(zoom_ranges[b+1]);
            rule.set_max_scale(zoom_ranges[b]);
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
        
    } else if (node_type == filter_neq) {
        BOOST_ASSERT(ut.size()==2);

        std::string a = (*this)(*it); it++;

        if (a == "[zoom]") {
            std::string err = "Not equal is not currently supported for zoom levels (at "
                              + get_location(ut).get_string() + ")"; 
            throw config_error(err);
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
    } else if (node_type ==  filter_le) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        
        if (a == "[zoom]") {
            int b = round(parse_zoom_value(*it));
            rule.set_min_scale(zoom_ranges[b]);
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
        
    } else if (node_type ==  filter_lt) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        
        if (a == "[zoom]") {
            int b = round(parse_zoom_value(*it));
            rule.set_min_scale(zoom_ranges[b-1]);
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
        
    } else if (node_type ==  filter_ge) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        
        if (a == "[zoom]") {
            int b = round(parse_zoom_value(*it));
            rule.set_max_scale(zoom_ranges[b]);
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
        
    } else if (node_type == filter_gt) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        
        if (a == "[zoom]") {
            int b = round(parse_zoom_value(*it));
            rule.set_max_scale(zoom_ranges[b+1]);
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
        
    }
    
    return out;
}

std::string filter_printer::operator() (bool b){
    return b ? "true" : "false";
}

std::string filter_printer::operator() (spirit::utf8_string_range_type const& str)
{
    std::stringstream out;
    
    typedef spirit::utf8_string_range_type::const_iterator iterator;
    iterator it = str.begin(),
            end = str.end();
    
    for (; it != end; ++it)
        out << *it;
        
    return out.str();
}

std::string filter_printer::operator() (spirit::utf8_symbol_range_type const& str)
{
    std::stringstream out;
    
    typedef spirit::utf8_string_range_type::const_iterator iterator;
    iterator it = str.begin(),
            end = str.end();

    for (; it != end; ++it)
        out << *it;
        
    return out.str();
}

}
