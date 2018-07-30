#include <sstream>

#include "EurekaWriter.hpp"

//! ----------------------------------------------------------------------------
//! Constructor
//! ----------------------------------------------------------------------------
Eureka::Writer::Writer( const char *i_inFile,
                        const char *i_outFile ) : m_time(clock()), m_elemID(1) {
  //! Open .msh file
  m_inMsh.open( i_inFile, std::ifstream::in );
  if( !m_inMsh.is_open() ) {
    std::cerr << "Couldn't open " << i_inFile << "! Exiting..\n";
    exit( EXIT_FAILURE );
  }

  //! Open .dat file
  m_out.open( i_outFile, std::ofstream::out );
  if( !m_out.is_open() ) {
    std::cerr << "Couldn't open " << i_outFile << "! Exiting..\n";
    exit( EXIT_FAILURE );
  }
}

//! ----------------------------------------------------------------------------
//! Destructor
//! ----------------------------------------------------------------------------
Eureka::Writer::~Writer() {
  //! Close files
  m_inMsh.close();
  m_inMat.close();
  m_out.close();

  //! Delete materials
  std::vector< Eureka::Material * >::const_iterator l_it;
  for( l_it = m_matList.begin(); l_it != m_matList.end(); ++l_it )
    delete (*l_it);

  //! Display time taken
  m_time = clock() - m_time;
  std::cout << "Done!\nTime taken = " << (float) m_time / CLOCKS_PER_SEC << "s\n";
}

//! ----------------------------------------------------------------------------
//! Parse the materials
//! ----------------------------------------------------------------------------
void Eureka::Writer::parseMaterials() {
  //! Open GeoGen.mat file
  m_inMat.open( "GeoGen.mat", std::ifstream::in );
  if( !m_inMat.is_open() ) {
    std::cerr << "Couldn't open GeoGen.mat! Exiting..\n";
    exit( EXIT_FAILURE );
  }

  std::string l_s;

  while( m_inMat ) {
    if( !std::getline( m_inMat, l_s ) )
      break;

    //! Create a new material with read-in name
    Eureka::Material *l_mat = new Eureka::Material( l_s );

    //! Get morphology
    std::getline( m_inMat, l_s );
    if( l_s.compare( "cyl" ) == 0 )
      l_mat->m_morph = geo::Morph::CYLINDER;
    else if( l_s.compare( "sph" ) == 0 )
      l_mat->m_morph = geo::Morph::SPHERE;
    else {
      std::cerr << "Unknown morphology (" << l_s << ")! Exiting..\n";
      m_inMsh.close();
      m_inMat.close();
      m_out.close();
      exit( EXIT_FAILURE );
    }

    //! Get number of particles
    std::getline( m_inMat, l_s );
    l_mat->m_numParticles = StrToUID( l_s );

    //! Reserve vectors
    l_mat->m_radList.reserve( l_mat->m_numParticles );
    l_mat->m_cylCP.reserve( l_mat->m_numParticles );
    l_mat->m_sphCP.reserve( l_mat->m_numParticles );

    //! Read the radius and control points
    for( UID l_i = 0; l_i < l_mat->m_numParticles; l_i++ ) {
      std::getline( m_inMat, l_s );
      std::istringstream l_ss( l_s );
      std::vector< std::string > l_words;

      while( l_ss ) {
        std::string l_next;

        //! Get the next string before hitting a space and put it in next
        if( !std::getline( l_ss, l_next, ' ' ) )
          break;

        l_words.push_back( l_next );
      }

      //! Should have exactly 7 columns (cyl) or 4 columns (sph)
      if( l_words.size() != 7 && l_words.size() != 4 )
        continue;

      //! Populate radius and control point list
      l_mat->m_radList.push_back( StrToReal( l_words[0] ) );

      switch( l_mat->m_morph ) {
        case geo::Morph::CYLINDER:
          l_mat->m_cylCP.push_back( std::make_pair(
                                      geo::Vector( StrToReal( l_words[1] ),
                                                   StrToReal( l_words[2] ),
                                                   StrToReal( l_words[3] ) ),
                                      geo::Vector( StrToReal( l_words[4] ),
                                                   StrToReal( l_words[5] ),
                                                   StrToReal( l_words[6] ) ) ) );
          break;
        case geo::Morph::SPHERE:
          l_mat->m_sphCP.push_back( geo::Vector( StrToReal( l_words[1] ),
                                                 StrToReal( l_words[2] ),
                                                 StrToReal( l_words[3] ) ) );
      }
    }

    //! Populate material list
    m_matList.push_back( l_mat );
  }
}

