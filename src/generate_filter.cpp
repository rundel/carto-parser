#include <generate/generate_filter.hpp>

#include <iosfwd>
#include <sstream>

#include <parse/parse_tree.hpp>
#include <parse/filter_grammar.hpp>

#include <utility/environment.hpp>
#include <utility/utree.hpp>
#include <utility/carto_error.hpp>


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
    BOOST_ASSERT(    annotations[ut.tag()].second == FILTER_VAR 
                  || annotations[ut.tag()].second == FILTER_VAR_ATTR);
    
    std::string key = as<std::string>(ut);
    
    //for (; it != end; ++it)
    //    key += detail::as<std::string>(*it);
        
    utree value = env.vars.lookup(key);
    
    if (value == utree::nil_type())
        throw carto_error("Unknown variable: @"+key, get_location(ut));
    
    return value;
}
    
double filter_printer::parse_zoom_value(utree const& ut)
{
    if (ut.tag() != 0){
        int node_type = annotations[ut.tag()].second;
    
        if (node_type == FILTER_VAR_ATTR) {
            return round( as<double>(parse_var(ut)) );
        } else {
            std::stringstream out;
            out << "Invalid node type: " << node_type;
            throw carto_error(out.str(), get_location(ut));
        }
        
    } else {
        return round( as<double>(ut) );
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
    
    if (node_type == FILTER_VAR || node_type == FILTER_VAR_ATTR) {
            
        utree value = parse_var(ut);
        
        if (node_type == FILTER_VAR_ATTR) 
            out += "[" + (*this)(value) + "]";
        else
            out += (*this)(value);
    } else if (node_type == FILTER_AND) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        std::string b = (*this)(*it);
        
        if (a == "" || b == "")
            out += (a=="") ? a : b;
        else
            out += "(" + a + " and " + b + ")";

    } else if (node_type ==  FILTER_OR) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        std::string b = (*this)(*it);
        
        if (a == "" || b == "")
            out += (a=="") ? a : b;
        else
            out += "(" + a + " or " + b + ")";
            
    } else if (node_type == FILTER_NOT) {
        BOOST_ASSERT(ut.size()==1);
        
        std::string a = (*this)(*it);
        
        if (a != "")
            out += "(not " + a + ")";
    } else if (node_type == FILTER_MATCH) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        std::string b = (*this)(*it);
        
        out += a + ".match('" + b + "')";
        
    } else if (node_type == FILTER_REPLACE) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        std::string b = (*this)(*it); it++;
        std::string c = (*this)(*it);
        
        out += a + ".replace('" + b + ", " + c + "')";
        
    } else if (node_type == FILTER_ATTRIBUTE) {
        //BOOST_ASSERT(ut.size()==1);
        out += "[" + (*this)(*it) + "]";
        
    } else if (node_type == FILTER_EXPRESSION) {
        
    } else if (node_type == FILTER_EQ) {
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
        
    } else if (node_type == FILTER_NEQ) {
        BOOST_ASSERT(ut.size()==2);

        std::string a = (*this)(*it); it++;

        if (a == "[zoom]") {
            throw carto_error("Not equal is not currently supported for zoom levels", get_location(ut));
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
    } else if (node_type ==  FILTER_LE) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        
        if (a == "[zoom]") {
            int b = round(parse_zoom_value(*it));
            rule.set_min_scale(zoom_ranges[b+1]);
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
        
    } else if (node_type ==  FILTER_LT) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        
        if (a == "[zoom]") {
            int b = round(parse_zoom_value(*it));
            rule.set_min_scale(zoom_ranges[b]);
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
        
    } else if (node_type ==  FILTER_GE) {
        BOOST_ASSERT(ut.size()==2);
        
        std::string a = (*this)(*it); it++;
        
        if (a == "[zoom]") {
            int b = round(parse_zoom_value(*it));
            rule.set_max_scale(zoom_ranges[b]);
        } else {
            std::string b = (*this)(*it);
            out += "(" + a + " = " + b + ")";
        }
        
    } else if (node_type == FILTER_GT) {
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
