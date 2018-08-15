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
 * EurekaGen program.
 **/

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