//! ----------------------------------------------------------------------------
//! Read in nodes from msh file
//! ----------------------------------------------------------------------------
void Eureka::Writer::readNodes() {
  std::cout << "Reading in nodes.. " << std::flush;
  std::string l_s;

  //! Skip first four lines
  for( int l_i = 0; l_i <= 4; l_i++ )
    std::getline( m_inMsh, l_s );

  //! Get number of nodes from next line
  m_numOfNodes = StrToUID( l_s );

  //! Store in node-info while skipping those lines for now
  for( UID l_i = 0; l_i < m_numOfNodes; l_i++ ) {
    std::getline( m_inMsh, l_s );

    std::istringstream l_ss( l_s );
    std::vector< std::string > l_words;

    while( l_ss ) {
      std::string l_next;

      //! Get the next string before hitting a space and put it in next
      if( !std::getline( l_ss, l_next, ' ' ) )
        break;

      l_words.push_back( l_next );
    }

    //! We should have exactly 4 columns
    if( l_words.size() != 4 )
      continue;

    UID l_id = StrToUID(  l_words[0] );
    real l_x = StrToReal( l_words[1] );
    real l_y = StrToReal( l_words[2] );
    real l_z = StrToReal( l_words[3] );

    //! Populate node map
    m_nodeMap[l_id] = Eureka::Node( l_x, l_y, l_z );

    bool l_flag = false;

    //! top_nodes
    if( l_z == m_height ) {
      m_topNodes.push_back( l_id );
      l_flag = true;
    }

    //! bottom_nodes
    if( l_z == 0.0 ) {
      m_bottomNodes.push_back( l_id );
      l_flag = true;
    }

    //! left_nodes
    if( l_x == 0.0 ) {
      m_leftNodes.push_back( l_id );
      l_flag = true;
    }

    //! right_nodes
    if( l_x == m_length ) {
      m_rightNodes.push_back( l_id );
      l_flag = true;
    }

    //! front_nodes
    if( l_y == 0.0 ) {
      m_frontNodes.push_back( l_id );
      l_flag = true;
    }

    //! back_nodes
    if( l_y == m_width ) {
      m_backNodes.push_back( l_id );
      l_flag = true;
    }

    //! corner_nodes
    if( (l_x == 0.0 || l_x == m_length) && (l_y == 0.0 || l_y == m_width) &&
        l_z == 0.0 ) {
      m_cornerNodes.push_back( l_id );
      l_flag = true;
    }

    //! top_corner_nodes
    if( (l_x == 0.0 || l_x == m_length) && (l_y == 0.0 || l_y == m_width) &&
        l_z == m_height ) {
      m_topCornerNodes.push_back( l_id );
      l_flag = true;
    }

    //! zleft_nodes
    if( l_x == 0.0 && (l_y == 0.0 || l_y == m_width) ) {
      m_zLeftNodes.push_back( l_id );
      l_flag = true;
    }

    //! zright_nodes
    if( l_x == m_length && (l_y == 0.0 || l_y == m_width) ) {
      m_zRightNodes.push_back( l_id );
      l_flag = true;
    }

    //! yleft_nodes
    if( l_x == 0.0 && (l_z == 0.0 || l_z == m_height) ) {
      m_yLeftNodes.push_back( l_id );
      l_flag = true;
    }

    //! yright_nodes
    if( l_x == m_length && (l_z == 0.0 || l_z == m_height) ) {
      m_yRightNodes.push_back( l_id );
      l_flag = true;
    }

    //! xfront_nodes
    if( l_y == 0.0 && (l_z == 0.0 || l_z == m_height) ) {
      m_xFrontNodes.push_back( l_id );
      l_flag = true;
    }

    //! xback_nodes
    if( l_y == m_width && (l_z == 0.0 || l_z == m_height) ) {
      m_xBackNodes.push_back( l_id );
      l_flag = true;
    }

    //! If none of the above means node lies in either piston or matrix
    if( !l_flag ) {
      if( l_z >= (m_height - m_pistonThicc) )
        m_pistonNodes.push_back( l_id );
      else
        m_matrixNodes.push_back( l_id );
    }
  }
}

