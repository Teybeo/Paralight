#ifndef OPENCL_TRIGOLUT_H
#define OPENCL_TRIGOLUT_H

#include <cmath>

//#define USE_TRIGO_LOOKUP

#define M_PI           3.14159265358979323846264338327950288
#define M_PI_F (float)(3.14159265358979323846264338327950288)
#define M_INV_PI (1.f / M_PI_F)
#define TAU      (2.f * M_PI_F)

#define COS_LUT_LENGTH  (1 << 10)
#define SIN_LUT_LENGTH  (1 << 10)
#define ACOS_LUT_LENGTH (1 << 10)
#define TAN_LUT_LENGTH  (1 << 10)

#define COS_LOOKUP(n) (cos_table[(int)std::round( (((n) * M_INV_PI) + 1) * 0.5f * (COS_LUT_LENGTH - 1) ) ])
#define SIN_LOOKUP(n) (sin_table[(int)std::round( (((n) * M_INV_PI) + 1) * 0.5f * (SIN_LUT_LENGTH - 1) ) ])
#define ACOS_LOOKUP(n) (acos_table[(int)std::round( ((n) + 1) * 0.5f * (ACOS_LUT_LENGTH - 1) ) ])
#define TAN_LOOKUP(n) (tan_table[(int)std::round( (n) * M_INV_PI * (TAN_LUT_LENGTH - 1) ) ])

extern float cos_table[COS_LUT_LENGTH];
extern float sin_table[SIN_LUT_LENGTH];
extern float acos_table[ACOS_LUT_LENGTH];
extern float tan_table[TAN_LUT_LENGTH];


/*
 * Input must be in [-PI, PI]
 * Output is in [-1, 1]
 * cos[-PI] = -1
 * cos[ 0 ] =  1
 * cos[ PI] = -1
*/
inline void initializeCosTable() {
    for (int i = 0; i < COS_LUT_LENGTH; ++i) {
        float percent = float(i) / (COS_LUT_LENGTH - 1); // Map to [0, 1]
        percent = (percent * 2) - 1;                     // Map to [-1, -1]
        cos_table[i] = cosf(percent * M_PI_F);           // Map to [-PI, PI]
    }
}

/*
 * Input must be in [-PI, PI]
 * Output is in [-1, 1]
 * sin[-PI] = 1
 * sin[ 0 ] = 0
 * sin[ PI] = 1
*/
inline void initializeSinTable() {
    for (int i = 0; i < SIN_LUT_LENGTH; ++i) {
        float percent = float(i) / (SIN_LUT_LENGTH - 1); // Map to [0, 1]
        percent = (percent * 2) - 1;                     // Map to [-1, -1]
        sin_table[i] = sinf(percent * M_PI_F);           // Map to [-PI, PI]
    }
}

/*
 * Input must be in [-1, 1]
 * Output is in [0, PI]
 * acos[-1] = PI
 * acos[ 0] = PI / 2
 * acos[ 1] = 0
*/
inline void initializeAcosTable() {
    for (int i = 0; i < ACOS_LUT_LENGTH; ++i) {
        float percent = float(i) / (ACOS_LUT_LENGTH - 1); // Map to [0, 1]
        percent = (percent * 2) - 1;                      // Map to [-1, -1]
        acos_table[i] = acosf(percent);
    }
}

#if 0
// Input must be in [0, 1]
// Output is in [TAU/4, 0]
inline void initializeAcosTable() {
    for (int i = 0; i < ACOS_LUT_LENGTH; ++i) {
        float percent = float(i) / (ACOS_LUT_LENGTH - 1);
        acos_table[i] = acosf(percent);
    }
}
#endif
// Input must be in [0, PI]
// Output is in [-inf, +inf]
/*
 * Input must be in [0, TAU/2]
 * Output is in [-inf, +inf]
 * tan[0]       = 0
 * tan[TAU / 4]- = +inf
 * tan[TAU / 4]+ = -inf
 * tan[PI] = 0
*/
inline void initializeTanTable() {
    for (int i = 0; i < TAN_LUT_LENGTH; ++i) {
        float percent = float(i) / (COS_LUT_LENGTH - 1);
        tan_table[i] = tanf(percent * M_PI_F);
    }
}


#endif //OPENCL_TRIGOLUT_H
