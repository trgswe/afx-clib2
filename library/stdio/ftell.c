/*
 * $Id: stdio_ftell.c,v 1.10 2006-01-08 12:04:24 clib2devs Exp $
*/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

long int
ftell(FILE *stream) {
    struct iob *file = (struct iob *) stream;
    struct file_action_message fam;
    _off64_t result = ERROR;
    _off64_t position;

    assert(stream != NULL);

    flockfile(stream);

    if (stream == NULL) {
        __set_errno(EFAULT);
        goto out;
    }

    assert(__is_valid_iob(file));
    assert(FLAG_IS_SET(file->iob_Flags, IOBF_IN_USE));
    assert(file->iob_BufferSize > 0);

    if (FLAG_IS_CLEAR(file->iob_Flags, IOBF_IN_USE)) {
        SHOWMSG("this file is not even in use");

        SET_FLAG(file->iob_Flags, IOBF_ERROR);

        __set_errno(EBADF);

        goto out;
    }

    SHOWMSG("calling the hook");

    SHOWPOINTER(&fam);

    fam.fam_Action = file_action_seek;
    fam.fam_Offset = 0;
    fam.fam_Mode = SEEK_CUR;

    SHOWVALUE(fam.fam_Offset);
    SHOWVALUE(fam.fam_Mode);

    assert(file->iob_Action != NULL);

    position = (_off64_t)(*file->iob_Action)(file, &fam);
    if (position == GETPOSITION_ERROR && fam.fam_Error != OK) {
        SET_FLAG(file->iob_Flags, IOBF_ERROR);

        __set_errno(fam.fam_Error);

        goto out;
    }

    /* If this is a valid file position, clear 'errno' so that
      it cannot be mistaken for an error. */
    if (position < 0)
        __set_errno(OK);

    if (__iob_read_buffer_is_valid(file)) {
        /* Subtract the number of bytes still in the buffer which have
         * not been read before.
         */
        position -= __iob_num_unread_bytes(file);
    } else if (__iob_write_buffer_is_valid(file)) {
        /* Add the number of bytes still stored in the buffer which have
         * not been written to disk yet.
         */
        position += __iob_num_unwritten_bytes(file);
    }

    result = position;

out:

    funlockfile(stream);

    return (result);
}
