/* classes: h_files */

#ifndef SCM_WIN32_DIRENT_H
#define SCM_WIN32_DIRENT_H

/* Copyright (C) 2001, 2006 Free Software Foundation, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* Directory stream type.
   The miscellaneous Unix `readdir' implementations read directory data
   into a buffer and return `struct dirent *' pointers into it.  */

#include <sys/types.h>
#include <sys/stat.h>

#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(x) (((x) & S_IFREG))
#endif

struct dirstream
{
  int fd;		/* File descriptor.  */
  char *data;		/* Directory block.  */
  size_t allocation;	/* Space allocated for the block.  */
  size_t size;		/* Total valid data in the block.  */
  size_t offset;	/* Current offset into the block.  */
  off_t filepos;	/* Position of next entry to read.  */
  char *mask;           /* Initial file mask. */
};

struct dirent
{
  long d_ino;
  off_t d_off;
  unsigned short int d_reclen;
  unsigned char d_type;
  char d_name[256];
};

#define d_fileno d_ino /* Backwards compatibility. */

/* This is the data type of directory stream objects.
   The actual structure is opaque to users.  */

typedef struct dirstream DIR;

DIR * opendir (const char * name);
struct dirent * readdir (DIR * dir);
int closedir (DIR * dir);
void rewinddir (DIR * dir);
void seekdir (DIR * dir, off_t offset);
off_t telldir (DIR * dir);
int dirfd (DIR * dir);

#endif /* SCM_WIN32_DIRENT_H */