//! ----------------------------------------------------------------------------
//! Read in elements from msh file
//! ----------------------------------------------------------------------------
void Eureka::Writer::readElems() {
  std::cout << "Done!\nReading in elems.. " << std::flush;
  std::string l_s;

  while( m_inMsh ) {
    if( !std::getline( m_inMsh, l_s ) )
      break;

    //! Ignore lines starting with '$'
    if( l_s[0] == '$' )
      continue;

    std::istringstream l_ss( l_s );
    std::vector< std::string > l_words;

    while( l_ss ) {
      std::string l_next;

      //! Get the next string before hitting a space and put it in next
      if( !std::getline( l_ss, l_next, ' ' ) )
        break;

      l_words.push_back( l_next );
    }

    //! We should have exactly 9 columns and only for tets
    if( l_words.size() != 9 && l_words[1] != "4" )
      continue;

    UID l_n1 = StrToUID( l_words[5] );
    UID l_n2 = StrToUID( l_words[6] );
    UID l_n3 = StrToUID( l_words[7] );
    UID l_n4 = StrToUID( l_words[8] );

    //! Populate elem map
    m_elemMap[m_elemID] = Eureka::Elem( l_n1, l_n2, l_n3, l_n4 );

    //! Get nodes
    Eureka::Node l_node1 = m_nodeMap[l_n1];
    Eureka::Node l_node2 = m_nodeMap[l_n2];
    Eureka::Node l_node3 = m_nodeMap[l_n3];
    Eureka::Node l_node4 = m_nodeMap[l_n4];

    //! Calculate centroid (P) of tet
    real l_x = (l_node1.m_x + l_node2.m_x + l_node3.m_x + l_node4.m_x) / 4.0;
    real l_y = (l_node1.m_y + l_node2.m_y + l_node3.m_y + l_node4.m_y) / 4.0;
    real l_z = (l_node1.m_z + l_node2.m_z + l_node3.m_z + l_node4.m_z) / 4.0;
    geo::Vector l_P( l_x, l_y, l_z );

    //! Check if tet lies inside piston
    if( l_z >= (m_height - m_pistonThicc) ) {
      m_pistonList.push_back( m_elemID++ );
      continue;
    }

    bool l_found = false;
    //! Check if tet lies inside any of the particles
    std::vector< Eureka::Material * >::const_iterator l_it;
    for( l_it = m_matList.begin(); l_it != m_matList.end(); ++l_it ) {
      //! Temporary material pointer
      Eureka::Material *l_mat = *l_it;

      l_found = false;
      real l_c1 = 0.0, l_c2 = 0.0, l_d = 0.0;

      for( UID l_i = 0; l_i < l_mat->m_numParticles; l_i++ ) {
        if( l_mat->m_morph == geo::Morph::CYLINDER ) {
          //! A & B are end pts of cylinder axis
          geo::Vector l_A = l_mat->m_cylCP[l_i].first;
          geo::Vector l_B = l_mat->m_cylCP[l_i].second;

          //! AB.AP
          l_c1 = geo::dot( geo::Vector( l_A, l_B ), geo::Vector( l_A, l_P ) );

          //! BA.BP
          l_c1 = geo::dot( geo::Vector( l_B, l_A ), geo::Vector( l_B, l_P ) );

          //! Distance of P from line joining A & B
          l_d  = geo::norm( geo::cross( geo::Vector( l_A, l_P ),
                                        geo::Vector( l_B, l_P ) ) ) /
                                        geo::norm( geo::Vector( l_A, l_B ) );

          //! Condition that P lies inside cyl defined by control pts A & B
          if( l_c1 >= 0.0 && l_c2 >= 0.0 && l_d <= l_mat->m_radList[l_i] ) {
            (l_mat->m_elemList).push_back( m_elemID );

            //! Safe to avoid further checking as particles are not intersecting
            l_found = true;
            break;
          }
        }

        else if( l_mat->m_morph == geo::Morph::SPHERE ) {
          //! Condition that P lies inside sph is if dist is less than radius
          if( geo::dist( l_P, l_mat->m_sphCP[l_i] ) <= l_mat->m_radList[l_i] ) {
            (l_mat->m_elemList).push_back( m_elemID );

            //! Safe to avoid further checking as particles are not intersecting
            l_found = true;
            break;
          }
        }
      }

      //! Skip checking further materials as 1 element belongs in 1 material
      if( l_found )
        break;
    }

    //! If at this pt means elem lies inside matrix
    if( !l_found )
      m_matrixList.push_back( m_elemID );

    //! Increment element ID
    m_elemID++;
  }
}

