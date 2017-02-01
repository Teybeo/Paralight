#include "TrigoLut.h"

#include <iostream>

using std::cout;
using std::endl;

float acos_table[ACOS_LUT_LENGTH];
float cos_table[COS_LUT_LENGTH];
float sin_table[SIN_LUT_LENGTH];
float tan_table[TAN_LUT_LENGTH];

void test_tables() {
    for (int a = 0; a < 10; ++a) {
        float percent = a / 10.f;
        percent = (percent * 2) - 1.f;
        float length = percent;
        cout << percent * 100 << "% of 3.14\n";
        cout << "acos[ " << length << "] = " << ACOS_LOOKUP(length) << "\n";
        cout << "acos( " << length << ") = " << acosf(length) << "\n";
        cout << "cos() of " << length << " = " << cosf(length) << "\n";
        cout << "cos[] of         = " << COS_LOOKUP(length) << "\n";
        cout << "sin() of " << length << " = " << sinf(length) << "\n";
        cout << "sin[] of         = " << SIN_LOOKUP(length) << "\n";
    }
}