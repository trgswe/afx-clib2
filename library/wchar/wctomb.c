/*
 * $Id: wchar_wctomb.c,v 1.3 2006-01-08 12:04:26 clib2devs Exp $
*/

#ifndef _WCHAR_HEADERS_H
#include "wchar_headers.h"
#endif /* _WCHAR_HEADERS_H */

#include <stdlib.h>

int
_wctomb_r(char *s, wchar_t wchar, mbstate_t *state) {
    if (strlen(__global_clib2->_current_locale) <= 1) { /* fall-through */
    } else if (!strcmp(__global_clib2->_current_locale, "C-UTF-8")) {
        if (s == NULL)
            return 0; /* UTF-8 encoding is not state-dependent */

        if (wchar <= 0x7f) {
            *s = wchar;
            return 1;
        } else if (wchar >= 0x80 && wchar <= 0x7ff) {
            *s++ = 0xc0 | ((wchar & 0x7c0) >> 6);
            *s = 0x80 | (wchar & 0x3f);
            return 2;
        } else if (wchar >= 0x800 && wchar <= 0xffff) {
            /* UTF-16 surrogates -- must not occur in normal UCS-4 data */
            if (wchar >= 0xd800 && wchar <= 0xdfff)
                return -1;

            *s++ = 0xe0 | ((wchar & 0xf000) >> 12);
            *s++ = 0x80 | ((wchar & 0xfc0) >> 6);
            *s = 0x80 | (wchar & 0x3f);
            return 3;
        } else if (wchar >= 0x10000 && wchar <= 0x1fffff) {
            *s++ = 0xf0 | ((wchar & 0x1c0000) >> 18);
            *s++ = 0x80 | ((wchar & 0x3f000) >> 12);
            *s++ = 0x80 | ((wchar & 0xfc0) >> 6);
            *s = 0x80 | (wchar & 0x3f);
            return 4;
        } else if (wchar >= 0x200000 && wchar <= 0x3ffffff) {
            *s++ = 0xf8 | ((wchar & 0x3000000) >> 24);
            *s++ = 0x80 | ((wchar & 0xfc0000) >> 18);
            *s++ = 0x80 | ((wchar & 0x3f000) >> 12);
            *s++ = 0x80 | ((wchar & 0xfc0) >> 6);
            *s = 0x80 | (wchar & 0x3f);
            return 5;
        } else if (wchar >= 0x4000000 && wchar <= 0x7fffffff) {
            *s++ = 0xfc | ((wchar & 0x40000000) >> 30);
            *s++ = 0x80 | ((wchar & 0x3f000000) >> 24);
            *s++ = 0x80 | ((wchar & 0xfc0000) >> 18);
            *s++ = 0x80 | ((wchar & 0x3f000) >> 12);
            *s++ = 0x80 | ((wchar & 0xfc0) >> 6);
            *s = 0x80 | (wchar & 0x3f);
            return 6;
        } else
            return -1;
    } else if (!strcmp(__global_clib2->_current_locale, "C-SJIS")) {
        unsigned char char2 = (unsigned char) wchar;
        unsigned char char1 = (unsigned char) (wchar >> 8);

        if (s == NULL)
            return 0; /* not state-dependent */

        if (char1 != 0x00) {
            /* first byte is non-zero..validate multi-byte char */
            if (_issjis1(char1) && _issjis2(char2)) {
                *s++ = (char) char1;
                *s = (char) char2;
                return 2;
            } else
                return -1;
        }
    } else if (!strcmp(__global_clib2->_current_locale, "C-EUCJP")) {
        unsigned char char2 = (unsigned char) wchar;
        unsigned char char1 = (unsigned char) (wchar >> 8);

        if (s == NULL)
            return 0; /* not state-dependent */

        if (char1 != 0x00) {
            /* first byte is non-zero..validate multi-byte char */
            if (_iseucjp(char1) && _iseucjp(char2)) {
                *s++ = (char) char1;
                *s = (char) char2;
                return 2;
            } else
                return -1;
        }
    } else if (!strcmp(__global_clib2->_current_locale, "C-JIS")) {
        int cnt = 0;
        unsigned char char2 = (unsigned char) wchar;
        unsigned char char1 = (unsigned char) (wchar >> 8);

        if (s == NULL)
            return 1; /* state-dependent */

        if (char1 != 0x00) {
            /* first byte is non-zero..validate multi-byte char */
            if (_isjis(char1) && _isjis(char2)) {
                if (state->__state == 0) {
                    /* must switch from ASCII to JIS state */
                    state->__state = 1;
                    *s++ = ESC_CHAR;
                    *s++ = '$';
                    *s++ = 'B';
                    cnt = 3;
                }
                *s++ = (char) char1;
                *s = (char) char2;
                return cnt + 2;
            } else
                return -1;
        } else {
            if (state->__state != 0) {
                /* must switch from JIS to ASCII state */
                state->__state = 0;
                *s++ = ESC_CHAR;
                *s++ = '(';
                *s++ = 'B';
                cnt = 3;
            }
            *s = (char) char2;
            return cnt + 1;
        }
    }

    if (s == NULL)
        return 0;

    /* otherwise we are dealing with a single byte character */
    *s = (char) wchar;
    return 1;
}

int
wctomb(char *s, wchar_t wchar) {
    if (!s)
        return 0;

    ENTER();

    size_t ret = _wctomb_r(s, wchar, NULL);
    if (ret == (size_t) - 1) {
        RETURN(-1);
        return -1;
    }

    RETURN((int) ret);
    return (int) ret;
}
