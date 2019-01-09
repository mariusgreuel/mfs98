/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     3dmath.h, vector and matrix operations

 Version:   1.00

 Date:      9-Jan-97

 Author:    MG

\***************************************************************************/

#ifndef __3DMATH_H_INCLUDED__
#define __3DMATH_H_INCLUDED__

#include <math.h>
#include "3dtypes.h"

#if defined( __MSVC__ )
#pragma warning( disable : 4244 )
#endif

#define _PI         3.1415926536
#define _2PI        (2*_PI)

#define X 0
#define Y 1
#define Z 2
#define H 3

// vector float[3]
class C3DVector {
public:
    float v[3];
public:
    C3DVector() {}
    C3DVector( float x, float y, float z ) { v[X] = x; v[Y] = y; v[Z] = z; }
    C3DVector( M3DVERTEX vec ) { v[X] = vec.x; v[Y] = vec.y; v[Z] = vec.z; }
    C3DVector( const C3DVector &v1 ) { v[X] = v1.v[X]; v[Y] = v1.v[Y]; v[Z] = v1.v[Z]; }
    C3DVector operator + () { return C3DVector( +v[X], +v[Y], +v[Z] ); }
    C3DVector operator - () { return C3DVector( -v[X], -v[Y], -v[Z] ); }
    C3DVector &operator  = ( const C3DVector &v1 ) { v[X]  = v1.v[X]; v[Y]  = v1.v[Y]; v[Z]  = v1.v[Z]; return *this; }
    C3DVector &operator += ( const C3DVector &v1 ) { v[X] += v1.v[X]; v[Y] += v1.v[Y]; v[Z] += v1.v[Z]; return *this; }
    C3DVector &operator -= ( const C3DVector &v1 ) { v[X] -= v1.v[X]; v[Y] -= v1.v[Y]; v[Z] -= v1.v[Z]; return *this; }
    C3DVector &operator *= ( const float f ) { v[X] *= f; v[Y] *= f; v[Z] *= f; return *this; }
    C3DVector &operator /= ( const float f ) { v[X] /= f; v[Y] /= f; v[Z] /= f; return *this; }
    friend C3DVector operator + ( const C3DVector &v1, const C3DVector &v2 ) { return C3DVector( v1.v[X] + v2.v[X], v1.v[Y] + v2.v[Y], v1.v[Z] + v2.v[Z] ); }
    friend C3DVector operator - ( const C3DVector &v1, const C3DVector &v2 ) { return C3DVector( v1.v[X] - v2.v[X], v1.v[Y] - v2.v[Y], v1.v[Z] - v2.v[Z] ); }
    friend C3DVector operator * ( const C3DVector &v1, float f ) { return C3DVector( v1.v[X] * f, v1.v[Y] * f, v1.v[Z] * f ); }
    friend C3DVector operator * ( float f, const C3DVector &v1 ) { return C3DVector( v1.v[X] * f, v1.v[Y] * f, v1.v[Z] * f ); }
    friend float abs( const C3DVector &v1 ) { return (float)sqrt( v1.v[X] * v1.v[X] + v1.v[Y] * v1.v[Y] + v1.v[Z] * v1.v[Z] ); }
    friend float dot( const C3DVector &v1, const C3DVector &v2 ) { return v1.v[X] * v2.v[X] + v1.v[Y] * v2.v[Y] + v1.v[Z] * v2.v[Z]; }
    friend C3DVector cross( const C3DVector &v1, const C3DVector &v2 ) { return C3DVector( v1.v[Y] * v2.v[Z] - v1.v[Z] * v2.v[Y], v1.v[Z] * v2.v[X] - v1.v[X] * v2.v[Z], v1.v[X] * v2.v[Y] - v1.v[Y] * v2.v[X] ); }
};

// homogenous vector float[4]
class C3DHVector : public C3DVector {
public:
    float h;
public:
    C3DHVector() {}
    C3DHVector( float x, float y, float z, float _h ) : C3DVector( x, y, z ) { h = _h; }
    C3DHVector( const C3DVector &v, float _h ) : C3DVector( v ) { h = _h; }
};

// matrix float[4][3]
class C3DMatrix {
public:
    C3DVector v[4];
protected:
    void RotateAbs( int c1, int c2, float _sin, float _cos );
    void RotateRel( int c1, int c2, float _sin, float _cos );
public:
    void Init();
    C3DMatrix &operator *= ( const C3DVector &vec );
    void RotateInAxe( const C3DVector &vec );
    void RotateToAxe( const C3DVector &vec );
    friend C3DMatrix operator * ( const C3DMatrix &m1, const C3DMatrix &m2 );
};

#endif // __3DMATH_H_INCLUDED__


