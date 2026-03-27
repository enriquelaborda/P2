



#include <math.h>
#include "pav_analysis.h"


float compute_power(const float *x, unsigned int N) {
    float power = 1e-12;
    for (unsigned int n = 0; n < N; n++) {
        power += x[n] * x[n];
    }
    return 10*log10(power/N);
}


float compute_am(const float *x, unsigned int N) {
    float am = 0.0;
    for (unsigned int n = 0; n < N; n++) {
        am += fabs(x[n]);
    }
    return am / N;
}


float compute_zcr(const float *x, unsigned int N, float fm) {
    float zc = 0.0;
    for (unsigned int n = 1; n < N; n++) {
        if ((x[n] >= 0 && x[n-1] < 0) || (x[n] < 0 && x[n-1] >= 0)) {
            zc += 1.0;
        }
    }
    return (zc * fm) / (2.0 * (N - 1));
}



