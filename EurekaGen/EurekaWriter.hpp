#ifndef EUREKA_WRITER_HPP
#define EUREKA_WRITER_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "EurekaConstants.h"
#include "../GeoGen/Geo.hpp"

namespace Eureka {
  struct Node;
  struct Elem;
  struct Material;

  class Writer;
}

//! ----------------------------------------------------------------------------
//! Node data-structure
//! ----------------------------------------------------------------------------
struct Eureka::Node {
  real m_x, m_y, m_z;

  Node() : m_x(0.0), m_y(0.0), m_z(0.0) {}

  Node( const real &i_x,
        const real &i_y,
        const real &i_z ) : m_x(i_x), m_y(i_y), m_z(i_z) {}

  //! Assignment operation
  Node & operator = ( const Node &i_node ) {
    this->m_x = i_node.m_x;
    this->m_y = i_node.m_y;
    this->m_z = i_node.m_z;

    return (*this);
  }
};

//! ----------------------------------------------------------------------------
//! Tetrahedron element data-structure
//! ----------------------------------------------------------------------------
struct Eureka::Elem {
  UID m_node1, m_node2, m_node3, m_node4;

  Elem() : m_node1(0), m_node2(0), m_node3(0), m_node4(0) {}

  Elem( const UID &i_node1,
        const UID &i_node2,
        const UID &i_node3,
        const UID &i_node4 ) : m_node1(i_node1), m_node2(i_node2),
                               m_node3(i_node3), m_node4(i_node4) {}
};

//! ----------------------------------------------------------------------------
//! Node data-structure
//! ----------------------------------------------------------------------------
struct Eureka::Material {
  std::string m_name;
  geo::Morph m_morph;
  UID m_numParticles;
  std::vector< real > m_radList;
  std::vector< std::pair< geo::Vector, geo::Vector > > m_cylCP;
  std::vector< geo::Vector > m_sphCP;
  std::vector< UID > m_elemList;

  Material() {}

  Material( const std::string &i_name ) : m_name(i_name) {}
};

//! ----------------------------------------------------------------------------
//! Writer class
//! ----------------------------------------------------------------------------
class Eureka::Writer {
private:
  clock_t m_time;

  UID m_elemID, m_numOfNodes;
  real m_length, m_width, m_height, m_pistonThicc;

  //! Node and element list
  std::map< UID, Eureka::Node > m_nodeMap;
  std::map< UID, Eureka::Elem > m_elemMap;

  //! Material list
  std::vector< Eureka::Material * > m_matList;

  //! Matrix and piston list
  std::vector< UID > m_matrixList, m_pistonList;

  //! Nodal groups
  std::vector< UID > m_topNodes,    m_bottomNodes;
  std::vector< UID > m_leftNodes,   m_rightNodes;
  std::vector< UID > m_frontNodes,  m_backNodes;
  std::vector< UID > m_cornerNodes, m_topCornerNodes;
  std::vector< UID > m_zLeftNodes,  m_zRightNodes;
  std::vector< UID > m_yLeftNodes,  m_yRightNodes;
  std::vector< UID > m_xFrontNodes, m_xBackNodes;
  std::vector< UID > m_matrixNodes, m_pistonNodes;

  std::ifstream m_inMsh, m_inMat;
  std::ofstream m_out;

  void parseMaterials();
  void readNodes();
  void readElems();
  void writeDatFile();
  void writeNodes();
  void writeElems();
  void writeNodalGroups();
  void writeElementGroups();

public:
  Writer( const char *i_inFile,
          const char *i_outFile );
  ~Writer();

  void parseConfigFile( const char *i_filename );
  void readMshWriteDat();
};

#endif