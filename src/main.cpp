#include <iostream>
#include <fstream>

//#define BOOST_SPIRIT_DEBUG

#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>
//#include <generate/generate_json.hpp>
//#include <generate/generate_json_dot.hpp>
//#include <generate/generate_mss_dot.hpp>

#include <parse/carto_grammar.hpp>


#include <mml_parser.hpp>
#include <mss_parser.hpp>

#include <mapnik/save_map.hpp>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/include/qi.hpp>
#include <position_iterator.hpp>


int main(int argc, char **argv) {

    using carto::parse_tree;
    
    std::string mapnik_dir = MAPNIKDIR;
    mapnik::datasource_cache::instance()->register_datasources(mapnik_dir); 
    
    namespace po = boost::program_options;
    
    std::string mapnik_input_dir = MAPNIKDIR;
    
    std::string input_file, output_file;
    
    po::options_description desc("carto");
    desc.add_options()
        ("help,h", "produce usage message")
        ("version,V","print version string")
        ("in", po::value<std::string>(&input_file),  "input carto file (mml or mss)")
        ("out", po::value<std::string>(&output_file), "output xml file");
    
    std::string usage("\nusage: carto map.[mml|mss] [map.xml]");
    
    po::positional_options_description p;
    p.add("in",1).add("out",1);
    
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);
    
    if (vm.count("version"))
    {
        std::cout << "version 0.0.0" << std::endl;
        return 1;
    }

    if (vm.count("help")) 
    {
        std::cout << desc << usage << std::endl;
        return 1;
    }

    if (!vm.count("in") 
         || (    !boost::algorithm::ends_with(input_file,".mml")
              && !boost::algorithm::ends_with(input_file,".mss") )
       )
    {
        std::cout << "Please provide an input carto file\n" << std::endl;
        std::cout << desc << usage << std::endl;
        return 1;
    }


    
    try {
        mapnik::Map m(800,600);
        
        if (boost::algorithm::ends_with(input_file,".mml"))
        {
            carto::mml_parser parser = carto::load_mml(input_file, false);
            parser.parse_map(m);
        }
        else if (boost::algorithm::ends_with(input_file,".mss")) 
        {
            carto::mss_parser parser = carto::load_mss(input_file, false);
            carto::style_env env;
            parser.parse_stylesheet(m, env);
        }
        
        std::string output = mapnik::save_map_to_string(m,false);
        
        if (!vm.count("out")) {
            std::cout << output << std::endl;
        } else {
            std::ofstream file;
            file.open(output_file.c_str());
            if (!file.is_open()) {
                std::cout << "Error: could not save xml to: " << output_file << "\n";
                return EXIT_FAILURE;
            }
            file << output;
            file.close();
        }
            
            
       
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    } catch(...) {
        std::cerr << "Error: Unknown error\n";
    }
    
    return 0;
}
