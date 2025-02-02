/*
 * $Id: ftw_nftw.c,v 1.5 2006-01-08 12:04:22 clib2devs Exp $
*/

/*
 * This nftw() implementation builds heavily on existing functionality and could certainly
 * be made more efficient.
 */

#ifndef    _FTW_HEADERS_H
#include "ftw_headers.h"
#endif /* _FTW_HEADERS_H */

static int
walk(const char *path, int (*func)(const char *, const struct stat *, int, struct FTW *), const int depth, int level,
     const int flags, const int base, int *const prune) {
    int result = OK;
    DIR *dp;
    struct dirent *de;
    struct stat st;
    char *next_name;
    char *old_cwd = 0;
    int type;
    int old_length;
    int errtmp;
    struct FTW extra_info;
    int stat_result;

    ENTER();

    SHOWSTRING(path);
    SHOWPOINTER(func);
    SHOWVALUE(depth);
    SHOWVALUE(level);
    SHOWVALUE(flags);

    __check_abort();

    if (level > depth)
        goto out;

    if (FLAG_IS_SET(flags, FTW_PHYS))
        stat_result = lstat(path, &st);
    else
        stat_result = stat(path, &st);

    if (stat_result == OK) {
        if (S_ISLNK(st.st_mode)) /* Should only be possible if lstat() is used. */
        {
            type = FTW_SL;
        } else if (S_ISDIR(st.st_mode)) {
            if (FLAG_IS_SET(st.st_mode, S_IRUSR))
                type = FTW_D;    /* Directory */
            else
                type = FTW_DNR;    /* Directory No Read-permission */
        } else if (S_ISREG(st.st_mode)) {
            type = FTW_F; /* File */
        } else {
            /* Skip */
            goto out;
        }
    } else {
        if (FLAG_IS_CLEAR(flags, FTW_PHYS) && lstat(path, &st) == OK)
            type = FTW_SLN;    /* Broken link. */
        else
            type = FTW_NS;    /* No Stat */
    }

    extra_info.quit = 0;
    extra_info.base = base;
    extra_info.level = level;

    if (type == FTW_D) {
        old_length = strlen(path);

        if (FLAG_IS_CLEAR(flags, FTW_DEPTH))
            result = (*func)(path, &st, type, &extra_info);

        if (extra_info.quit == 0) {
            dp = opendir(path);    /* Also takes care of Unix->Amiga pathname conversion. */
            if (dp == NULL) {
                result = ERROR;
                goto out;
            }

            if (FLAG_IS_SET(flags, FTW_CHDIR)) /* Change to directory before traversing. */
            {
                old_cwd = malloc(old_length + NAME_MAX);
                if (old_cwd == NULL) {
                    __set_errno(ENOMEM);

                    result = ERROR;
                    goto out;
                }

                getcwd(old_cwd, old_length + NAME_MAX);
                chdir(path);
            }

            next_name = malloc(old_length + NAME_MAX +
                               2); /* Allocate new for each recursive step to handle extremely long path names. */
            if (next_name == NULL) {
                __set_errno(ENOMEM);

                result = ERROR;
                goto out;
            }

            strcpy(next_name, path);
            if (old_length > 0 && next_name[old_length - 1] != ':' && next_name[old_length - 1] != '/')
                next_name[old_length++] = '/';

            while ((result == 0) && (de = readdir(dp)) != NULL) {
                strlcpy(&next_name[old_length], de->d_name, NAME_MAX + 2);

                result = walk(next_name, func, depth, level + 1, flags, old_length, prune);
                if ((*prune) != 0) {
                    (*prune) = 0;
                    break;
                }
            }

            errtmp = __get_errno();

            closedir(dp);

            __set_errno(errtmp);

            free(next_name);

            if ((result == 0) && FLAG_IS_SET(flags, FTW_DEPTH))
                result = (*func)(path, &st, FTW_DP, &extra_info);
        }

        if (extra_info.quit == FTW_PRUNE) /* Abort traversal of current directory. */
            (*prune) = 1;
    } else {
        result = (*func)(path, &st, type, &extra_info);
    }

out:

    if (old_cwd != NULL) {
        errtmp = __get_errno();

        chdir(old_cwd); /* Restore (pop) old directory. */
        free(old_cwd);

        __set_errno(errtmp);
    }

    RETURN(result);
    return (result);
}

static int
index_of_end_part(const char *path) {
    int result = OK;
    int i;

    i = strlen(path) - 1;
    while (i-- > 0) {
        if (path[i] == '/' || path[i] == ':') {
            result = i + 1;
            break;
        }
    }

    return (result);
}

int
nftw(const char *path, int (*func)(const char *, const struct stat *, int, struct FTW *), int depth, int flags) {
    char *base;
    int len;
    int base_index;
    int prune = 0;
    int result = ERROR;

    ENTER();

    SHOWSTRING(path);
    SHOWPOINTER(func);
    SHOWVALUE(depth);
    SHOWVALUE(flags);

    if (path == NULL) {
        SHOWMSG("NULL pathname to nftw().");
        __set_errno(EFAULT);
        goto out;
    }

    if (func == NULL) {
        SHOWMSG("No function supplied.");
        __set_errno(EFAULT);
        goto out;
    }

    if (depth < 0) {
        SHOWMSG("Invalid depth.\n");
        __set_errno(EINVAL);
        goto out;
    }

    if ((flags & ~FTW_ALL_FLAGS) != 0) {
        SHOWMSG("Bogus flags.");

        __set_errno(EINVAL);
        goto out;
    }

    /* Make sure the path never ends with '/' unless used to indicate parent directory. */

    len = strlen(path);
    if (len > 1 && path[len - 1] == '/' && path[len - 2] != '/') {
        int error;

        base = strdup(path);
        if (base == NULL) {
            __set_errno(ENOMEM);
            goto out;
        }

        base[len - 1] = '\0';

        base_index = index_of_end_part(base);

        result = walk(base, func, depth, 0, flags, base_index, &prune);

        error = __get_errno();
        free(base);
        __set_errno(error);
    } else {
        base_index = index_of_end_part(path);

        result = walk(path, func, depth, 0, flags, base_index, &prune);
    }

out:

    RETURN(result);
    return (result);
}
