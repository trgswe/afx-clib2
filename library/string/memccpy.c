/*
 * $Id: string_memccpy.c,v 1.0 2021-03-29 16:54:36 clib2devs Exp $
*/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

#ifndef _STRING_HEADERS_H
#include "string_headers.h"
#endif /* _STRING_HEADERS_H */

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof (long) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (long))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < LITTLEBLOCKSIZE)

/* Macros for detecting endchar */
#if LONG_MAX == 2147483647L
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#else
#if LONG_MAX == 9223372036854775807L
/* Nonzero if X (a long int) contains a NULL byte. */
#define DETECTNULL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error long int is not a 32bit or 64bit type.
#endif
#endif

void *
memccpy(void *dst0, const void *src0, int endchar0, size_t len0)
{
    void *ptr = NULL;
    char *dst = dst0;
    const char *src = src0;
    long *aligned_dst;
    const long *aligned_src;
    int len = len0;
    char endchar = endchar0 & 0xff;

    ENTER();
    SHOWPOINTER(dst0);
    SHOWPOINTER(src0);
    SHOWVALUE(len0);

    /* If the size is small, or either SRC or DST is unaligned,
     then punt into the byte copy loop.  This should be rare.  */
    if (!TOO_SMALL(len) && !UNALIGNED(src, dst))
    {
        int i;
        unsigned long mask = 0;

        aligned_dst = (long *)dst;
        aligned_src = (long *)src;

        /* The fast code reads the ASCII one word at a time and only
         performs the bytewise search on word-sized segments if they
         contain the search character, which is detected by XORing
         the word-sized segment with a word-sized block of the search
         character and then detecting for the presence of NULL in the
         result.  */
        for (i = 0; i < LITTLEBLOCKSIZE; i++)
            mask = (mask << 8) + endchar;

        /* Copy one long word at a time if possible.  */
        while (len >= LITTLEBLOCKSIZE)
        {
            unsigned long buffer = (unsigned long)(*aligned_src);
            buffer ^= mask;
            if (DETECTNULL(buffer))
                break; /* endchar is found, go byte by byte from here */
            *aligned_dst++ = *aligned_src++;
            len -= LITTLEBLOCKSIZE;
        }

        /* Pick up any residual with a byte copier.  */
        dst = (char *)aligned_dst;
        src = (char *)aligned_src;
    }

    while (len--)
    {
        if ((*dst++ = *src++) == endchar)
        {
            ptr = dst;
            break;
        }
    }

    SHOWPOINTER(ptr);
    LEAVE();
    return ptr;
}