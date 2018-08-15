/**
 * @file This file is part of MeshGen.
 *
 * @section LICENSE
 * MIT License
 *
 * Copyright (c) 2018 Rajdeep Konwar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @section DESCRIPTION
 * GeoGen program.
 **/

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