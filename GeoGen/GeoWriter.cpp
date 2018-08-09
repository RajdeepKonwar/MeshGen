#include <chrono>
#include <ctime>
#include <random>

#include "GeoWriter.h"

//! ----------------------------------------------------------------------------
//! Randomizer
//! ----------------------------------------------------------------------------
template< typename T >
inline T randomizer( const T &i_low,
              const T &i_high ) {
  return (static_cast< T >(rand()) /
          static_cast< T >(RAND_MAX / (i_high - i_low)) + i_low);
}

//! ----------------------------------------------------------------------------
//! Constructor
//! ----------------------------------------------------------------------------
geo::Writer::Writer( const char* i_filename ) : m_time(clock()),
                                                m_pointID(1),
                                                m_lineID(1),
                                                m_lineLoopID(1),
                                                m_surfaceID(1),
                                                m_surfaceLoopID(3),
                                                m_length(10000.0),
                                                m_width(5000.0),
                                                m_height(5500.0),
                                                m_meshSize(200.0),
                                                m_tolParticles(50.0),
                                                m_tolPartBound(50.0),
                                                m_pistonThicc(500.0),
                                                m_seed(0) {
  m_out.open( i_filename, std::ofstream::out );
  if( !m_out.is_open() ) {
    std::cerr << "Couldn't open " << i_filename << "! Exiting..\n";
    exit( EXIT_FAILURE );
  }

  m_mat.open( "GeoGen.mat", std::ofstream::out );
  if( !m_mat.is_open() ) {
    std::cerr << "Couldn't open GeoGen.mat! Exiting..\n";
    exit( EXIT_FAILURE );
  }
}

//! ----------------------------------------------------------------------------
//! Destructor
//! ----------------------------------------------------------------------------
geo::Writer::~Writer() {
  //! Close output files
  m_out.close();
  m_mat.close();

  //! Delete materials
  std::vector< geo::Material * >::const_iterator l_it;
  for( l_it = m_matList.begin(); l_it != m_matList.end(); ++l_it )
    delete (*l_it);

  //! Display time taken
  m_time = clock() - m_time;
  std::cout << "Done!\nTime taken = " << (float) m_time / CLOCKS_PER_SEC << "s\n";
}

//! ----------------------------------------------------------------------------
//! Perform collision detection for a cylinder
//! ----------------------------------------------------------------------------
bool geo::Writer::collisionDetection( const geo::Cylinder &i_cylinder ) const {
  std::vector< geo::Cylinder >::const_iterator l_cylIt;
  std::vector< geo::Sphere >::const_iterator   l_sphIt;

  //! Perform collision detection against cylinders
  for( l_cylIt = m_cylList.begin(); l_cylIt != m_cylList.end(); ++l_cylIt ) {
    //! Vector perpendicular to both cylinder axes
    geo::Vector l_n = geo::cross( i_cylinder.m_axis, l_cylIt->m_axis );

    //! Vector joining base centers
    geo::Vector l_p1p2( i_cylinder.m_center, l_cylIt->m_center );

    //! Distance b/w the cylinder axes
    real l_d  = std::abs( geo::dot( l_p1p2, l_n ) ) / geo::norm( l_n );

    //! Collision check
    if( l_d <= (i_cylinder.m_radius + l_cylIt->m_radius + m_tolParticles) )
      return true;
  }

  //! Perform collision detection against spheres
  for( l_sphIt = m_sphList.begin(); l_sphIt != m_sphList.end(); ++l_sphIt ) {
    //! Vector connecting sphere center to cylinder base center
    geo::Vector l_ap( i_cylinder.m_center, l_sphIt->m_center );

    //! Distance of sphere center from cylinder axis
    real l_d  = geo::norm( geo::cross( l_ap, i_cylinder.m_axis ) ) /
                                                geo::norm( i_cylinder.m_axis );

    //! Collision check
    if( l_d <= (l_sphIt->m_radius + i_cylinder.m_radius + m_tolParticles ) )
      return true;
  }

  return false;
}

//! ----------------------------------------------------------------------------
//! Perform collision detection for a sphere
//! ----------------------------------------------------------------------------
bool geo::Writer::collisionDetection( const geo::Sphere &i_sphere ) const {
  std::vector< geo::Cylinder >::const_iterator l_cylIt;
  std::vector< geo::Sphere >::const_iterator   l_sphIt;

  //! Perform collision detection against cylinders
  for( l_cylIt = m_cylList.begin(); l_cylIt != m_cylList.end(); ++l_cylIt ) {
    //! Vector connecting sphere center to cylinder base center
    geo::Vector l_ap( l_cylIt->m_center, i_sphere.m_center );

    //! Distance of sphere center from cylinder axis
    real l_d  = geo::norm( geo::cross( l_ap, l_cylIt->m_axis ) ) /
                                                  geo::norm( l_cylIt->m_axis );

    //! Collision check
    if( l_d <= (i_sphere.m_radius + l_cylIt->m_radius + m_tolParticles ) )
      return true;
  }

  //! Perform collision detection against spheres
  for( l_sphIt = m_sphList.begin(); l_sphIt != m_sphList.end(); ++l_sphIt )
    if( geo::dist( l_sphIt->m_center, i_sphere.m_center ) <=
                      (l_sphIt->m_radius + i_sphere.m_radius + m_tolParticles) )
      return true;

  return false;
}

//! ----------------------------------------------------------------------------
//! Checks if any point in list lies out of bounds
//! ----------------------------------------------------------------------------
bool geo::Writer::outOfBounds( const std::initializer_list< geo::Vector > &i_list ) const {
  std::initializer_list< geo::Vector >::const_iterator l_it;

  for( l_it = i_list.begin(); l_it != i_list.end(); ++l_it )
    if( l_it->m_x <= m_tolPartBound || l_it->m_x >= (m_length - m_tolPartBound) ||
        l_it->m_y <= m_tolPartBound || l_it->m_y >= (m_width  - m_tolPartBound) ||
        l_it->m_z <= m_tolPartBound ||
        l_it->m_z >= (m_height - m_pistonThicc - m_tolPartBound) )
      return true;

  return false;
}

