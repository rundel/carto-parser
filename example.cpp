#include <iostream>
#include <fstream>

#define BOOST_SPIRIT_DEBUG

#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>
#include <generate/generate_json.hpp>
#include <generate/generate_json_dot.hpp>
#include <generate/generate_mss_dot.hpp>

#include <parse/carto_grammar.hpp>


#include <mml_parser.hpp>
#include <mss_parser.hpp>

#include <mapnik/save_map.hpp>

#include <boost/filesystem.hpp>


#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/include/qi.hpp>
#include <position_iterator.hpp>

enum out_type {
    out_ast,
    out_json,
    out_dot,
    out_xml
};

int main(int argc, char **argv) {

    using carto::parse_tree;
    using carto::generate_json;
    using carto::generate_dot;
    using carto::generate_mss_dot;
    using boost::filesystem::path;
    
    std::string mapnik_dir = MAPNIKDIR;
    mapnik::datasource_cache::instance()->register_datasources(mapnik_dir + "/lib/mapnik2/input/"); 
    
    char const* filename;
    path p;
    
    out_type out = out_xml;
    
    std::string usage = std::string("Usage: ") + argv[0] + " [-o output] <filename>\n";
    
    if (argc != 2 && argc != 4) {
        std::cout << usage;
        return 1;
    } else if (argc == 2) {
        filename = argv[1];
        p = path(argv[1]);
    } else {
        if (strncmp(argv[1],"-o",2) != 0) {
            std::cout << usage;
            std::cout << "\"" << argv[1] << "\" option not recognized.\n";
            return 1;
        }
        
        if (strncmp(argv[2],"ast",3)==0)
            out = out_ast;
        else if (strncmp(argv[2],"json",4)==0)
            out = out_json;
        else if (strncmp(argv[2],"dot",3)==0)
            out = out_dot;
        else if (strncmp(argv[2],"xml",3)==0)
            out = out_xml;
        else {
            std::cout << usage;
            std::cout << "output must be one of the following values: ast, json, dot, xml.\n";
            return 1;
        }
            
        filename = argv[3];
        p = path(argv[3]);
    }

    mapnik::Map m(800,600);
    
    if (p.extension() ==  ".mml") {
        
        carto::mml_parser parser = carto::load_mml(filename, false);
        parser.parse_map(m);

        parse_tree pt = parser.get_parse_tree();
        std::string output;
        
        switch(out) {
            case out_ast:
                std::cout << pt.ast();
                break;
            case out_json:
                generate_json(pt,output);
                break;
            case out_dot:
                generate_dot(pt,output);
                break;
            case out_xml:
                output = mapnik::save_map_to_string(m,false);
                break;
        }
        
        std::cout << output << std::endl;
                
    } else if (p.extension() ==  ".mss") {
        
        /*
        std::ifstream file(filename, std::ios_base::in);

        if (!file) { 
            std::cout << "Error: " << filename << " does not exist!\n";
            return 1;
        }

        std::string in;
        file.unsetf(std::ios::skipws);
        copy(std::istream_iterator<char>(file),
             std::istream_iterator<char>(),
             std::back_inserter(in));
        
        
        
        
        using boost::spirit::utree;
        using carto::position_iterator;
        
        utree ast;    
        carto::annotations_type annotations;
        
        typedef position_iterator<std::string::const_iterator> iterator_type;
        //typedef std::string::const_iterator iterator_type;
        
        
        carto::carto_parser<iterator_type> p("file", annotations);

        iterator_type first(in.begin());
        iterator_type last(in.end());
        
        try {
            bool r = boost::spirit::qi::phrase_parse(first, last, p, boost::spirit::ascii::space, ast);
            if (!r) {
                std::cout << "Parse failed\n";
                return 1;
            }
        } catch (std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        } catch(...) {
            std::cerr << "Error: Unknown error\n";
        }*/
        
        carto::mss_parser parser = carto::load_mss(filename, false);
        parser.parse_stylesheet(m);

        parse_tree pt = parser.get_parse_tree();
        std::string output;

        switch(out) {
            case out_ast:
                std::cout << pt.ast();
                break;
            case out_xml:
                output = mapnik::save_map_to_string(m,false);
                break;
            case out_json:
                //generate_json(pt,output);
                std::cout << "Not supported";
                break;
            case out_dot:
                generate_mss_dot(pt,output);
                break;
        }
        std::cout << output << std::endl;
        
    } else {
        std::cout << "Error: " << filename << " has an invalid extension " << p.extension() << "\n";
        return 1;
    }

    
    return 0;
}
