/*
 * $Id: stat_fchmod.c,v 1.14 2021-01-31 14:39:23 clib2devs Exp $
*/

#ifndef _STAT_HEADERS_H
#include "stat_headers.h"
#endif /* _STAT_HEADERS_H */

int
fchmod(int file_descriptor, mode_t mode) {
    struct ExamineData *fib = NULL;
    ULONG protection;
    BPTR parent_dir = ZERO;
    BPTR old_current_dir = ZERO;
    BOOL current_dir_changed = FALSE;
    int result = ERROR;
    struct fd *fd = NULL;
    LONG success;

    ENTER();

    SHOWVALUE(file_descriptor);
    SHOWVALUE(mode);

    assert(file_descriptor >= 0 && file_descriptor < __num_fd);
    assert(__fd[file_descriptor] != NULL);
    assert(FLAG_IS_SET(__fd[file_descriptor]->fd_Flags, FDF_IN_USE));

    __check_abort();

    __stdio_lock();

    fd = __get_file_descriptor(file_descriptor);
    if (fd == NULL) {
        __set_errno(EBADF);
        goto out;
    }

    __fd_lock(fd);

    if (FLAG_IS_SET(fd->fd_Flags, FDF_IS_SOCKET)) {
        __set_errno(EINVAL);
        goto out;
    }

    if (FLAG_IS_SET(fd->fd_Flags, FDF_STDIO)) {
        __set_errno(EBADF);
        goto out;
    }

    protection = 0;

    if (FLAG_IS_CLEAR(mode, S_IRUSR))
        SET_FLAG(protection, EXDF_NO_READ);

    if (FLAG_IS_CLEAR(mode, S_IWUSR)) {
        SET_FLAG(protection, EXDF_NO_WRITE);
        SET_FLAG(protection, EXDF_NO_DELETE);
    }

    if (FLAG_IS_CLEAR(mode, S_IXUSR))
        SET_FLAG(protection, EXDF_NO_EXECUTE);

    if (FLAG_IS_SET(mode, S_IRGRP))
        SET_FLAG(protection, EXDF_GRP_READ);

    if (FLAG_IS_SET(mode, S_IWGRP)) {
        SET_FLAG(protection, EXDF_GRP_WRITE);
        SET_FLAG(protection, EXDF_GRP_DELETE);
    }

    if (FLAG_IS_SET(mode, S_IXGRP))
        SET_FLAG(protection, EXDF_GRP_EXECUTE);

    if (FLAG_IS_SET(mode, S_IROTH))
        SET_FLAG(protection, EXDF_OTR_READ);

    if (FLAG_IS_SET(mode, S_IWOTH)) {
        SET_FLAG(protection, EXDF_OTR_WRITE);
        SET_FLAG(protection, EXDF_OTR_DELETE);
    }

    if (FLAG_IS_SET(mode, S_IXOTH))
        SET_FLAG(protection, EXDF_OTR_EXECUTE);

    parent_dir = __safe_parent_of_file_handle(fd->fd_File);
    if (parent_dir == ZERO) {
        SHOWMSG("couldn't find parent directory");

        __set_errno(__translate_io_error_to_errno(IoErr()));
        goto out;
    }

    fib = ExamineObjectTags(EX_FileHandleInput, fd->fd_File, TAG_DONE);
    success = (fib != NULL);
    if (NO success) {
        SHOWMSG("could not obtain file name");

        __set_errno(__translate_io_error_to_errno(IoErr()));
        goto out;
    }

    old_current_dir = CurrentDir(parent_dir);
    current_dir_changed = TRUE;

    if (CANNOT SetProtection((STRPTR)fib->Name, protection))
    {
        __set_errno(__translate_io_error_to_errno(IoErr()));
        goto out;
    }

    result = OK;

out:

    __fd_unlock(fd);
    __stdio_unlock();

    if (current_dir_changed)
        CurrentDir(old_current_dir);

    if (fib != NULL)
        FreeDosObject(DOS_EXAMINEDATA, fib);

    UnLock(parent_dir);

    RETURN(result);
    return (result);
}

void
__convert_stat64_to_stat(const struct stat64 *st64, struct stat *st) {
    ENTER();

    assert(st64 != NULL && st != NULL);

    memset(st, 0, sizeof(*st));

    st->st_dev = st64->st_dev;
    st->st_ino = st64->st_ino;
    st->st_mode = st64->st_mode;
    st->st_nlink = st64->st_nlink;
    st->st_uid = st64->st_uid;
    st->st_gid = st64->st_gid;
    st->st_rdev = st64->st_rdev;
    st->st_size = st64->st_size;
    st->st_atime = st64->st_atime;
    st->st_mtime = st64->st_mtime;
    st->st_ctime = st64->st_ctime;
    st->st_blksize = st64->st_blksize;
    st->st_blocks = st64->st_blocks;
    st->st_spare1 = st->st_atime;
    st->st_spare2 = st->st_mtime;
    st->st_spare3 = st->st_ctime;
    LEAVE();
}