//! ----------------------------------------------------------------------------
//! Write point to geo script
//! ----------------------------------------------------------------------------
inline void geo::Writer::writePoint( const geo::Vector   &i_point,
                                     const real          &i_cl ) {
  m_out << "Point(" << m_pointID++ << ") = { " << i_point.m_x
        << "," << i_point.m_y << "," << i_point.m_z
        << "," << i_cl << " };\n";
}

//! ----------------------------------------------------------------------------
//! Write line to geo script
//! ----------------------------------------------------------------------------
inline void geo::Writer::writeLine( const std::pair< real, real > &i_line ) {
  m_out << "Line(" << m_lineID++ << ") = { " << i_line.first
        << "," << i_line.second << " };\n";
}

//! ----------------------------------------------------------------------------
//! Write circle arc to geo script
//! ----------------------------------------------------------------------------
inline void geo::Writer::writeCircle( const std::tuple< real, real, real > &i_circle ) {
  m_out << "Circle(" << m_lineID++ << ") = { "
        << std::get< 0 >(i_circle) << "," << std::get< 1 >(i_circle) << ","
        << std::get< 2 >(i_circle) << " };\n";
}

//! ----------------------------------------------------------------------------
//! Write line loop to geo script
//! ----------------------------------------------------------------------------
inline void geo::Writer::writeLineLoop( const std::initializer_list< ID > &i_list ) {
  m_out << "Line Loop(" << m_lineLoopID++ << ") = { ";

  std::initializer_list< ID >::const_iterator l_it;
  for( l_it = i_list.begin(); l_it != i_list.end(); ++l_it )
    m_out << *l_it << ((l_it + 1) == i_list.end() ? "" : ",");

  m_out << " };\n";
}

//! ----------------------------------------------------------------------------
//! Write plane surface to geo script
//! ----------------------------------------------------------------------------
inline void geo::Writer::writePlaneSurface( const ID &i_loopID ) {
  m_out << "Plane Surface(" << m_surfaceID++ << ") = { " << i_loopID << " };\n";
}

//! ----------------------------------------------------------------------------
//! Write surface filling to geo script
//! ----------------------------------------------------------------------------
inline void geo::Writer::writeSurface( const ID &i_loopID ) {
  m_out << "Surface(" << m_surfaceID++ << ") = { " << i_loopID << " };\n";
}

//! ----------------------------------------------------------------------------
//! Write surface loop to geo script
//! ----------------------------------------------------------------------------
inline void geo::Writer::writeSurfaceLoop( const ID                &i_loopID,
                                           const std::vector< ID > &i_list ) {
  m_out << "Surface Loop(" << i_loopID << ") = { ";

  std::vector< ID >::const_iterator l_it;
  for( l_it = i_list.begin(); l_it != i_list.end(); ++l_it )
    m_out << *l_it << ((l_it + 1) == i_list.end() ? "" : ",");

  m_out << " };\n";
}

//! ----------------------------------------------------------------------------
//! Write all surface loops to geo script
//! ----------------------------------------------------------------------------
void geo::Writer::writeSurfaceLoops() {
  //! 1st surface loop is for box
  m_out << "Surface Loop(1) = { ";
  for( ID l_i = 1; l_i < m_surfaceID; l_i++ ) {
    //! Skip piston surfaces
    if( l_i == 2 || l_i == 4 || l_i == 6 || l_i == 8 || l_i == 10 )
      continue;

    m_out << l_i << ((l_i + 1) == m_surfaceID ? "" : ",");
  }

  //! Second surface loop is for piston volume
  m_out << " };\nSurface Loop(2) = { 2, 4, 6, 8, 10, 11 };\n";


  //! Write surface loops for each particle in map
  std::map< ID, std::vector< ID > >::const_iterator l_it;
  for( l_it = m_surfMap.begin(); l_it != m_surfMap.end(); ++l_it )
    writeSurfaceLoop( l_it->first, l_it->second );
}

//! ----------------------------------------------------------------------------
//! Write all volumes to geo script
//! ----------------------------------------------------------------------------
void geo::Writer::writeVolumes() {
  for( ID l_i = 1; l_i < m_surfaceLoopID; l_i++ )
    m_out << "Volume(" << l_i << ") = { " << l_i << " };\n";
}

//! ----------------------------------------------------------------------------
//! Write a control points to mat file
//! ----------------------------------------------------------------------------
void geo::Writer::writeControlPoints( const real                                 &i_rad,
                                      const std::initializer_list< geo::Vector > &i_list ) {
    m_mat << i_rad;

    std::initializer_list< geo::Vector >::const_iterator l_it;
    for( l_it = i_list.begin(); l_it != i_list.end(); ++l_it )
      m_mat << " " << l_it->m_x << " " << l_it->m_y << " " << l_it->m_z;

    m_mat << std::endl;
}

//! ----------------------------------------------------------------------------
//! Write header info to geo script
//! ----------------------------------------------------------------------------
void geo::Writer::writeHeader() {
  if( !m_seed )
    m_seed = (unsigned) time( nullptr );

  //! Seed the randomizer
  srand( m_seed );

  std::chrono::system_clock::time_point l_p = std::chrono::system_clock::now();
  std::time_t l_t = std::chrono::system_clock::to_time_t( l_p );

  m_out << "/** Gmsh geometry script generated by GeoGen (author: Rajdeep Konwar)\n"
        << " *  Copyright (c) 2018, Robert Bosch LLC\n"
        << " *  Timestamp: " << std::ctime( &l_t )
        << " *  Rand seed: " << m_seed << "\n"
        << " **/\n\n";

  //! Frontal algorithm
  m_out << "Mesh.Algorithm = 6;\n\n";
}

