/* 
** Copyright 2001, Manuel J. Petit. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#include <unistd.h>
#include <syscalls.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

DIR *opendir(const char *path)
{
	int fd = sys_open(path, STREAM_TYPE_DIR, O_RDONLY);
	DIR *ndir = NULL;
	
	if (fd < 0)
		return NULL;
	
	/* allocate the memory for the DIR structure */
	if ((ndir = (DIR*)malloc(sizeof(DIR) + strlen(path))) == NULL)
		return NULL;
	ndir->fd = fd;
	memcpy(ndir->me.d_name, path, strlen(path));
	ndir->me.d_reclen = (unsigned short)strlen(path);
	ndir->ent = NULL;
	
	return ndir;
}

int closedir(DIR *dir)
{
	int rv = sys_close(dir->fd);
	if (rv < 0) {
		errno = rv;
		rv = -1;
	}
	if (dir->ent)
		free(dir->ent);
	free(dir);
	return rv;
}

struct dirent *readdir(DIR *dirp)
{
	/* get the next entry and return a pointer to a dirent structure 
	 * containing the data
	 */
	/* We don't allocate until we know how big it needs to be... */
	struct dirent *dent = NULL;
	int err = 0;
	char buf[1024]; /* make a system defined size constant */

	if (dirp->ent) {
		free(dirp->ent);
		dirp->ent = NULL;
	}
	
	buf[0] = '\0';
	err = sys_read(dirp->fd, buf, -1, sizeof(buf));
	if (err <= 0) {
		return NULL;
	}

	dent = (struct dirent *)malloc(sizeof(struct dirent) + strlen(buf));
	if (!dent)
		return NULL;

	memset(dent, 0, sizeof(struct dirent) + strlen(buf));
	dent->d_reclen = (unsigned short)strlen(buf);
	memcpy(dent->d_name, buf, strlen(buf));
	/* unless we stat we can't get the vnode id, so skip it */
	dirp->ent = dent;
	
	return dent;
}

void rewinddir(DIR *dirp)
{
	/* simply close the directory and reopen... crude! */
	int err = 0;
	
	/* free the entry as it won't be valid now... */
	if (dirp->ent) {
		free(dirp->ent);
		dirp->ent = NULL;
	}
	
	err = sys_close(dirp->fd);
	if (err < 0) {
		errno = err;
		return;
	}
	
	err = sys_open(dirp->me.d_name, STREAM_TYPE_DIR, O_RDONLY);
	if (err < 0) {
		/* oh no! what do we do?
		 * for now, do nothing as the previous fd is still there
		 * and presumably it's valid?
		 */
		errno = err;
		return;
	}
}
