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
 * Core math and vector calculus functions.
 **/

#include "Geo.hpp"

//! ----------------------------------------------------------------------------
//! Norm (Euclidean) of vector
//! ----------------------------------------------------------------------------
real geo::norm( const geo::Vector &i_vec ) {
  return sqrt( std::pow( i_vec.m_x, 2.0 ) +
               std::pow( i_vec.m_y, 2.0 ) +
               std::pow( i_vec.m_z, 2.0 ) );
}

//! ----------------------------------------------------------------------------
//! Distance between two vectors
//! ----------------------------------------------------------------------------
real geo::dist( const geo::Vector &i_vec1,
                const geo::Vector &i_vec2 ) {
  return sqrt( std::pow( i_vec1.m_x - i_vec2.m_x, 2.0 ) +
               std::pow( i_vec1.m_y - i_vec2.m_y, 2.0 ) +
               std::pow( i_vec1.m_z - i_vec2.m_z, 2.0 ) );
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
geo::Matrix geo::getRotMat( const geo::Vector &i_a1,
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
//! Get inradius of a tet
//! ----------------------------------------------------------------------------
real geo::getInRadius( const geo::Vector &i_A,
                       const geo::Vector &i_B,
                       const geo::Vector &i_C,
                       const geo::Vector &i_D ) {
  geo::Vector l_b( i_A, i_B );
  geo::Vector l_c( i_A, i_C );
  geo::Vector l_d( i_A, i_D );

  geo::Vector l_bc = geo::cross( l_b, l_c );
  geo::Vector l_cd = geo::cross( l_c, l_d );
  geo::Vector l_db = geo::cross( l_d, l_b );

  return (geo::dot( l_b, l_cd ) / (geo::norm( l_bc ) +
                                   geo::norm( l_cd ) +
                                   geo::norm( l_db ) +
                                   geo::norm( l_bc + l_cd + l_db )));
}

//! ----------------------------------------------------------------------------
//! Get cofactor of a matrix
//! ----------------------------------------------------------------------------
void getCofactor( real i_mat[4][4], real i_temp[4][4],
                  int i_p, int i_q, int i_n ) {
  int l_i = 0, l_j = 0;

  for( int l_row = 0; l_row < i_n; l_row++ ) {
    for( int l_col = 0; l_col < i_n; l_col++ ) {
      if( l_row != i_p && l_col != i_q ) {
        i_temp[l_i][l_j++] = i_mat[l_row][l_col];

        if( l_j == i_n - 1 ) {
          l_j = 0;
          l_i++;
        }
      }
    }
  }
}

//! ----------------------------------------------------------------------------
//! Get determinant of a 4x4 matrix
//! ----------------------------------------------------------------------------
real determinant( real i_mat[4][4], int i_n ) {
  real l_det = 0.0;

  if( i_n == 1 )
    return i_mat[0][0];

  real l_temp[4][4];
  int l_sign = 1;

  for( int l_i = 0; l_i < i_n; l_i++ ) {
    getCofactor( i_mat, l_temp, 0, l_i, i_n );
    l_det += l_sign * i_mat[0][l_i] * determinant( l_temp, i_n - 1 );
    l_sign = -l_sign;
  }

  return l_det;
}

//! ----------------------------------------------------------------------------
//! Get circumradius of a tet
//! ----------------------------------------------------------------------------
real geo::getCircumRadius( const geo::Vector &i_A,
                           const geo::Vector &i_B,
                           const geo::Vector &i_C,
                           const geo::Vector &i_D ) {
  real l_x1 = i_A.m_x, l_y1 = i_A.m_y, l_z1 = i_A.m_z,
       l_x2 = i_B.m_x, l_y2 = i_B.m_y, l_z2 = i_B.m_z,
       l_x3 = i_C.m_x, l_y3 = i_C.m_y, l_z3 = i_C.m_z,
       l_x4 = i_D.m_x, l_y4 = i_D.m_y, l_z4 = i_D.m_z;

  real l_m1[4][4] = { { l_x1*l_x1 + l_y1*l_y1 + l_z1*l_z1, l_y1, l_z1, 1.0 },
                      { l_x2*l_x2 + l_y2*l_y2 + l_z2*l_z2, l_y2, l_z2, 1.0 },
                      { l_x3*l_x3 + l_y3*l_y3 + l_z3*l_z3, l_y3, l_z3, 1.0 },
                      { l_x4*l_x4 + l_y4*l_y4 + l_z4*l_z4, l_y4, l_z4, 1.0 } };

  real l_m2[4][4] = { { l_x1*l_x1 + l_y1*l_y1 + l_z1*l_z1, l_x1, l_z1, 1.0 },
                      { l_x2*l_x2 + l_y2*l_y2 + l_z2*l_z2, l_x2, l_z2, 1.0 },
                      { l_x3*l_x3 + l_y3*l_y3 + l_z3*l_z3, l_x3, l_z3, 1.0 },
                      { l_x4*l_x4 + l_y4*l_y4 + l_z4*l_z4, l_x4, l_z4, 1.0 } };

  real l_m3[4][4] = { { l_x1*l_x1 + l_y1*l_y1 + l_z1*l_z1, l_x1, l_y1, 1.0 },
                      { l_x2*l_x2 + l_y2*l_y2 + l_z2*l_z2, l_x2, l_y2, 1.0 },
                      { l_x3*l_x3 + l_y3*l_y3 + l_z3*l_z3, l_x3, l_y3, 1.0 },
                      { l_x4*l_x4 + l_y4*l_y4 + l_z4*l_z4, l_x4, l_y4, 1.0 } };

  real l_m4[4][4] = { { l_x1, l_y1, l_z1, 1.0 },
                      { l_x2, l_y2, l_z2, 1.0 },
                      { l_x3, l_y3, l_z3, 1.0 },
                      { l_x4, l_y4, l_z4, 1.0 } };

  real l_m5[4][4] = { { l_x1*l_x1 + l_y1*l_y1 + l_z1*l_z1, l_x1, l_y1, l_z1 },
                      { l_x2*l_x2 + l_y2*l_y2 + l_z2*l_z2, l_x2, l_y2, l_z2 },
                      { l_x3*l_x3 + l_y3*l_y3 + l_z3*l_z3, l_x3, l_y3, l_z3 },
                      { l_x4*l_x4 + l_y4*l_y4 + l_z4*l_z4, l_x4, l_y4, l_z4 } };

  real l_Dx = determinant( l_m1, 4 );
  real l_Dy = determinant( l_m2, 4 );
  real l_Dz = determinant( l_m3, 4 );
  real l_a  = determinant( l_m4, 4 );
  real l_c  = determinant( l_m5, 4 );

  return (sqrt( l_Dx * l_Dx + l_Dy * l_Dy + l_Dz * l_Dz - 4.0 * l_a * l_c ) /
          (2.0 * std::abs( l_a )));
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