/*
 * $Id: math_llroundf.c,v 1.0 2022-03-13 12:04:23 clib2devs Exp $
*/

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/*
 * If type has more precision than dtype, the endpoints dtype_(min|max) are
 * of the form xxx.5; they are "out of range" because lround() rounds away
 * from 0.  On the other hand, if type has less precision than dtype, then
 * all values that are out of range are integral, so we might as well assume
 * that everything is in range.  At compile time, INRANGE(x) should reduce to
 * two floating-point comparisons in the former case, or TRUE otherwise.
 */
static const float dtype_min = LLONG_MIN - 0.5;
static const float dtype_max = LLONG_MAX + 0.5;
#define INRANGE(x) (dtype_max - LLONG_MAX != 0.5 || ((x) > dtype_min && (x) < dtype_max))

long long
llroundf(float x)
{
    if (INRANGE(x)) {
        x = roundf(x);
        return ((long long) x);
    } else {
        feraiseexcept(FE_INVALID);
        return (LLONG_MAX);
    }
}