//! ----------------------------------------------------------------------------
//! Write to dat file
//! ----------------------------------------------------------------------------
void Eureka::Writer::writeDatFile() {
  m_out << "3 4 " << m_numOfNodes << " " << m_elemMap.size() << std::endl;

  //! Write nodes
  writeNodes();

  //! Write elems
  writeElems();

  //! Write nodal groups
  writeNodalGroups();

  //! Write element groups
  writeElementGroups();
}

//! ----------------------------------------------------------------------------
//! Write nodes
//! ----------------------------------------------------------------------------
void Eureka::Writer::writeNodes() {
  std::cout << "Done!\nWriting nodes.. " << std::flush;
  std::map< UID, Eureka::Node >::iterator l_nodeIt;

  //! Write nodes
  for( l_nodeIt = m_nodeMap.begin(); l_nodeIt != m_nodeMap.end(); ++l_nodeIt )
    m_out << l_nodeIt->first << " " << (l_nodeIt->second).m_x << " "
          << (l_nodeIt->second).m_y << " " << (l_nodeIt->second).m_z << std::endl;
}

//! ----------------------------------------------------------------------------
//! Write elements
//! ----------------------------------------------------------------------------
void Eureka::Writer::writeElems() {
  std::cout << "Done!\nWriting elems.. " << std::flush;
  std::map< UID, Eureka::Elem >::iterator l_elemIt;

  //! Write elems
  for( l_elemIt = m_elemMap.begin(); l_elemIt != m_elemMap.end(); ++l_elemIt )
    m_out << l_elemIt->first << " "
          << (*l_elemIt).second.m_node1 << " " << (*l_elemIt).second.m_node2 << " "
          << (*l_elemIt).second.m_node3 << " " << (*l_elemIt).second.m_node4 << "\n";
}