//! ----------------------------------------------------------------------------
//! Write box dimensions to geo script
//! ----------------------------------------------------------------------------
void geo::Writer::writeBoxAndPiston() {
  m_out << "//! Box\n";

  //! Height of piston matrix interface
  real l_piston = m_height - m_pistonThicc;

  //! Points
  writePoint( geo::Vector( 0.0,      0.0,     0.0      ), m_meshSize );   //! 1
  writePoint( geo::Vector( m_length, 0.0,     0.0      ), m_meshSize );   //! 2
  writePoint( geo::Vector( m_length, m_width, 0.0      ), m_meshSize );   //! 3
  writePoint( geo::Vector( 0.0,      m_width, 0.0      ), m_meshSize );   //! 4
  writePoint( geo::Vector( 0.0,      0.0,     l_piston ), m_meshSize );   //! 5
  writePoint( geo::Vector( m_length, 0.0,     l_piston ), m_meshSize );   //! 6
  writePoint( geo::Vector( m_length, m_width, l_piston ), m_meshSize );   //! 7
  writePoint( geo::Vector( 0.0,      m_width, l_piston ), m_meshSize );   //! 8
  writePoint( geo::Vector( 0.0,      0.0,     m_height ), m_meshSize );   //! 9
  writePoint( geo::Vector( m_length, 0.0,     m_height ), m_meshSize );   //! 10
  writePoint( geo::Vector( m_length, m_width, m_height ), m_meshSize );   //! 11
  writePoint( geo::Vector( 0.0,      m_width, m_height ), m_meshSize );   //! 12

  m_out << std::endl;

  //! Lines
  writeLine( std::make_pair( 1, 2 ) );      //! 1
  writeLine( std::make_pair( 2, 3 ) );      //! 2
  writeLine( std::make_pair( 3, 4 ) );      //! 3
  writeLine( std::make_pair( 4, 1 ) );      //! 4
  writeLine( std::make_pair( 1, 5 ) );      //! 5
  writeLine( std::make_pair( 2, 6 ) );      //! 6
  writeLine( std::make_pair( 3, 7 ) );      //! 7
  writeLine( std::make_pair( 4, 8 ) );      //! 8
  writeLine( std::make_pair( 5, 6 ) );      //! 9
  writeLine( std::make_pair( 6, 7 ) );      //! 10
  writeLine( std::make_pair( 7, 8 ) );      //! 11
  writeLine( std::make_pair( 8, 5 ) );      //! 12
  writeLine( std::make_pair( 9, 10 ) );     //! 13
  writeLine( std::make_pair( 10, 11 ) );    //! 14
  writeLine( std::make_pair( 11, 12 ) );    //! 15
  writeLine( std::make_pair( 12, 9 ) );     //! 16
  writeLine( std::make_pair( 5, 9 ) );      //! 17
  writeLine( std::make_pair( 6, 10 ) );     //! 18
  writeLine( std::make_pair( 7, 11 ) );     //! 19
  writeLine( std::make_pair( 8, 12 ) );     //! 20

  m_out << std::endl;

  //! Line loops
  writeLineLoop( {  8, -11,  -7,   3 } );   //! 1
  writeLineLoop( { 20, -15, -19,  11 } );   //! 2
  writeLineLoop( {  4,   5, -12,  -8 } );   //! 3
  writeLineLoop( { 17, -16, -20,  12 } );   //! 4
  writeLineLoop( {  6,  -9,  -5,   1 } );   //! 5
  writeLineLoop( {  9,  18, -13, -17 } );   //! 6
  writeLineLoop( {  2,   7, -10,  -6 } );   //! 7
  writeLineLoop( { 10,  19, -14, -18 } );   //! 8
  writeLineLoop( { -1,  -4,  -3,  -2 } );   //! 9
  writeLineLoop( { 13,  14,  15,  16 } );   //! 10
  writeLineLoop( { -9, -12, -11, -10 } );   //! 11

  m_out << std::endl;

  //! Plane surfaces
  writePlaneSurface( 1 );
  writePlaneSurface( 2 );
  writePlaneSurface( 3 );
  writePlaneSurface( 4 );
  writePlaneSurface( 5 );
  writePlaneSurface( 6 );
  writePlaneSurface( 7 );
  writePlaneSurface( 8 );
  writePlaneSurface( 9 );
  writePlaneSurface( 10 );
  writePlaneSurface( 11 );

  m_out << std::endl;
}

