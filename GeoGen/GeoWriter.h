#ifndef GEO_WRITER_H
#define GEO_WRITER_H

#include <fstream>
#include <map>
#include <vector>
#include <tuple>
#include <initializer_list>

#include "Geo.hpp"

namespace geo {
  enum class Distrib {
    GAUSSIAN,
    UNIFORM
  };

  struct Material;

  class Writer;
}

//! ----------------------------------------------------------------------------
//! Material-block data-structure
//! ----------------------------------------------------------------------------
struct geo::Material {
  real        m_meshSize, m_radMean, m_lenMean, m_radStdDev, m_lenStdDev;
  real        m_volFrac, m_radMin, m_radMax, m_lenMin, m_lenMax;
  ID          m_count;
  std::string m_name;
  Morph       m_morph;
  Distrib     m_radDistrib, m_lenDistrib;

  Material() : m_meshSize(0.0), m_radMean(0.0), m_lenMean(0.0),
               m_radStdDev(0.0), m_lenStdDev(0.0), m_volFrac(0.0),
               m_radMin(0.0), m_radMax(0.0), m_lenMin(0.0), m_lenMax(0.0),
               m_count(0) {}
};

//! ----------------------------------------------------------------------------
//! Writer class
//! ----------------------------------------------------------------------------
class geo::Writer {
private:
  clock_t m_time;

  //! ID variables
  ID m_pointID;
  ID m_lineID;
  ID m_lineLoopID;
  ID m_surfaceID;
  ID m_surfaceLoopID;

  //! Box dimensions
  real m_length, m_width, m_height;

  //! Global mesh size
  real m_meshSize;

  //! Tolerance between particles
  real m_tolParticles;

  //! Tolerance between paritcles and boundaries
  real m_tolPartBound;

  //! Piston thickness
  real m_pistonThicc;

  //! Random seed
  unsigned int m_seed;

  //! File output stream
  std::ofstream m_out, m_mat;

  //! Surface ID map
  std::map< ID, std::vector< ID > > m_surfMap;

  //! Lists of all created particles
  std::vector< geo::Cylinder > m_cylList;
  std::vector< geo::Sphere >   m_sphList;

  //! Material list
  std::vector< geo::Material * > m_matList;

  //! Collision detection routines
  bool collisionDetection( const geo::Cylinder &i_cylinder ) const;
  bool collisionDetection( const geo::Sphere &i_sphere ) const ;

  //! Check if out of bounds
  bool outOfBounds( const std::initializer_list< geo::Vector > &i_list ) const;

  //! Check for empty fields in config file
  void chkEmpty( const std::string &i_name,
                 const std::string &i_val,
                 const std::string &i_mat = "" );

  //! Helper functions
  void writePoint( const geo::Vector   &i_point,
                   const real          &i_cl );
  void writeLine( const std::pair< real, real > &i_line );
  void writeCircle( const std::tuple< real, real, real > &i_circle );
  void writeLineLoop( const std::initializer_list< ID > &i_list );
  void writePlaneSurface( const ID &i_loopID );
  void writeSurface( const ID &i_loopID );
  void writeSurfaceLoop( const ID                &i_loopID,
                         const std::vector< ID > &i_list );
  void writeSurfaceLoops();
  void writeVolumes();
  void writeControlPoints( const real                                 &i_rad,
                           const std::initializer_list< geo::Vector > &i_list );

  //! Writer functions
  void writeHeader();
  void writeBoxAndPiston();
  void writeCylinder( const geo::Material *i_mat );
  void writeSphere( const geo::Material *i_mat );
  void writeMaterials();
  void writeFooter();

public:
  Writer( const char* i_filename );
  ~Writer();

  void parseConfigFile( const char *i_filename );
  void writeGeo();
};

#endif
