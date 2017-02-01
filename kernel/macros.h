#ifndef MACROS_H
#define MACROS_H

/* On HD4600 and -fast-relaxed-math, cos returns sin values
   so use native_cos instead which is correct */
// No effect on Nvidia with -fast-relaxed-math

#if 1
#define cos native_cos
#define sin native_sin
#define sqrt native_sqrt
#define rsqrt native_rsqrt
#define powr native_powr
#define log native_log
#define log2 native_log2
#define log10 native_log10
#define exp native_exp

#define normalize fast_normalize
#define length fast_length
#endif

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define DUMP_SIZE(type) printf("sizeof "STRINGIFY(type)" %d\n", sizeof(type));

#define DEBUG_PIXEL(x, y) if (get_global_id(0) == x && get_global_id(1) == y)
#define DEBUG_PIXEL_AND_FRAME(x, y, mod) if (get_global_id(0) == x && get_global_id(1) == y && (options->frame_number % mod == 0))

#define PRINT_VEC(msg, vec) printf(msg "(%f, %f, %f)\n", (vec).x, (vec).y, (vec).z);
#define PRINT(msg, x) printf(msg "%.9f\n", (x));
#define PRINT_I(msg, x) printf(msg "%d\n", (x));

#define CHECK_NAN(msg, X)\
        if (any(isnan(X))) \
            printf(msg " %f, %f, %f at (%d, %d)\n", (X).x, (X).y, (X).z, get_global_id(0), get_global_id(1));
//            printf("NaN at (%d, %d)\n", get_global_id(0), get_global_id(1));

#define CHECK_NAN_S(msg, X)\
        if (isnan(X)) \
            printf(msg " %f at (%d, %d)\n", (X), get_global_id(0), get_global_id(1));


#define CHECK_FPNORMAL(msg, X)\
        if (!isnormal((X).x) && (X).x != 0 && \
            !isnormal((X).y) && (X).y != 0 && \
            !isnormal((X).z) && (X).z != 0) \
            printf(msg " %f, %f, %f at (%d, %d)\n", (X).x, (X).y, (X).z, get_global_id(0), get_global_id(1));
//            printf("NaN at (%d, %d)\n", get_global_id(0), get_global_id(1));

#define CHECK_FPNORMAL_S(msg, X)\
        if (!isnormal(X) && (X) != 0) \
            printf(msg " %.9f at (%d, %d)\n", (X), get_global_id(0), get_global_id(1));

#define CHECK_FINITE(X)\
        if (!isfinite((X)))
//            printf(msg " %f, %f, %f at (%d, %d)\n", (X).x, (X).y, (X).z, get_global_id(0), get_global_id(1));
//
//#define CHECK_FINITE(msg, X)\
//        if (any(isfinite((X))))  \
//            printf(msg " %f, %f, %f at (%d, %d)\n", (X).x, (X).y, (X).z, get_global_id(0), get_global_id(1));
//
#define CHECK_ZERO(msg, X) \
        if (all(isequal((X), (float3)(0)))) \
            printf(msg " zero vec\n");

#endif
