#include <iostream>
#include <fstream>

#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>
#include <generate/generate_json.hpp>
#include <generate/generate_dot.hpp>

#include <mml_parser.hpp>

#include <mapnik/save_map.hpp>

int main(int argc, char **argv) {

    using carto::parse_tree;
    using carto::generate_json;
    using carto::generate_dot;
    
    
    char const* filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        std::cerr << "Error: No input file provided.\n";
        return 1;
    }

    std::ifstream file(filename, std::ios_base::in);

    if (!file)
        throw std::exception();

    std::string in;
    file.unsetf(std::ios::skipws);
    copy(std::istream_iterator<char>(file),
         std::istream_iterator<char>(),
         std::back_inserter(in));

    mapnik::Map m(800,600);
    

    carto::mml_parser parser(in, filename);
    parser.parse_map(m, false);
    
    parse_tree pt = parser.get_parse_tree();
    
    //std::cout << pt.ast() << std::endl;
    //
    std::string out;
    //generate_json(pt,out);
    //std::cout << out << std::endl << std::endl;
    //
    //generate_dot(pt,out);
    //std::cout << out << std::endl << std::endl;
    //
    out = mapnik::save_map_to_string(m,false);
    std::cout << out << std::endl << std::endl;
    
    
    return(0);
}
