#include <iostream>
#include <fstream>

#include <config.hpp>
#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>
#include <generate/generate_json.hpp>
#include <generate/generate_dot.hpp>

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
    {
        std::cerr << "Error: Could not open input file: " << filename << "\n";
        return 1;
    }
    
    
    std::string in;
    file.unsetf(std::ios::skipws);
    copy(std::istream_iterator<char>(file),
         std::istream_iterator<char>(),
         std::back_inserter(in));
    
     parse_tree pt(in);
     
     std::cout << pt.ast() << std::endl;
     
     std::string out;
     generate_json(pt,out);
     std::cout << out << std::endl;
     
     generate_dot(pt,out);
     std::cout << out << std::endl;

    
    return(0);
}
