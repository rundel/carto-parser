#include <utility/environment.hpp>

#include <boost/spirit/include/support_utree.hpp>
#include <boost/unordered_map.hpp>

namespace carto {

namespace spirit = boost::spirit;
using spirit::utree;

environment::environment(void): parent(), definitions() { }

environment::environment(environment const& parent_)
  : parent(&parent_), 
    definitions() { }

utree environment::lookup (std::string const& name) const {
    map_type::const_iterator it = definitions.find(name);

    if (it == definitions.end()) {
        if (parent)
            return (parent->lookup(name));

        return utree::nil_type();
    }

    return it->second; 
}

void environment::define (std::string const& name, utree const& val) {
    //BOOST_ASSERT(!definitions.count(name));
    definitions[name] = val;
}

bool environment::defined (std::string const& name) const {
    if (!definitions.count(name)) {
        if (parent)
            return parent->defined(name);

        return false;
    }

    return true; 
}

bool environment::locally_defined (std::string const& name) const {
    return definitions.count(name); 
} 

style_env::style_env() 
  : vars(),
    mixins() { }
    
//style_env::style_env(environment vars_, environment mixins_)
//  : vars(vars_),
//    mixins(mixins_) { }
    
style_env::style_env(style_env const& env)
  : vars(env.vars),
    mixins(env.mixins) { }

}

