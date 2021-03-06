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
 * Core data structures and functions of Geo namespace.
 **/

#ifndef GEO_HPP
#define GEO_HPP

#include <iostream>
#include <cmath>

#include "GeoConstants.h"

//! ----------------------------------------------------------------------------
//! Namespace: geo
//! ----------------------------------------------------------------------------
namespace geo {
  enum class Morph {
    CYLINDER,
    SPHERE
  };

  struct Vector;
  struct Matrix;

  struct Cylinder;
  struct Sphere;

  extern real norm( const Vector &i_vec );

  extern real dist( const Vector &i_vec1,
                    const Vector &i_vec2 );

  extern real dot( const Vector &i_vec1,
                   const Vector &i_vec2 );
  extern Vector dot( const Matrix &i_mat,
                     const Vector &i_vec );

  extern Vector cross( const Vector &i_vec1,
                       const Vector &i_vec2 );
  extern Vector unitcross( const Vector &i_vec1,
                           const Vector &i_vec2 );

  extern Matrix getRotMat( const Vector &i_a1,
                           const Vector &i_a2 );

  extern real getInRadius( const Vector &i_A,
                           const Vector &i_B,
                           const Vector &i_C,
                           const Vector &i_D );

  extern real getCircumRadius( const Vector &i_A,
                               const Vector &i_B,
                               const Vector &i_C,
                               const Vector &i_D );
}

//! --------------------------------------------------------------------------
//! Vector data-structure
//! --------------------------------------------------------------------------
struct geo::Vector {
  real m_x, m_y, m_z;

  //! Default constructor
  Vector() : m_x(0.0), m_y(0.0), m_z(0.0) {}

  //! Construct vector from explicit values
  Vector( const real &i_x,
          const real &i_y,
          const real &i_z ) : m_x(i_x), m_y(i_y), m_z(i_z) {}

  //! Copy constructor
  Vector( const Vector &i_vec ) : m_x(i_vec.m_x),
                                  m_y(i_vec.m_y),
                                  m_z(i_vec.m_z) {}

  //! Construct vector from 2 points
  Vector( const Vector &i_point1,
          const Vector &i_point2 ) : m_x(i_point2.m_x - i_point1.m_x),
                                     m_y(i_point2.m_y - i_point1.m_y),
                                     m_z(i_point2.m_z - i_point1.m_z) {}

  //! Assignment operation
  Vector & operator = ( const Vector &i_vec ) {
    this->m_x = i_vec.m_x;
    this->m_y = i_vec.m_y;
    this->m_z = i_vec.m_z;

    return *this;
  }

  //! Equality operation
  bool operator == ( const Vector &i_vec ) const {
    return ((m_x == i_vec.m_x) && (m_y == i_vec.m_y) && (m_z == i_vec.m_z));
  }

  //! Addition operation
  Vector & operator + ( const Vector &i_vec ) {
    this->m_x += i_vec.m_x;
    this->m_y += i_vec.m_y;
    this->m_z += i_vec.m_z;

    return *this;
  }

  //! Division operation (by a real value)
  Vector & operator / ( const real &i_val ) {
    this->m_x /= i_val;
    this->m_y /= i_val;
    this->m_z /= i_val;

    return *this;
  }
};

//! --------------------------------------------------------------------------
//! Matrix data-structure
//! --------------------------------------------------------------------------
struct geo::Matrix {
  Vector m_row1, m_row2, m_row3;

  Matrix( const Vector &i_vec1,
          const Vector &i_vec2,
          const Vector &i_vec3 ) : m_row1(i_vec1),
                                   m_row2(i_vec2),
                                   m_row3(i_vec3) {}
};

//! --------------------------------------------------------------------------
//! Cylinder data-structure
//! --------------------------------------------------------------------------
struct geo::Cylinder {
  geo::Vector m_center;
  geo::Vector m_axis;
  real        m_radius;
  real        m_length;

  Cylinder() : m_center(), m_axis(), m_radius(0.0), m_length(0.0) {}

  Cylinder( const geo::Vector &i_center,
            const geo::Vector &i_axis,
            const real        &i_radius,
            const real        &i_length ) : m_center(i_center),
                                            m_axis(i_axis),
                                            m_radius(i_radius),
                                            m_length(i_length) {}

  Cylinder & operator = ( const Cylinder &i_cyl ) {
    this->m_center  = i_cyl.m_center;
    this->m_axis    = i_cyl.m_axis;
    this->m_radius  = i_cyl.m_radius;
    this->m_length  = i_cyl.m_length;

    return *this;
  }
};

//! --------------------------------------------------------------------------
//! Sphere data-structure
//! --------------------------------------------------------------------------
struct geo::Sphere {
  geo::Vector m_center;
  real        m_radius;

  Sphere() : m_center(), m_radius(0.0) {}

  Sphere( const geo::Vector &i_center,
          const real        &i_radius ) : m_center(i_center),
                                          m_radius(i_radius) {}

  Sphere & operator = ( const Sphere &i_sph ) {
    this->m_center = i_sph.m_center;
    this->m_radius = i_sph.m_radius;

    return *this;
  }
};

#endif