#include "Geo.hpp"

//! ----------------------------------------------------------------------------
//! Norm (Euclidean) of vector
//! ----------------------------------------------------------------------------
real geo::norm( const geo::Vector &i_vec ) {
  return sqrt( pow( i_vec.m_x, 2.0 ) +
               pow( i_vec.m_y, 2.0 ) +
               pow( i_vec.m_z, 2.0 ) );
}

//! ----------------------------------------------------------------------------
//! Distance between two vectors
//! ----------------------------------------------------------------------------
real geo::dist( const geo::Vector &i_vec1,
                const geo::Vector &i_vec2 ) {
  return sqrt( pow( i_vec1.m_x - i_vec2.m_x, 2.0 ) +
               pow( i_vec1.m_y - i_vec2.m_y, 2.0 ) +
               pow( i_vec1.m_z - i_vec2.m_z, 2.0 ) );
}

//! ----------------------------------------------------------------------------
//! Dot product of two vectors
//! ----------------------------------------------------------------------------
real geo::dot( const geo::Vector &i_vec1,
               const geo::Vector &i_vec2 ) {
  return (i_vec1.m_x * i_vec2.m_x +
          i_vec1.m_y * i_vec2.m_y +
          i_vec1.m_z * i_vec2.m_z);
}

//! ----------------------------------------------------------------------------
//! Dot product of matrix and vector
//! ----------------------------------------------------------------------------
geo::Vector geo::dot( const geo::Matrix &i_mat,
                      const geo::Vector &i_vec ) {
  return geo::Vector( i_mat.m_row1.m_x * i_vec.m_x +
                      i_mat.m_row1.m_y * i_vec.m_y +
                      i_mat.m_row1.m_z * i_vec.m_z,

                      i_mat.m_row2.m_x * i_vec.m_x +
                      i_mat.m_row2.m_y * i_vec.m_y +
                      i_mat.m_row2.m_z * i_vec.m_z,

                      i_mat.m_row3.m_x * i_vec.m_x +
                      i_mat.m_row3.m_y * i_vec.m_y +
                      i_mat.m_row3.m_z * i_vec.m_z );
}

//! ----------------------------------------------------------------------------
//! Cross product of two vectors
//! ----------------------------------------------------------------------------
geo::Vector geo::cross( const geo::Vector &i_vec1,
                        const geo::Vector &i_vec2 ) {
  return geo::Vector( i_vec1.m_y * i_vec2.m_z - i_vec1.m_z * i_vec2.m_y,
                      i_vec1.m_z * i_vec2.m_x - i_vec1.m_x * i_vec2.m_z,
                      i_vec1.m_x * i_vec2.m_y - i_vec1.m_y * i_vec2.m_x );
}

//! ----------------------------------------------------------------------------
//! Unit cross product of two vectors
//! ----------------------------------------------------------------------------
geo::Vector geo::unitcross( const geo::Vector &i_vec1,
                            const geo::Vector &i_vec2 ) {
  if( i_vec1 == i_vec2 )
    return geo::Vector();

  geo::Vector l_prod = geo::cross( i_vec1, i_vec2 );
  l_prod = l_prod / geo::norm( l_prod );

  return l_prod;
}

//! ----------------------------------------------------------------------------
//! Get rotation matrix
//! ----------------------------------------------------------------------------
geo::Matrix geo::getRMat( const geo::Vector &i_a1,
                          const geo::Vector &i_a2 ) {
  //! cos(theta)
  real l_c = geo::dot( i_a1, i_a2 ) / (geo::norm( i_a1 ) * geo::norm( i_a2 ));
  //! sin(theta)
  real l_s = sqrt( 1.0 - l_c * l_c );
  real l_C = 1.0 - l_c;

  //! Axis of rotation
  geo::Vector l_ax = geo::unitcross( i_a1, i_a2 );

  //! Rotation matrix
  geo::Matrix l_rmat( geo::Vector( l_ax.m_x * l_ax.m_x * l_C + l_c,
                                   l_ax.m_x * l_ax.m_y * l_C - l_ax.m_z * l_s,
                                   l_ax.m_x * l_ax.m_z * l_C + l_ax.m_y * l_s ),

                      geo::Vector( l_ax.m_y * l_ax.m_x * l_C + l_ax.m_z * l_s,
                                   l_ax.m_y * l_ax.m_y * l_C + l_c,
                                   l_ax.m_y * l_ax.m_z * l_C - l_ax.m_x * l_s ),

                      geo::Vector( l_ax.m_z * l_ax.m_x * l_C - l_ax.m_y * l_s,
                                   l_ax.m_z * l_ax.m_y * l_C + l_ax.m_x * l_s,
                                   l_ax.m_z * l_ax.m_z * l_C + l_c ) );

  return l_rmat;
}

//! ----------------------------------------------------------------------------
//! Print vector
//! ----------------------------------------------------------------------------
std::ostream & operator << (       std::ostream &o_stream,
                             const geo::Vector  &i_vec ) {
  o_stream << "(" << i_vec.m_x << "," << i_vec.m_y << "," << i_vec.m_z << ")";

  return o_stream;
}

//! ----------------------------------------------------------------------------
//! Print matrix
//! ----------------------------------------------------------------------------
std::ostream & operator << (       std::ostream &o_stream,
                             const geo::Matrix  &i_mat ) {
  o_stream << std::showpos << "\n[ "     << i_mat.m_row1.m_x << "  "
           << i_mat.m_row1.m_y << "  "   << i_mat.m_row1.m_z << "\n  "
           << i_mat.m_row2.m_x << "  "   << i_mat.m_row2.m_y << "  "
           << i_mat.m_row2.m_z << "\n  " << i_mat.m_row3.m_x << "  "
           << i_mat.m_row3.m_y << "  "   << i_mat.m_row3.m_z << " ]\n"
           << std::noshowpos;

  return o_stream;
}