#include <iostream>
#include <fstream>

#include <mapnik/load_map.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>



int main(int argc, char **argv) {
    
    std::cout << "Using MAPNIKDIR = " << MAPNIKDIR << "\n";

    namespace po = boost::program_options;
    
    std::string mapnik_input_dir = MAPNIKDIR;
    
    std::string input_file;
    int width, height;
    
    po::options_description desc("carto");
    desc.add_options()
        ("help,h", "produce this usage message")
        ("version,V","print version string")
        ("xml", po::value<std::string>(&input_file),  "input xml style")
        ("width",  po::value<int>(&width),  "map width")
        ("height", po::value<int>(&height), "map height");

    
    std::string usage("\nusage: render map.xml [--width 800] [--height 600]");
    
    po::positional_options_description p;
    p.add("xml",1);
    
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

    if (!vm.count("xml"))
    {
        std::cout << "Please provide an input xml file\n" << std::endl;
        std::cout << desc << usage << std::endl;
        return 1;
    }

    if (!vm.count("width")) width = 800;
    if (!vm.count("height")) height = 600;
    
    try {

        std::string mapnik_dir = MAPNIKDIR;
        mapnik::datasource_cache::instance().register_datasources(mapnik_dir);
        mapnik::freetype_engine::register_fonts(mapnik_dir + "/../fonts/", TRUE);
        mapnik::freetype_engine::register_fonts("/usr/share/fonts/", TRUE);

        mapnik::Map m(width,height);
        
        mapnik::load_map(m, input_file, TRUE);

        m.zoom_all();
        //m.zoom_to_box(mapnik::box2d<double>(-8584936.07589,4691869.09667,-8561639.82286,4720949.89071));

        mapnik::image_32 buf(m.width(),m.height());
        mapnik::agg_renderer<mapnik::image_32> ren(m,buf);
        ren.apply();

        mapnik::save_to_file(buf,"demo.png","png");
            
       
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    } catch(...) {
        std::cerr << "Error: Unknown error\n";
    }
    
    return 0;
}
