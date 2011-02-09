/* Copyright (C) 2001, 2006, 2008 Free Software Foundation, Inc.
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

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "win32-dirent.h"

DIR *
opendir (const char * name)
{
  DIR *dir;
  HANDLE hnd;
  char *file;
  WIN32_FIND_DATAA find;

  if (!name || !*name) 
    return NULL;
  file = (char*)malloc (strlen (name) + 3);
  strcpy (file, name);
  if (file[strlen (name) - 1] != '/' && file[strlen (name) - 1] != '\\')
    strcat (file, "/*");
  else
    strcat (file, "*");
  
  if ((hnd = FindFirstFileA (file, &find)) == INVALID_HANDLE_VALUE)
    {
      free (file);
      return NULL;
    }

  dir = (DIR*)malloc (sizeof (DIR));
  dir->mask = file;
  dir->fd = (int) hnd;
  dir->data = (char*)malloc (sizeof (WIN32_FIND_DATA));
  dir->allocation = sizeof (WIN32_FIND_DATA);
  dir->size = dir->allocation;
  dir->filepos = 0;
  memcpy (dir->data, &find, sizeof (WIN32_FIND_DATA));
  return dir;
}

struct dirent *
readdir (DIR * dir)
{
  static struct dirent entry;
  WIN32_FIND_DATAA *find;

  entry.d_ino = 0;
  entry.d_type = 0;
  find = (WIN32_FIND_DATAA *) dir->data;

  if (dir->filepos)
    {
      if (!FindNextFileA ((HANDLE) dir->fd, find))
	return NULL;
    }

  entry.d_off = dir->filepos;
  strncpy (entry.d_name, find->cFileName, sizeof (entry.d_name));
  entry.d_reclen = strlen (find->cFileName);
  dir->filepos++;
  return &entry;
}

int 
closedir (DIR * dir)
{
  HANDLE hnd = (HANDLE) dir->fd;
  free (dir->data);
  free (dir->mask);
  free (dir);
  return FindClose (hnd) ? 0 : -1;
}

void 
rewinddir (DIR * dir)
{
  HANDLE hnd = (HANDLE) dir->fd;
  WIN32_FIND_DATAA *find = (WIN32_FIND_DATAA *) dir->data;

  FindClose (hnd);
  hnd = FindFirstFileA (dir->mask, find);
  dir->fd = (int) hnd;
  dir->filepos = 0;
}

void 
seekdir (DIR * dir, off_t offset)
{
  off_t n;

  rewinddir (dir);
  for (n = 0; n < offset; n++)
    {
      if (FindNextFile ((HANDLE) dir->fd, (WIN32_FIND_DATA *) dir->data))
	dir->filepos++;
    }
}

off_t 
telldir (DIR * dir)
{
  return dir->filepos;
}

int 
dirfd (DIR * dir)
{
  return dir->fd;
}