//! ----------------------------------------------------------------------------
//! Write cylindrical particle to geo script
//! ----------------------------------------------------------------------------
void geo::Writer::writeCylinder( const geo::Material *i_mat ) {
  m_out << "//! Cylinder\n";

  //! Cylinder control points
  ID l_cpC1 = m_pointID;       //! Center (left)
  ID l_cp1  = m_pointID + 1;   //! Bottom (-z)
  ID l_cp2  = m_pointID + 2;   //! Top    (+z)
  ID l_cp3  = m_pointID + 3;   //! Rear   (-x)
  ID l_cp4  = m_pointID + 4;   //! Front  (+x)

  ID l_cpC2 = m_pointID + 5;   //! Center (right)
  ID l_cp5  = m_pointID + 6;   //! Bottom (-z)
  ID l_cp6  = m_pointID + 7;   //! Top    (+z)
  ID l_cp7  = m_pointID + 8;   //! Rear   (-x)
  ID l_cp8  = m_pointID + 9;   //! Front  (+x)

  //! Circle arc IDs
  ID l_ca1  = m_lineID;
  ID l_ca2  = m_lineID + 1;
  ID l_ca3  = m_lineID + 2;
  ID l_ca4  = m_lineID + 3;
  ID l_ca5  = m_lineID + 4;
  ID l_ca6  = m_lineID + 5;
  ID l_ca7  = m_lineID + 6;
  ID l_ca8  = m_lineID + 7;

  //! Lines joining faces
  ID l_l1   = m_lineID + 8;
  ID l_l2   = m_lineID + 9;
  ID l_l3   = m_lineID + 10;
  ID l_l4   = m_lineID + 11;

  //! Line loop IDs
  ID l_cll1 = m_lineLoopID;
  ID l_cll2 = m_lineLoopID + 1;
  ID l_cll3 = m_lineLoopID + 2;
  ID l_cll4 = m_lineLoopID + 3;
  ID l_cll5 = m_lineLoopID + 4;
  ID l_cll6 = m_lineLoopID + 5;

  //! Populate surface map
  std::vector< ID > l_vec{ l_cll1, l_cll2, l_cll3, l_cll4, l_cll5, l_cll6 };
  m_surfMap[m_surfaceLoopID++] = l_vec;

  //! Cylinder axis (always toward +ve x-axis)
  geo::Vector l_cylAxis( 1.0, 0.0, 0.0 );

  //! Points on cylinder
  geo::Vector l_cP1, l_cP2, l_cP3, l_cP4, l_cP5,
              l_cP6, l_cP7, l_cP8, l_cP9, l_cP10, l_rAxis;

  geo::Cylinder l_cyl;
  ID l_count = 0;
  real l_rad = 100.0, l_len = 1000.0;

  //! Seed the Gaussian random generator
  std::default_random_engine l_generator( m_seed );

  real l_meanRad = (i_mat->m_radMean ? i_mat->m_radMean :
                                    ((i_mat->m_radMin + i_mat->m_radMax) / 2.0));
  real l_meanLen = (i_mat->m_lenMean ? i_mat->m_lenMean :
                                    ((i_mat->m_lenMin + i_mat->m_lenMax) / 2.0));

  std::normal_distribution< real > l_distribRad( l_meanRad, i_mat->m_radStdDev );
  std::normal_distribution< real > l_distribLen( l_meanLen, i_mat->m_lenStdDev );

  //! Repeat till cylinder lies inside bounding box and is free of collisions
  do {
    //! Randomize radius
    switch( i_mat->m_radDistrib ) {
      case geo::Distrib::GAUSSIAN:
        l_rad = l_distribRad( l_generator );
        break;
      case geo::Distrib::UNIFORM:
        l_rad = randomizer< real >( i_mat->m_radMin, i_mat->m_radMax );
        break;
    }

    //! Randomize length
    switch( i_mat->m_lenDistrib ) {
      case geo::Distrib::GAUSSIAN:
        l_len = l_distribLen( l_generator );
        break;
      case geo::Distrib::UNIFORM:
        l_len = randomizer< real >( i_mat->m_lenMin, i_mat->m_lenMax );
        break;
    }

    //! Originally, cylinder is at center to ease rotation
    l_cP1   = geo::Vector( 0.0,    0.0,    0.0   );
    l_cP2   = geo::Vector( 0.0,   -l_rad,  0.0   );
    l_cP3   = geo::Vector( 0.0,    l_rad,  0.0   );
    l_cP4   = geo::Vector( 0.0,    0.0,   -l_rad );
    l_cP5   = geo::Vector( 0.0,    0.0,    l_rad );
    l_cP6   = geo::Vector( l_len,  0.0,    0.0   );
    l_cP7   = geo::Vector( l_len, -l_rad,  0.0   );
    l_cP8   = geo::Vector( l_len,  l_rad,  0.0   );
    l_cP9   = geo::Vector( l_len,  0.0,   -l_rad );
    l_cP10  = geo::Vector( l_len,  0.0,    l_rad );

    //! (Random) Axis to be rotated to
    l_rAxis = geo::Vector( randomizer< real >( -1.0, 1.0 ),
                           randomizer< real >( -1.0, 1.0 ),
                           randomizer< real >( -1.0, 1.0 ) );

    //! Get rotation matrix
    geo::Matrix l_rmat = geo::getRotMat( l_cylAxis, l_rAxis );

    //! Random translations
    geo::Vector l_cB( randomizer< real >( 0.0, m_length ),
                      randomizer< real >( 0.0, m_width  ),
                      randomizer< real >( 0.0, m_height ) );

    //! New points
    l_cP1   = geo::dot( l_rmat, l_cP1  ) + l_cB;
    l_cP2   = geo::dot( l_rmat, l_cP2  ) + l_cB;
    l_cP3   = geo::dot( l_rmat, l_cP3  ) + l_cB;
    l_cP4   = geo::dot( l_rmat, l_cP4  ) + l_cB;
    l_cP5   = geo::dot( l_rmat, l_cP5  ) + l_cB;
    l_cP6   = geo::dot( l_rmat, l_cP6  ) + l_cB;
    l_cP7   = geo::dot( l_rmat, l_cP7  ) + l_cB;
    l_cP8   = geo::dot( l_rmat, l_cP8  ) + l_cB;
    l_cP9   = geo::dot( l_rmat, l_cP9  ) + l_cB;
    l_cP10  = geo::dot( l_rmat, l_cP10 ) + l_cB;

    l_cyl = geo::Cylinder( l_cP1, l_rAxis, l_rad, l_len );

    //! Infinite loop breaker
    if( l_count++ >= ITERLIM ) {
      std::cerr << "Reached limit for iterative cylinder insertion! Exiting..\n";
      m_out.close();
      m_mat.close();
      exit( EXIT_FAILURE );
    }
  } while( outOfBounds( { l_cP1, l_cP2, l_cP3, l_cP4, l_cP5,
                          l_cP6, l_cP7, l_cP8, l_cP9, l_cP10 } ) ||
           collisionDetection( l_cyl ) );

  //! Store newly inserted cylinder info (for collision detection)
  m_cylList.push_back( l_cyl );

  //! Write out control points to mat file
  writeControlPoints( l_rad, { l_cP1, l_cP6 } );

  //! Points
  writePoint( l_cP1,  i_mat->m_meshSize );
  writePoint( l_cP2,  i_mat->m_meshSize );
  writePoint( l_cP3,  i_mat->m_meshSize );
  writePoint( l_cP4,  i_mat->m_meshSize );
  writePoint( l_cP5,  i_mat->m_meshSize );
  writePoint( l_cP6,  i_mat->m_meshSize );
  writePoint( l_cP7,  i_mat->m_meshSize );
  writePoint( l_cP8,  i_mat->m_meshSize );
  writePoint( l_cP9,  i_mat->m_meshSize );
  writePoint( l_cP10, i_mat->m_meshSize );

  m_out << std::endl;

  //! Circle arcs - face 1
  writeCircle( std::make_tuple( l_cp3, l_cpC1, l_cp1 ) );
  writeCircle( std::make_tuple( l_cp1, l_cpC1, l_cp4 ) );
  writeCircle( std::make_tuple( l_cp4, l_cpC1, l_cp2 ) );
  writeCircle( std::make_tuple( l_cp2, l_cpC1, l_cp3 ) );

  //! face 2
  writeCircle( std::make_tuple( l_cp5, l_cpC2, l_cp7 ) );
  writeCircle( std::make_tuple( l_cp7, l_cpC2, l_cp6 ) );
  writeCircle( std::make_tuple( l_cp6, l_cpC2, l_cp8 ) );
  writeCircle( std::make_tuple( l_cp8, l_cpC2, l_cp5 ) );

  m_out << std::endl;

  //! Lines
  writeLine( std::make_pair( l_cp1, l_cp5 ) );
  writeLine( std::make_pair( l_cp2, l_cp6 ) );
  writeLine( std::make_pair( l_cp3, l_cp7 ) );
  writeLine( std::make_pair( l_cp4, l_cp8 ) );

  m_out << std::endl;

  //! Arc line loops
  writeLineLoop( {  l_ca1, l_ca2,  l_ca3,  l_ca4 } );
  writeLineLoop( {  l_ca5, l_ca6,  l_ca7,  l_ca8 } );
  writeLineLoop( { -l_ca1, l_l3,  -l_ca5, -l_l1  } );
  writeLineLoop( {  l_l1, -l_ca8, -l_l4,  -l_ca2 } );
  writeLineLoop( {  l_l4, -l_ca7, -l_l2,  -l_ca3 } );
  writeLineLoop( {  l_l2, -l_ca6, -l_l3,  -l_ca4 } );

  m_out << std::endl;

  //! Plane surface - faces
  writePlaneSurface( l_cll1 );
  writePlaneSurface( l_cll2 );

  m_out << std::endl;

  //! Surface fillings - along length
  writeSurface( l_cll3 );
  writeSurface( l_cll4 );
  writeSurface( l_cll5 );
  writeSurface( l_cll6 );

  m_out << std::endl;
}

