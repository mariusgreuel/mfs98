/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     heli.cpp

 Version:   1.00

 Date:      18-Feb-97

 Author:    MG

\***************************************************************************/

#include "stdafx.h"

#include "3dmath.h"
#include "variable.h"

/////////////////////////////////////////////////////////////////////////////
// Defines...

#define ROLL            0
#define GIER            1
#define NICK            2
#define PITCH           3

#define sgn(x)        (x >= 0.0 ? 1.0 : -1.0)
#define sqr(x)        ((x) * (x))

/////////////////////////////////////////////////////////////////////////////
// Locals...

static float            MotorRPM;

static C3DVector        Force;
static C3DVector        Moment;
static C3DVector        GroundSpeed;
static C3DVector        RotateSpeed;
static C3DVector        AirSpeed;

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern float            fCalculationPeriod;
extern float            Stick[4];
extern C3DVector        WindSpeed;

extern void Disp(LPSTR fmt, ...);

/////////////////////////////////////////////////////////////////////////////
// Parameters...

extern CVarList VarList;

#pragma init_seg(user)
CVarSection VarSectionHeli(VarList, "Helicopter");
//CVarSection VarSectionHeli(VarList, "Hubschrauber");

static CFVar Sensitivity[] = {
    CFVar(VarSectionHeli, "RollSensitivity",   1.0, 0.0, 5.0),
    CFVar(VarSectionHeli, "GierSensitivity",   1.0, 0.0, 5.0),
    CFVar(VarSectionHeli, "NickSensitivity",   1.0, 0.0, 5.0),
};
static CFVar Spin[] = {
    CFVar(VarSectionHeli, "RollSpin",          2.0, 0.0, 5.0),
    CFVar(VarSectionHeli, "GierSpin",          1.0, 0.0, 5.0),
    CFVar(VarSectionHeli, "NickSpin",          2.0, 0.0, 5.0),
};
static CFVar Inertia[] = {
    CFVar(VarSectionHeli, "RollInertia",       1.0, 0.1, 5.1),
    CFVar(VarSectionHeli, "GierInertia",       0.6, 0.1, 5.1),
    CFVar(VarSectionHeli, "NickInertia",       1.0, 0.1, 5.1),
};
static CFVar TurnWithWind[] = {
    CFVar(VarSectionHeli, "RollWithWind",      0.0, 0.0, 0.5),
    CFVar(VarSectionHeli, "WeatherFane",       0.1, 0.0, 0.5),
    CFVar(VarSectionHeli, "PitchWithWind",     0.0, 0.0, 0.5),
};
static CFVar Drag[] = {
    CFVar(VarSectionHeli, "FrontDrag",         0.0, 0.0, 5.0),
    CFVar(VarSectionHeli, "TopDrag",           0.0, 0.0, 5.0),
    CFVar(VarSectionHeli, "SideDrag",          0.0, 0.0, 5.0),
};
/*static CFVar RPMSlow[] = {
    CFVar(VarSectionHeli, "RollRPMSlow",       0.0, 0.0, 1.0),
    CFVar(VarSectionHeli, "GierRPMSlow",       0.0, 0.0, 1.0),
    CFVar(VarSectionHeli, "NickRPMSlow",       0.0, 0.0, 1.0),
};*/

static CFVar
    MassInertia(       VarSectionHeli, "MassInertia",          0.2, 0.01, 1.01),
    EngineRiseRate(    VarSectionHeli, "EngineRiseRate",       1.0, 0.0, 1.0),
    EngineFallRate(    VarSectionHeli, "EngineFallRate",       1.0, 0.0, 1.0),
    MainRotorThrust(   VarSectionHeli, "MainRotorThrust",        1.0, 0.50, 3.0),
    TailRotorThrust(   VarSectionHeli, "TailRotorThrust",        0.05, -0.25, 0.25);
//    RPMLift(           VarSectionHeli, "RPMLift",              1.0, 0.0, 100.0),
//    CollectiveLift(    VarSectionHeli, "CollectiveLift",       0.0, 0.0, 100.0),
//    TranslationalLift( VarSectionHeli, "TranslationalLift",    0.0, 0.0, 100.0),
//    GroundEffectRange( VarSectionHeli, "GroundEffectRange",    1.0, 0.1, 10.0),
//    GroundEffectFactor(VarSectionHeli, "GroundEffectFactor",   0.3, 0.0, 2.0);

/////////////////////////////////////////////////////////////////////////////

void HeliInit1()
{
    MotorRPM = 0.0;
    GroundSpeed = C3DVector(0.0, 0.0, 0.0);
    RotateSpeed = C3DVector(0.0, 0.0, 0.0);
}

void HeliInit2()
{
    GroundSpeed = C3DVector(0.0, 0.0, 0.0);
}

void HeliCalc(C3DMatrix &m)
{
    float   NewMotorRPM;
    float   GroundEffect;
    int     i, j;

    AirSpeed = GroundSpeed - WindSpeed;

    NewMotorRPM = (Stick[ PITCH ] + 1.0) * 0.5;
    if (NewMotorRPM >= MotorRPM)
      MotorRPM = MotorRPM * (1.0 - EngineRiseRate) + NewMotorRPM * EngineRiseRate;
    else
      MotorRPM = MotorRPM * (1.0 - EngineFallRate) + NewMotorRPM * EngineFallRate;

    GroundEffect = 1;// + GroundEffectFactor / (1 + m.v[H].v[Y] / GroundEffectRange   );

    for(i=0; i<3; i++)
        Moment.v[i] = Stick[i] * Sensitivity[i];
    Moment.v[Y] += dot(m.v[Z], AirSpeed) * TurnWithWind[Y] * 0.5;

    Force  = m.v[Y] * MotorRPM * MainRotorThrust;
    Force += m.v[Z] * MotorRPM * TailRotorThrust;
    Force.v[Y] -= 0.5;

    Disp("MotorRPM %3.3f", MotorRPM);
    Disp("RotateSpeed.v[Y] %3.3f", RotateSpeed.v[Y]);

    for(i=0; i<3; i++) {
        Moment.v[i] -= RotateSpeed.v[i] * Spin[i];
        for(j=0; j<3; j++)
            Force.v[i] -= AirSpeed.v[i] * abs(m.v[j].v[i]) * Drag[j];
        RotateSpeed.v[i] += Moment.v[i] / Inertia[i] * fCalculationPeriod;
        GroundSpeed.v[i] += Force.v[i] / MassInertia * fCalculationPeriod;
    }

//    for(i=0; i<4; i++)
//        Disp("M%d: %2.3f, %2.3f, %2.3f", i, m.v[i].v[X], m.v[i].v[Y], m.v[i].v[Z]);

}

C3DVector &HeliGetGroundSpeed()
{
    return GroundSpeed;
}

C3DVector &HeliGetRotateSpeed()
{
    return RotateSpeed;
}


