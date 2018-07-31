#include "EurekaWriter.hpp"

//! ----------------------------------------------------------------------------
//! Main program: EurekaGen
//! ----------------------------------------------------------------------------
int main( int i_argc, char **i_argv ) {
  if( i_argc != 7 ) {
    std::cerr << "Usage: " << i_argv[0] << " -f conf_file -i msh_file -o dat_file\n";
    return EXIT_FAILURE;
  } else if( (i_argv == nullptr)   ||
             (i_argv[1][0] != '-') || (i_argv[1][1] != 'f') ||
             (i_argv[3][0] != '-') || (i_argv[3][1] != 'i') ||
             (i_argv[5][0] != '-') || (i_argv[5][1] != 'o') ) {
    std::cerr << "Usage: " << i_argv[0] << " -f conf_file -i msh_file -o dat_file\n";
    return EXIT_FAILURE;
  }

  //! Filenames
  std::string l_conFile = std::string( i_argv[2] );
  std::string l_mshFile = std::string( i_argv[4] );
  std::string l_datFile = std::string( i_argv[6] );

  //! Writer object
  Eureka::Writer l_writer( l_mshFile.c_str(), l_datFile.c_str() );

  //! Parse config file
  l_writer.parseConfigFile( l_conFile.c_str() );

  //! Write header
  l_writer.readMshWriteDat();

  return EXIT_SUCCESS;
}