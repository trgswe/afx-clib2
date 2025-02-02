/*
 * $Id: math_logbf.c,v 1.4 2022-03-13 12:04:23 clib2devs Exp $
 *

 *
 *
 * PowerPC math library based in part on work by Sun Microsystems
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 *
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

static const float
        two25 = 3.355443200e+07;		/* 0x4c000000 */

float
logbf(float x) {
    int32_t ix;
    GET_FLOAT_WORD(ix, x);
    ix &= 0x7fffffff;            /* high |x| */
    if (ix == 0) return (float) -1.0 / fabsf(x);
    if (ix >= 0x7f800000) return x * x;
    if (ix < 0x00800000) {
        x *= two25;         /* convert subnormal x to normal */
        GET_FLOAT_WORD(ix, x);
        ix &= 0x7fffffff;
        return (float) ((ix >> 23) - 127 - 25);
    } else
        return (float) ((ix >> 23) - 127);

}