//! ----------------------------------------------------------------------------
//! Write spherical particle to geo script
//! ----------------------------------------------------------------------------
void geo::Writer::writeSphere( const geo::Material *i_mat ) {
  m_out << "//! Sphere\n";

  //! Sphere control points
  ID l_cpC  = m_pointID;       //! Center
  ID l_cp1  = m_pointID + 1;   //! Left   (-y)
  ID l_cp2  = m_pointID + 2;   //! Right  (+y)
  ID l_cp3  = m_pointID + 3;   //! Bottom (-z)
  ID l_cp4  = m_pointID + 4;   //! Top    (+z)
  ID l_cp5  = m_pointID + 5;   //! Rear   (-x)
  ID l_cp6  = m_pointID + 6;   //! Front  (+x)

  //! Circle arc IDs
  ID l_ca1  = m_lineID;
  ID l_ca2  = m_lineID + 1;
  ID l_ca3  = m_lineID + 2;
  ID l_ca4  = m_lineID + 3;
  ID l_ca5  = m_lineID + 4;
  ID l_ca6  = m_lineID + 5;
  ID l_ca7  = m_lineID + 6;
  ID l_ca8  = m_lineID + 7;
  ID l_ca9  = m_lineID + 8;
  ID l_ca10 = m_lineID + 9;
  ID l_ca11 = m_lineID + 10;
  ID l_ca12 = m_lineID + 11;

  //! Line loop IDs
  ID l_sll1 = m_lineLoopID;
  ID l_sll2 = m_lineLoopID + 1;
  ID l_sll3 = m_lineLoopID + 2;
  ID l_sll4 = m_lineLoopID + 3;
  ID l_sll5 = m_lineLoopID + 4;
  ID l_sll6 = m_lineLoopID + 5;
  ID l_sll7 = m_lineLoopID + 6;
  ID l_sll8 = m_lineLoopID + 7;

  //! Populate surface map
  std::vector< ID > l_vec{ l_sll1, l_sll2, l_sll3, l_sll4,
                           l_sll5, l_sll6, l_sll7, l_sll8 };
  m_surfMap[m_surfaceLoopID++] = l_vec;

  real l_cX, l_cY, l_cZ,l_rad = 300.0;
  geo::Sphere l_sph;
  ID l_count = 0;

  //! Seed the Gaussian random generator
  std::default_random_engine l_generator( m_seed );

  real l_meanRad = (i_mat->m_radMean ? i_mat->m_radMean :
                                    ((i_mat->m_radMin + i_mat->m_radMax) / 2.0));
  std::normal_distribution< real > l_distrib( l_meanRad, i_mat->m_radStdDev );

  //! Repeat till sphere doesn't collide with any other particle
  do {
    //! Randomize radius
    switch( i_mat->m_radDistrib ) {
      case geo::Distrib::GAUSSIAN:
        l_rad = l_distrib( l_generator );
        break;
      case geo::Distrib::UNIFORM:
        l_rad = randomizer< real >( i_mat->m_radMin, i_mat->m_radMax );
        break;
    }

    //! Randomize center
    l_cX = randomizer< real >( m_tolPartBound + l_rad,
                               m_length - l_rad - m_tolPartBound );
    l_cY = randomizer< real >( m_tolPartBound + l_rad,
                               m_width  - l_rad - m_tolPartBound );
    l_cZ = randomizer< real >( m_tolPartBound + l_rad,
                               m_height - l_rad - m_tolPartBound - m_pistonThicc );

    l_sph = geo::Sphere( geo::Vector( l_cX, l_cY, l_cZ ), l_rad );

    //! Infinite loop breaker
    if( l_count++ > ITERLIM ) {
      std::cerr << "Reached limit for iterative sphere insertion! Exiting..\n";
      m_out.close();
      m_mat.close();
      exit( EXIT_FAILURE );
    }
  } while( collisionDetection( l_sph ) );

  //! Store newly inserted sphere info (for collision detection)
  m_sphList.push_back( l_sph );

  //! Write out control points to mat file
  writeControlPoints( l_rad, { geo::Vector( l_cX, l_cY, l_cZ ) } );

  //! Points
  writePoint( geo::Vector( l_cX, l_cY, l_cZ ),         i_mat->m_meshSize );
  writePoint( geo::Vector( l_cX - l_rad, l_cY, l_cZ ), i_mat->m_meshSize );
  writePoint( geo::Vector( l_cX + l_rad, l_cY, l_cZ ), i_mat->m_meshSize );
  writePoint( geo::Vector( l_cX, l_cY - l_rad, l_cZ ), i_mat->m_meshSize );
  writePoint( geo::Vector( l_cX, l_cY + l_rad, l_cZ ), i_mat->m_meshSize );
  writePoint( geo::Vector( l_cX, l_cY, l_cZ - l_rad ), i_mat->m_meshSize );
  writePoint( geo::Vector( l_cX, l_cY, l_cZ + l_rad ), i_mat->m_meshSize );

  m_out << std::endl;

  //! Circle arcs
  writeCircle( std::make_tuple( l_cp1, l_cpC, l_cp3 ) );
  writeCircle( std::make_tuple( l_cp3, l_cpC, l_cp2 ) );
  writeCircle( std::make_tuple( l_cp2, l_cpC, l_cp4 ) );
  writeCircle( std::make_tuple( l_cp4, l_cpC, l_cp1 ) );
  writeCircle( std::make_tuple( l_cp3, l_cpC, l_cp6 ) );
  writeCircle( std::make_tuple( l_cp6, l_cpC, l_cp4 ) );
  writeCircle( std::make_tuple( l_cp4, l_cpC, l_cp5 ) );
  writeCircle( std::make_tuple( l_cp5, l_cpC, l_cp3 ) );
  writeCircle( std::make_tuple( l_cp1, l_cpC, l_cp6 ) );
  writeCircle( std::make_tuple( l_cp6, l_cpC, l_cp2 ) );
  writeCircle( std::make_tuple( l_cp2, l_cpC, l_cp5 ) );
  writeCircle( std::make_tuple( l_cp5, l_cpC, l_cp1 ) );

  m_out << std::endl;

  //! Arc line loops
  writeLineLoop( {  l_ca1,   l_ca5,  -l_ca9  } );
  writeLineLoop( {  l_ca2,  -l_ca10, -l_ca5  } );
  writeLineLoop( {  l_ca10,  l_ca3,  -l_ca6  } );
  writeLineLoop( {  l_ca9,   l_ca6,   l_ca4  } );
  writeLineLoop( { -l_ca2,  -l_ca8,  -l_ca11 } );
  writeLineLoop( {  l_ca8,  -l_ca1,  -l_ca12 } );
  writeLineLoop( {  l_ca12, -l_ca4,   l_ca7  } );
  writeLineLoop( {  l_ca11, -l_ca7,  -l_ca3  } );

  m_out << std::endl;

  //! Surface fillings
  writeSurface( l_sll1 );
  writeSurface( l_sll2 );
  writeSurface( l_sll3 );
  writeSurface( l_sll4 );
  writeSurface( l_sll5 );
  writeSurface( l_sll6 );
  writeSurface( l_sll7 );
  writeSurface( l_sll8 );

  m_out << std::endl;
}

