#include "GeoWriter.h"

//! ----------------------------------------------------------------------------
//! Main program: GeoGen
//! ----------------------------------------------------------------------------
int main( int i_argc, char **i_argv ) {
  if( i_argc != 5 ) {
    std::cerr << "Usage: " << i_argv[0] << " -f conf_file -o geo_file\n";
    return EXIT_FAILURE;
  } else if( (i_argv == nullptr)   ||
             (i_argv[1][0] != '-') || (i_argv[1][1] != 'f') ||
             (i_argv[3][0] != '-') || (i_argv[3][1] != 'o') ) {
    std::cerr << "Usage: " << i_argv[0] << " -f conf_file -o geo_file\n";
    return EXIT_FAILURE;
  }

  //! Filenames
  std::string l_configFile = std::string( i_argv[2] );
  std::string l_geoFile    = std::string( i_argv[4] );

  //! Writer object
  geo::Writer l_writer( l_geoFile.c_str() );

  //! Parse config file
  l_writer.parseConfigFile( l_configFile.c_str() );

  //! Write geo file
  l_writer.writeGeo();

  return EXIT_SUCCESS;
}