//! ----------------------------------------------------------------------------
//! Write nodal groups
//! ----------------------------------------------------------------------------
void Eureka::Writer::writeNodalGroups() {
  std::cout << "Done!\nWriting nodal groups.. " << std::flush;
  std::vector< UID >::const_iterator l_it;

  m_out << "7 top_nodes " << m_topNodes.size() << std::endl;
  for( l_it = m_topNodes.begin(); l_it != m_topNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 bottom_nodes " << m_bottomNodes.size() << std::endl;
  for( l_it = m_bottomNodes.begin(); l_it != m_bottomNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 left_nodes " << m_leftNodes.size() << std::endl;
  for( l_it = m_leftNodes.begin(); l_it != m_leftNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 right_nodes " << m_rightNodes.size() << std::endl;
  for( l_it = m_rightNodes.begin(); l_it != m_rightNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 front_nodes " << m_frontNodes.size() << std::endl;
  for( l_it = m_frontNodes.begin(); l_it != m_frontNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 back_nodes " << m_backNodes.size() << std::endl;
  for( l_it = m_backNodes.begin(); l_it != m_backNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 corner_nodes " << m_cornerNodes.size() << std::endl;
  for( l_it = m_cornerNodes.begin(); l_it != m_cornerNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 top_corner_nodes " << m_topCornerNodes.size() << std::endl;
  for( l_it = m_topCornerNodes.begin(); l_it != m_topCornerNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 zleft_nodes " << m_zLeftNodes.size() << std::endl;
  for( l_it = m_zLeftNodes.begin(); l_it != m_zLeftNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 zright_nodes " << m_zRightNodes.size() << std::endl;
  for( l_it = m_zRightNodes.begin(); l_it != m_zRightNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 yleft_nodes " << m_yLeftNodes.size() << std::endl;
  for( l_it = m_yLeftNodes.begin(); l_it != m_yLeftNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 yright_nodes " << m_yRightNodes.size() << std::endl;
  for( l_it = m_yRightNodes.begin(); l_it != m_yRightNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 xfront_nodes " << m_xFrontNodes.size() << std::endl;
  for( l_it = m_xFrontNodes.begin(); l_it != m_xFrontNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 xback_nodes " << m_xBackNodes.size() << std::endl;
  for( l_it = m_xBackNodes.begin(); l_it != m_xBackNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 matrix_nodes " << m_matrixNodes.size() << std::endl;
  for( l_it = m_matrixNodes.begin(); l_it != m_matrixNodes.end(); ++l_it )
    m_out << *l_it << std::endl;

  m_out << "7 piston_nodes " << m_pistonNodes.size() << std::endl;
  for( l_it = m_pistonNodes.begin(); l_it != m_pistonNodes.end(); ++l_it )
    m_out << *l_it << std::endl;
}

//! ----------------------------------------------------------------------------
//! Write element groups
//! ----------------------------------------------------------------------------
void Eureka::Writer::writeElementGroups() {
  std::cout << "Done!\nWriting element groups.. " << std::flush;

  std::vector< UID >::const_iterator        l_it;
  std::vector< Material * >::const_iterator l_matIt;

  //! Matrix
  m_out << "8 matrix " << m_matrixList.size() << std::endl;
  for( l_it = m_matrixList.begin(); l_it != m_matrixList.end(); ++l_it )
    m_out << *l_it << std::endl;

  //! Piston
  m_out << "8 Piston " << m_pistonList.size() << std::endl;
  for( l_it = m_pistonList.begin(); l_it != m_pistonList.end(); ++l_it )
    m_out << *l_it << std::endl;

  //! Materials
  for( l_matIt = m_matList.begin(); l_matIt != m_matList.end(); ++l_matIt ) {
    Eureka::Material *l_mat = *l_matIt;
    m_out << "8 " << l_mat->m_name << " " << l_mat->m_elemList.size() << std::endl;

    for( l_it = (l_mat->m_elemList).begin(); l_it != (l_mat->m_elemList).end();
                                                                        ++l_it )
      m_out << *l_it << std::endl;
  }
}

//! ----------------------------------------------------------------------------
//! Config file parser
//! ----------------------------------------------------------------------------
void Eureka::Writer::parseConfigFile( const char *i_filename ) {
  std::ifstream l_confFn( i_filename, std::ios::in );
  if( !l_confFn.is_open() ) {
    std::cerr << "Cannot open " << i_filename << "! Exiting..\n";
    m_inMsh.close();
    m_out.close();
    exit( EXIT_FAILURE );
  }

  std::string l_lineBuf;

  while( getline( l_confFn, l_lineBuf ) ) {
    size_t l_k = -1, l_l;

    while( (++l_k < l_lineBuf.length()) && (l_lineBuf[l_k] == ' ') );

    if( (l_k >= l_lineBuf.length()) || (l_lineBuf[l_k] == '#') )
      continue;

    l_l = l_k - 1;

    while( (++l_l < l_lineBuf.length()) && (l_lineBuf[l_l] != '=') );

    if( l_l >= l_lineBuf.length() )
      continue;

    std::string l_varName  = l_lineBuf.substr( l_k, l_l - l_k );
    std::string l_varValue = l_lineBuf.substr( l_l + 1 );

    //! Skip entries without values
    if( l_varValue.empty() )
      continue;

    //! Box
    if( l_varName.compare( "length" ) == 0 )
      m_length      = StrToReal( l_varValue );
    else if( l_varName.compare( "width" ) == 0 )
      m_width       = StrToReal( l_varValue );
    else if( l_varName.compare( "height" ) == 0 )
      m_height      = StrToReal( l_varValue );
    else if( l_varName.compare( "piston_thicc" ) == 0 )
      m_pistonThicc = StrToReal( l_varValue );
  }

  l_confFn.close();

  parseMaterials();
}

//! ----------------------------------------------------------------------------
//! Read msh file and write to dat file
//! ----------------------------------------------------------------------------
void Eureka::Writer::readMshWriteDat() {
  //! Read Nodes
  readNodes();

  //! Read elems
  readElems();

  //! Write dat file
  writeDatFile();
}