//! ----------------------------------------------------------------------------
//! Write materials
//! ----------------------------------------------------------------------------
void geo::Writer::writeMaterials() {
  //! Total matrix volume
  real l_totVol = m_length * m_width * (m_height - m_pistonThicc);

  //! Write material info
  std::vector< geo::Material * >::const_iterator l_it;
  for( l_it = m_matList.begin(); l_it != m_matList.end(); ++l_it ) {
    //! Material pointer
    geo::Material *l_mat = *l_it;
    m_mat << l_mat->m_name << std::endl;

    //! Assign material mesh size to global mesh size if not specified by user
    if( !l_mat->m_meshSize )
      l_mat->m_meshSize = m_meshSize;

    if( !l_mat->m_volFrac && !l_mat->m_count ) {
      std::cerr << "Did not specify volume fraction or count for "
                << l_mat->m_name << "! Exiting..\n";
      m_out.close();
      m_mat.close();
      exit( EXIT_FAILURE );
    }

    if( l_mat->m_volFrac && l_mat->m_count ) {
      std::cerr << "Cannot specify both volume fraction and count for "
                << l_mat->m_name << "! Exiting..\n";
      m_out.close();
      m_mat.close();
      exit( EXIT_FAILURE );
    }

    switch( l_mat->m_morph ) {
      case geo::Morph::CYLINDER: {
        real l_r = (l_mat->m_radMean ? l_mat->m_radMean :
                                    ((l_mat->m_radMin + l_mat->m_radMax) / 2.0));
        real l_l = (l_mat->m_lenMean ? l_mat->m_lenMean :
                                    ((l_mat->m_lenMin + l_mat->m_lenMax) / 2.0));

        ID l_cylCount = 0;
        if( l_mat->m_volFrac )
          l_cylCount = (ID) ((l_mat->m_volFrac * l_totVol) /
                             (M_PI * std::pow( l_r, 2.0 ) * l_l));
        else
          l_cylCount = l_mat->m_count;

        //! For Gaussian distribution, we need both mean and standard deviation
        if( l_mat->m_radMean && !l_mat->m_radStdDev ) {
          std::cerr << "Did not specify standard deviation (radius) for "
                    << l_mat->m_name << "! Exiting..\n";
          m_out.close();
          m_mat.close();
          exit( EXIT_FAILURE);
        }

        //! For Gaussian distribution, we need both mean and standard deviation
        if( l_mat->m_lenMean && !l_mat->m_lenStdDev ) {
          std::cerr << "Did not specify standard deviation (length) for "
                    << l_mat->m_name << "! Exiting..\n";
          m_out.close();
          m_mat.close();
          exit( EXIT_FAILURE);
        }

        //! Assign rad std dev if not specified by user (needs min and max tho)
        if( !l_mat->m_radStdDev && l_mat->m_radMin && l_mat->m_radMax )
          l_mat->m_radStdDev = (l_mat->m_radMax - l_mat->m_radMin) / 6.0;

        //! Assign len std dev if not specified by user (needs min and max tho)
        if( !l_mat->m_lenStdDev && l_mat->m_lenMin && l_mat->m_lenMax )
          l_mat->m_lenStdDev = (l_mat->m_lenMax - l_mat->m_lenMin) / 6.0;

        std::cout << l_mat->m_name << ": " << l_cylCount << " cyl" << std::endl;
        //! Write no. of cylinders to mat file
        m_mat << "cyl\n" << l_cylCount << std::endl;

        for( ID l_i = 0; l_i < l_cylCount; l_i++ )
          writeCylinder( l_mat );

        break;
      }

      case geo::Morph::SPHERE: {
        real l_r = (l_mat->m_radMean ? l_mat->m_radMean :
                                    ((l_mat->m_radMin + l_mat->m_radMax) / 2.0));

        ID l_sphCount = 0;
        if( l_mat->m_volFrac )
          l_sphCount = (ID) ((l_mat->m_volFrac * l_totVol) /
                             ((4.0 / 3.0) * M_PI * std::pow( l_r, 3.0 )));
        else
          l_sphCount = l_mat->m_count;

        //! For Gaussian distribution, we need both mean and standard deviation
        if( l_mat->m_radMean && !l_mat->m_radStdDev ) {
          std::cerr << "Did not specify standard deviation (radius) for "
                    << l_mat->m_name << "! Exiting..\n";
          m_out.close();
          m_mat.close();
          exit( EXIT_FAILURE);
        }

        //! Assign rad std dev if not specified by user (needs min and max tho)
        if( !l_mat->m_radStdDev && l_mat->m_radMin && l_mat->m_radMax )
          l_mat->m_radStdDev = (l_mat->m_radMax - l_mat->m_radMin) / 6.0;

        std::cout << l_mat->m_name << ": " << l_sphCount << " sph" << std::endl;
        //! Write no. of spheres to mat file
        m_mat << "sph\n" << l_sphCount << std::endl;

        for( ID l_i = 0; l_i < l_sphCount; l_i++ )
          writeSphere( l_mat );

        break;
      }
    }
  }
}

