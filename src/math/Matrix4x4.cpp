#include "Matrix4x4.hpp"

Matrix4x4::Matrix4x4()
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }
}

