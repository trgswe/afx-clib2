/*
 * $Id: string_strcpy.c,v 1.4 2006-01-08 12:04:27 clib2devs Exp $
*/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

#ifndef _STRING_HEADERS_H
#include "string_headers.h"
#endif /* _STRING_HEADERS_H */

char *
strcpy(char *dest, const char *src)
{
	char *result = dest;

	assert(dest != NULL && src != NULL);

	if (dest == NULL || src == NULL)
	{
		__set_errno(EFAULT);
		goto out;
	}

	if (dest != src)
	{
		/* Make sure __global_clib2 has been created */
		if (__global_clib2 != NULL)
		{
			switch (__global_clib2->cpufamily)
			{
				case CPUFAMILY_4XX:
					result = __strcpy440(dest, src);
					break;
				default:
				{
					while (((*dest++) = (*src++)) != '\0')
						DO_NOTHING;
				}
			}
		}
		else {
			/* Fallback to standard function */
			while (((*dest++) = (*src++)) != '\0')
				DO_NOTHING;
		}
	}
out:

	return (result);
}