//! ----------------------------------------------------------------------------
//! Write footer information (surface loops and volumes) to geo script
//! ----------------------------------------------------------------------------
void geo::Writer::writeFooter() {
  m_out << "//! ------------------------------------------------------------\n";

  //! Write surface loops info
  writeSurfaceLoops();

  m_out << std::endl;

  //! Write volumes info
  writeVolumes();
}

//! ----------------------------------------------------------------------------
//! Checks if value in config entry is empty
//! ----------------------------------------------------------------------------
void geo::Writer::chkEmpty( const std::string &i_name,
                            const std::string &i_val,
                            const std::string &i_mat ) {
  if( i_val.empty() ) {
    std::cerr << "No value found for " << i_name
              << (!i_mat.empty() ? " in " : "")
              << (!i_mat.empty() ? i_mat : "" ) << "! Exiting..\n";
    m_out.close();
    m_mat.close();
    exit( EXIT_FAILURE );
  }
}

//! ----------------------------------------------------------------------------
//! Config file parser
//! ----------------------------------------------------------------------------
void geo::Writer::parseConfigFile( const char *i_filename ) {
  std::ifstream l_confFn( i_filename, std::ios::in );
  if( !l_confFn.is_open() ) {
    std::cerr << "Cannot open " << i_filename << "! Exiting..\n";
    m_out.close();
    m_mat.close();
    exit( EXIT_FAILURE );
  }

  std::string l_lineBuf;
  geo::Material *l_mat;

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
    //if( l_varValue.empty() )
      //continue;

    //! Box
    if( l_varName == "length" ) {
      chkEmpty( l_varName, l_varValue );
      m_length          = StrToReal( l_varValue );
    }
    else if( l_varName == "width" ) {
      chkEmpty( l_varName, l_varValue );
      m_width           = StrToReal( l_varValue );
    }
    else if( l_varName == "height" ) {
      chkEmpty( l_varName, l_varValue );
      m_height          = StrToReal( l_varValue );
    }
    else if( l_varName == "global_mesh_size" ) {
      chkEmpty( l_varName, l_varValue );
      m_meshSize        = StrToReal( l_varValue );
    }

    //! Tolerance values
    else if( l_varName == "tol_particles" ) {
      if( l_varValue.empty() )
        continue;
      m_tolParticles    = StrToReal( l_varValue );
    }
    else if( l_varName == "tol_particles_boundaries" ) {
      if( l_varValue.empty() )
        continue;
      m_tolPartBound    = StrToReal( l_varValue );
    }

    //! Random seed
    else if( l_varName == "rand_seed" ) {
      if( l_varValue.empty() )
        continue;
      m_seed            = StrToID( l_varValue );
    }

    //! Piston thickness
    else if( l_varName == "piston_thicc" ) {
      if( l_varValue.empty() )
        continue;
      m_pistonThicc     = StrToID( l_varValue );
    }

    //! Material
    else if( l_varName == "material" ) {
      chkEmpty( l_varName, l_varValue );
      l_mat = new geo::Material();
      m_matList.push_back( l_mat );
      l_mat->m_name     = l_varValue;
    }
    else if( l_varName == "vol_frac" ) {
      //chkEmpty( l_varName, l_varValue, l_mat->m_name );
      if( l_varValue.empty() )
        continue;
      l_mat->m_volFrac  = StrToReal( l_varValue );
    }
    else if( l_varName == "count" ) {
      if( l_varValue.empty() )
        continue;
      l_mat->m_count    = StrToID( l_varValue );
    }
    else if( l_varName == "mesh_size" ) {
      if( l_varValue.empty() )
        continue;
      l_mat->m_meshSize = StrToReal( l_varValue );
    }
    else if( l_varName == "morph" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      if( (l_varValue == "cylinder") || (l_varValue == "cyl") )
        l_mat->m_morph  = geo::Morph::CYLINDER;
      else if( (l_varValue == "sphere") || (l_varValue == "sph") )
        l_mat->m_morph  = geo::Morph::SPHERE;
      else {
        std::cerr << "Unknown morphology (" << l_varValue << ")! Exiting..\n";
        m_out.close();
        m_mat.close();
        exit( EXIT_FAILURE);
      }
    }

    else if( l_varName == "rad_distrib" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      if( l_varValue == "gaussian" || l_varValue == "gauss" )
        l_mat->m_radDistrib = geo::Distrib::GAUSSIAN;
      else if( l_varValue == "uniform" || l_varValue == "flat" )
        l_mat->m_radDistrib = geo::Distrib::UNIFORM;
      else {
        std::cerr << "Unknown distribution (" << l_varValue << ")! Exiting..\n";
        m_out.close();
        m_mat.close();
        exit( EXIT_FAILURE);
      }
    }
    else if( l_varName == "rad_mean" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      if( l_mat->m_radDistrib == geo::Distrib::GAUSSIAN )
        l_mat->m_radMean  = StrToReal( l_varValue );
      else {
        std::cerr << "Cannot use rad_mean with Uniform distribution!\n"
                  << "Please specify rad_min & rad_max OR use Gaussian distribution. Exiting..\n";
        m_out.close();
        m_mat.close();
        exit( EXIT_FAILURE );
      }
    }
    else if( l_varName == "rad_min" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      l_mat->m_radMin     = StrToReal( l_varValue );
    }
    else if( l_varName == "rad_max" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      l_mat->m_radMax     = StrToReal( l_varValue );
    }
    else if( l_varName == "rad_std_dev" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      l_mat->m_radStdDev  = StrToReal( l_varValue );
    }

    else if( l_varName == "len_distrib" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      if( l_mat->m_morph == geo::Morph::SPHERE ) {
        std::cerr << "Cannot use len specs with sphere morphology! Exiting..\n";
        m_out.close();
        m_mat.close();
        exit( EXIT_FAILURE );
      }

      if( l_varValue == "gaussian" || l_varValue == "gauss" )
        l_mat->m_lenDistrib  = geo::Distrib::GAUSSIAN;
      else if( l_varValue == "uniform" || l_varValue == "flat" )
        l_mat->m_lenDistrib  = geo::Distrib::UNIFORM;
      else {
        std::cerr << "Unknown distribution (" << l_varValue << ")! Exiting..\n";
        m_out.close();
        m_mat.close();
        exit( EXIT_FAILURE);
      }
    }
    else if( l_varName == "len_mean" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      if( l_mat->m_morph == geo::Morph::SPHERE ) {
        std::cerr << "Cannot use len specs with sphere morphology! Exiting..\n";
        m_out.close();
        m_mat.close();
        exit( EXIT_FAILURE );
      }

      if( l_mat->m_lenDistrib == geo::Distrib::GAUSSIAN )
        l_mat->m_lenMean = StrToReal( l_varValue );
      else {
        std::cerr << "Cannot use len_mean with Uniform distribution!\n"
                  << "Please specify len_min & len_max OR use Gaussian distribution. Exiting..\n";
        m_out.close();
        m_mat.close();
        exit( EXIT_FAILURE );
      }
    }
    else if( l_varName == "len_min" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      l_mat->m_lenMin     = StrToReal( l_varValue );
    }
    else if( l_varName == "len_max" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      l_mat->m_lenMax     = StrToReal( l_varValue );
    }
    else if( l_varName == "len_std_dev" ) {
      chkEmpty( l_varName, l_varValue, l_mat->m_name );
      l_mat->m_lenStdDev  = StrToReal( l_varValue );
    }

    else
      std::cerr << "Unknown setting (" << l_varName << "). Ignored.\n";
  }

  l_confFn.close();
}

//! ----------------------------------------------------------------------------
//! Write to geo file
//! ----------------------------------------------------------------------------
void geo::Writer::writeGeo() {
  //! Write header
  writeHeader();

  //! Write brake pad bounding box info along with piston
  writeBoxAndPiston();

  //! Write materials
  writeMaterials();

  //! Write footer
  writeFooter();
}
