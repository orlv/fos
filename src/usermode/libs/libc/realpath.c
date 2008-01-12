#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
/*
 * from glibc
 */

static inline void *
__rawmemchr (const void *__s, int __c)
{
  register unsigned long int __d0;
  register unsigned char *__res;
  __asm__ __volatile__
    ("cld\n\t"
     "repne; scasb\n\t"
     : "=D" (__res), "=&c" (__d0)
     : "a" (__c), "0" (__s), "1" (0xffffffff),
       "m" ( *(struct { char __x[0xfffffff]; } *)__s)
     : "cc");
  return __res - 1;
}
char * realpath (const char *name, char *resolved)
{
  char *rpath, *dest;
  const char *start, *end, *rpath_limit;
  long int path_max;

  if (name == NULL)
      return NULL;


  if (name[0] == '\0')
      return NULL;
   
    path_max = 1024;

  if (resolved == NULL)
    {
      rpath = malloc (path_max);
      if (rpath == NULL)
	return NULL;
    }
  else
    rpath = resolved;
  rpath_limit = rpath + path_max;

  if (name[0] != '/')
    {
      if (!getcwd (rpath, path_max))
	{
	  rpath[0] = '\0';
	  goto error;
	}
      dest = __rawmemchr (rpath, '\0');
    }
  else
    {
      rpath[0] = '/';
      dest = rpath + 1;
    }

  for (start = end = name; *start; start = end)
    {
//      struct stat64 st;

      /* Skip sequence of multiple path-separators.  */
      while (*start == '/')
	++start;

      /* Find end of path component.  */
      for (end = start; *end && *end != '/'; ++end)
	/* Nothing.  */;

      if (end - start == 0)
	break;
      else if (end - start == 1 && start[0] == '.')
	/* nothing */;
      else if (end - start == 2 && start[0] == '.' && start[1] == '.')
	{
	  /* Back up to previous component, ignore if at root already.  */
	  if (dest > rpath + 1)
	    while ((--dest)[-1] != '/');
	}
      else
	{
	  size_t new_size;

	  if (dest[-1] != '/')
	    *dest++ = '/';

	  if (dest + (end - start) >= rpath_limit)
	    {
	      ptrdiff_t dest_offset = dest - rpath;
	      char *new_rpath;

	      if (resolved)
		{
		  if (dest > rpath + 1)
		    dest--;
		  *dest = '\0';
		  goto error;
		}
	      new_size = rpath_limit - rpath;
	      if (end - start + 1 > path_max)
		new_size += end - start + 1;
	      else
		new_size += path_max;
	      new_rpath = (char *) realloc (rpath, new_size);
	      if (new_rpath == NULL)
		goto error;
	      rpath = new_rpath;
	      rpath_limit = rpath + new_size;

	      dest = rpath + dest_offset;
	    }

	  dest = memcpy (dest, start, end - start) + (end - start) ;
	  *dest = '\0';
	  DIR *tmp = opendir(rpath);
	  if(!tmp) goto error;
          closedir(tmp);
          
	}
    }
  if (dest > rpath + 1 && dest[-1] == '/')
    --dest;
  *dest = '\0';

  return rpath;

error:
  if (resolved == NULL)
    free (rpath);
  return NULL;
}


