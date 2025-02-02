/*
 * $Id: unistd_dup2.c,v 1.10 2006-01-08 12:04:27 clib2devs Exp $
*/

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

int 
dup2(int file_descriptor1, int file_descriptor2)
{
	struct fd *fd1;
	int result = ERROR;

	ENTER();

	SHOWVALUE(file_descriptor1);
	SHOWVALUE(file_descriptor2);

    __check_abort();

	__stdio_lock();

	assert(file_descriptor1 >= 0 && file_descriptor1 < __num_fd);
	assert(__fd[file_descriptor1] != NULL);
	assert(FLAG_IS_SET(__fd[file_descriptor1]->fd_Flags, FDF_IN_USE));

	fd1 = __get_file_descriptor(file_descriptor1);
	if (fd1 == NULL)
	{
		__set_errno(EBADF);
		goto out;
	}

	if (file_descriptor2 < 0)
	{
		/* Try to find a place to put the duplicate into. */
		file_descriptor2 = __find_vacant_fd_entry();
		if (file_descriptor2 < 0)
		{
			/* No free space, so let's grow the table. */
			if (__grow_fd_table(0) < 0)
			{
				SHOWMSG("not enough memory for new file descriptor");
				goto out;
			}

			file_descriptor2 = __find_vacant_fd_entry();
			assert(file_descriptor2 >= 0);
		}
	}
	else if (file_descriptor1 != file_descriptor2)
	{
		/* Make sure the requested duplicate exists. */
		if (__grow_fd_table(file_descriptor2 + 1) < 0)
			goto out;

		assert(file_descriptor2 >= 0 && file_descriptor2 < __num_fd);
		assert(__fd[file_descriptor2] != NULL);
	}

	if (file_descriptor1 != file_descriptor2)
	{
		struct fd *fd2;

		/* Have a look at the requested file descriptor. */
		assert(0 <= file_descriptor2 && file_descriptor2 < __num_fd);

		fd2 = __fd[file_descriptor2];

		assert(fd2 != NULL);

		/* Make sure that the entry is cleaned up before we used it. */
		if (FLAG_IS_SET(fd2->fd_Flags, FDF_IN_USE))
		{
			SHOWMSG("closing file descriptor #2");

			if (close(file_descriptor2) < 0)
				goto out;
		}

		__duplicate_fd(fd2, fd1);
	}

	result = file_descriptor2;

out:

	__stdio_unlock();

	RETURN(result);
	return (result);
}
