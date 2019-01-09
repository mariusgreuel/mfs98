/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     3dmath.cpp, vector and matrix operations

 Version:   1.00

 Date:      9-Jan-97

 Author:    MG

\***************************************************************************/

#include "stdafx.h"

#include "3dmath.h"

#define ROUNDOFF        (1e-4)      // assume zero for everything below this value

#define ROT_X           1, 2        // rotation axes
#define ROT_NX          2, 1
#define ROT_Y           0, 2
#define ROT_NY          2, 0
#define ROT_Z           0, 1
#define ROT_NZ          1, 0

/////////////////////////////////////////////////////////////////////////////
// C3DMatrix

// create unit matrix
void C3DMatrix::Init()
{
    for(int i=0; i<4; i++)
        for(int j=0; j<3; j++)
            v[i].v[j] = i == j ? 1.0 : 0.0;
}

// scale matrix by vector
C3DMatrix &C3DMatrix::operator *= (const C3DVector &vec)
{
    for(int i=0; i<4; i++)
        for(int j=0; j<3; j++)
            v[i].v[j] *= vec.v[j];
    return *this;
}

// matrix multiplication
C3DMatrix operator * (const C3DMatrix &m1, const C3DMatrix &m2)
{
    C3DMatrix m;

    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            float d = 0.0;
            for(int k=0; k<3; k++)
                d += m1.v[i].v[k] * m2.v[k].v[j];
            m.v[i].v[j] = d;
        }
    }
    for(int j=0; j<3; j++) {
        float d = 0.0;
        for(int k=0; k<3; k++)
            d += m1.v[3].v[k] * m2.v[k].v[j];
        d += m2.v[3].v[j];
        m.v[3].v[j] = d;
    }
    return m;
}

// rotate matrix in the coordinate orgin
void C3DMatrix::RotateAbs(int c1, int c2, float _sin, float _cos)
{
    for(int i=0; i<4; i++) {
        float temp = _sin * v[i].v[c1] + _cos * v[i].v[c2];
        v[i].v[c1] = _cos * v[i].v[c1] - _sin * v[i].v[c2];
        v[i].v[c2] = temp;
    }
}

// rotate matrix in its own orgin
void C3DMatrix::RotateRel(int c1, int c2, float _sin, float _cos)
{
    for(int i=0; i<3; i++) {
        float temp = _sin * v[c1].v[i] + _cos * v[c2].v[i];
        v[c1].v[i] = _cos * v[c1].v[i] - _sin * v[c2].v[i];
        v[c2].v[i] = temp;
    }
}

// rotate matrix by free axe vector
// lenght of vector is angle in radiants
void C3DMatrix::RotateInAxe(const C3DVector &vec)
{
    float a, b, sx, cx, sy, cy;

    a = vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];
    b = sqrt(a + vec.v[X] * vec.v[X]);
    if (b > ROUNDOFF) {
        a = sqrt(a);
        if (a > ROUNDOFF) {
            sx = vec.v[Y] / a;
            cx = vec.v[Z] / a;
        } else {
            sx = 0.0;
            cx = 1.0;
        }
        sy = vec.v[X] / b;
        cy = a / b;
        RotateRel(ROT_X, sx, cx);
        RotateRel(ROT_Y, sy, cy);
        RotateRel(ROT_Z, sin(b), cos(b));
        RotateRel(ROT_NY, sy, cy);
        RotateRel(ROT_NX, sx, cx);
    }
}

// rotate matrix so that it points to the vector
void C3DMatrix::RotateToAxe(const C3DVector &vec)
{
    float a, b, sx, cx, sy, cy;

    a = vec.v[X] * vec.v[X] + vec.v[Z] * vec.v[Z];
    b = sqrt(a + vec.v[Y] * vec.v[Y]);
    if (b > ROUNDOFF) {
        a = sqrt(a);
        if (a > ROUNDOFF) {
            sy = vec.v[X] / a;
            cy = vec.v[Z] / a;
        } else {
            sy = 0.0;
            cy = 1.0;
        }
        sx = vec.v[Y] / b;
        cx = a / b;
        RotateAbs(ROT_Y, sy, cy);
        RotateAbs(ROT_X, sx, cx);
    }
}


