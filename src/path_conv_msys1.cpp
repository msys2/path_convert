
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MSYS__
# include <sys/cygwin.h>
#endif
#include <ctype.h>

#include "path_conv_msys1.h"

#ifndef __MSYS__
static const char* ROOT_PATH = "C:/msys32";
#include <unistd.h>
#endif

static bool
isabswinpath (const char * path)
{
  int plen = strlen (path);
  bool p0alpha = isalpha (path[0]) != 0;
  bool p1colon = (plen > 1 && path[1] == ':');
  bool rval = (
     ((plen == 2) && p0alpha && p1colon)
    || (
           (plen > 2) 
         && p0alpha 
         && p1colon 
         && (strchr (&path[2], ':') == (char *)NULL)
        )
    || (
           plen > 3 
        && path[0] == '\\' 
        && path[1] == '\\' 
        && path[3] == '\\'
       )
   );
  return rval;
}

static char *
ScrubRetpath (char * const retpath)
{ 
  char * sspath = (char *)retpath;
  //
  // Check for null path because Win32 doesn't like them.
  // I.E.:  Path lists of c:/foo;;c:/bar need changed to 
  // c:/foo;c:/bar.
  //
  // This need be executed only if we actually converted the path.
  //
  while (*sspath) {
    if (*sspath == ';' && sspath[1] == ';')
        for (char *i = sspath; *i; i++)
            *i = *(i + 1);
    else
        sspath++;
  }
  if (*(sspath - 1) == ';')
      *(sspath - 1) = '\0';

  //
  // If we modified the path then convert all / to \ if we have a path list
  // else convert all \ to /.
  // 
  if ((strchr (retpath, ';'))) {
      backslashify (retpath, retpath, 0);
  } else {
      slashify (retpath, retpath, 0);
  }
  debug_printf("returning: %s", retpath);
  return retpath;
}

//
// The returned pointer should be freed with free unless,
// as it turns out, it is equal to the input pointer.
//
char *
arg_heuristic_with_exclusions (char const * const arg, char const * exclusions, size_t exclusions_count)
{
  int arglen = (arg ? strlen (arg): 0);
  
  if (arglen == 0) {
      char *retpath = (char *)malloc (sizeof (char));
      memset (retpath, 0, sizeof (char));
      return retpath;
  }
  
  debug_printf("Input value: (%s)", arg);
  for (size_t excl = 0; excl < exclusions_count; ++excl) {
      if ( strstr (arg, exclusions) == arg )
          return (char*)arg;
      exclusions += strlen (exclusions) + 1;
  }

  //
  // copy of the path string that we can overwrite
  //
  char *spath = (char *)alloca (arglen + 1);
  memcpy (spath, arg, arglen + 1);

  char * sspath;

  //
  // retpath contains the converted path string to be returned
  //
  char *retpath = (char *)malloc(((MAX_PATH - arglen) > 0) ? MAX_PATH : arglen + MAX_PATH);
  memset (retpath, 0, MAX_PATH);
  int retpath_len = 0;
  int retpath_buflen = MAX_PATH;

#define retpathcat(retstr) \
  retpath_len += strlen(retstr); \
  if (retpath_buflen <= retpath_len) \
    { \
      retpath_buflen = ((retpath_buflen * 2 <= retpath_len) ? \
    retpath_len + 1 : retpath_buflen * 2); \
      retpath = (char *)realloc (retpath, retpath_buflen); \
    } \
  strcat (retpath, retstr);

#define retpathcpy(retstr) \
  retpath_len = strlen (retstr); \
  *retpath = '\0'; \
  if (retpath_buflen <= retpath_len ) \
    { \
      retpath_buflen = ((retpath_buflen * 2 <= retpath_len) ? \
    retpath_len + 1 : retpath_buflen * 2); \
      retpath = (char *)realloc (retpath, retpath_buflen); \
    } \
  strcpy (retpath, retstr);

  //
  // Just return win32 paths and path lists.
  //
  if (isabswinpath (arg) || (strchr (arg, ';') > 0)) {
      debug_printf("returning Win32 absolute path: %s", arg);
      return ((char *)arg);
  }
  //
  // Multiple forward slashes are treated special,
  // Remove one and return for the form of //foo or ///bar
  // but just return for the form of //server/share.
  //
  else if (arg[0] == '/' && arg[1] == '/') {
      int tidx = 2;
      while (spath[tidx] && spath[tidx] == '/')
          tidx++;
      if (strchr (&spath[tidx], '/')) {
          retpathcpy (spath);
      } else {
          retpathcpy (&spath[1]);
      }
      return ScrubRetpath (retpath);
  }
  //
  // special case confusion elimination
  // Translate a path that looks similar to /c: to c:/.
  //
  else if (arg[0] == '/' && isabswinpath (arg + 1)) {
      retpathcpy (&arg[1]);
      return ScrubRetpath (retpath);
  }
  //
  // Check for variable set.
  //
  else if ((sspath = strchr(spath, '=')) && isalpha (spath[0])) {
      if (isabswinpath (sspath + 1)) {
          debug_printf("returning: %s", arg);
          return (char *)arg;
      }
      char *swin32_path = arg_heuristic(sspath + 1);
      if (swin32_path == (sspath + 1)) {
          debug_printf("returning: %s", arg);
          return (char *)arg;
      }
      *sspath = '\0';
      retpathcpy (spath);
      retpathcat ("=");
      retpathcat (swin32_path);
      free (swin32_path);
      return ScrubRetpath (retpath);
  }
  //
  // Check for paths after commas, if string begins with a '-' character.
  //
  else if ((sspath = strchr(spath, ',')) && spath[0] == '-') {
      if (isabswinpath (sspath + 1)) {
          debug_printf("returning: %s", arg);
          return (char *)arg;
      }
      char *swin32_path = arg_heuristic(sspath + 1);
      if (swin32_path == (sspath + 1)) {
          debug_printf("returning: %s", arg);
          return (char *)arg;
      }
      *sspath = '\0';
      retpathcpy (spath);
      retpathcat (",");
      retpathcat (swin32_path);
      free (swin32_path);
      return ScrubRetpath (retpath);
  }
  //
  // Check for POSIX path lists.
  // But we have to allow processing of quoted strings and switches first
  // which uses recursion so this code will be seen again.
  //
  else {
      sspath = strchr (spath, ':');
      //
      // Prevent http://some.string/ from being modified.
      // 
      if (   (sspath > 0 && strlen (sspath) > 2)
          && (sspath[1] == '/')
          && (sspath[2] == '/'))
      {
          debug_printf("returning: %s", arg);
          return ((char *)arg);
      }
      else
          if ((sspath > 0) && (strchr (spath, '/') > 0)
              // 
              // Prevent strings beginning with -, ", ', or @ from being processed,
              // remember that this is a recursive routine.
              // 
              && (strchr ("-\"\'@", spath[0]) == 0)
              // 
              // Prevent ``foo:echo /bar/baz'' from being considered a path list.
              // 
              && (strlen (sspath) > 1 && strchr (":./", sspath[1]) > 0)
              )
          {
              //
              // Yes, convert to Win32 path list.
              //
              while (sspath)
              {
                  *sspath = '\0';
                  char *swin32_path = arg_heuristic (spath);
                  //
                  // Just ignore sret; swin32_path has the value we need.
                  //
                  retpathcat (swin32_path);
                  if (swin32_path != spath)
                      free (swin32_path);
                  spath = sspath + 1;
                  sspath = strchr (spath, ':');
                  retpathcat (";");
                  //
                  // Handle the last path in the list.
                  //
                  if (!sspath) {
                      char *swin32_path = arg_heuristic (spath);
                      retpathcat (swin32_path);
                      if (swin32_path != spath)
                      free (swin32_path);
                  }
              }
              return ScrubRetpath (retpath);
          } else {
              switch (spath[0]) {
              case '/':
                  //
                  // Just a normal POSIX path.
                  //
                  {
                      //
                      // Convert only up to a ".." path component, and
                      // keep all what follows as is.
                      //
                      sspath = strstr (spath, "/..");
                      if (sspath) {
                          *sspath = '\0';
                          char *swin32_path = arg_heuristic (spath);
                          if (swin32_path == spath) {
                              debug_printf("returning: %s", arg);
                              return ((char *)arg);
                          }
                          retpathcpy (swin32_path);
                          retpathcat ("/");
                          retpathcat (sspath+1);
                          free (swin32_path);
                          return ScrubRetpath (retpath);
                      }
                      if (strcmp(spath, "/dev/null") == 0) {
                          retpathcpy("nul");
                          return ScrubRetpath (retpath);
                      }
                      path_conv p (spath, 0);
                      /*if (p.error) {
                          set_errno(p.error);
                          debug_printf("returning: %s", arg);
                          return ((char *)arg);
                      }*/
                      char win32_path1[PATH_MAX + 1];
                      ssize_t result = cygwin_conv_path(CCP_POSIX_TO_WIN_A|CCP_ABSOLUTE, spath, win32_path1, PATH_MAX+1);
                      retpathcpy (win32_path1);
                      return ScrubRetpath (retpath);
                  }
              case '-':
                  //
                  // here we check for POSIX paths as attributes to a POSIX switch.
                  //
                  sspath = strchr (spath, '=');
                  if (sspath) {
                      //
                      // just use recursion if we find a set variable token.
                      //
                      *sspath = '\0';
                      if (isabswinpath (sspath + 1)) {
                          debug_printf("returning: %s", arg);
                          return (char *)arg;
                      }
                      char *swin32_path = arg_heuristic(sspath + 1);
                      if (swin32_path == sspath + 1) {
                          debug_printf("returning: %s", arg);
                          return ((char *)arg);
                      }
                      retpathcpy (spath);
                      retpathcat ("=");
                      retpathcat (swin32_path);
                      free (swin32_path);
                      return ScrubRetpath (retpath);
                  } else {
                      //
                      // Check for single letter option with a
                      // path argument attached, eg -I/include */
                      //
                      if (spath[1] && spath[2] == '/') {
                          debug_printf("spath = %s", spath);
                          sspath = spath + 2;
                          char *swin32_path = arg_heuristic (sspath);
                          if (swin32_path == sspath) {
                              debug_printf("returning: %s", arg);
                              return ((char *)arg);
                          }
                          sspath = (char *)spath;
                          sspath++;
                          sspath++;
                          *sspath = '\0';
                          retpathcpy (spath);
                          *sspath = '/';
                          retpathcat (swin32_path);
                          free (swin32_path);
                          return ScrubRetpath (retpath);
                      } else {
                          debug_printf("returning: %s", arg);
                          return ((char *)arg);
                      }
                  }
                  break;
              case '@':
                  //
                  // here we check for POSIX paths as attributes to a response
                  // file argument (@file). This is specifically to support
                  // MinGW binutils and gcc.
                  //
                  sspath = spath + 1;
                  if (isabswinpath (sspath)) {
                      debug_printf("returning: %s", arg);
                      return (char *)arg;
                  }
                  if (spath[1] == '/') {
                      debug_printf("spath = %s", spath);
                      char *swin32_path = arg_heuristic (sspath);
                      if (swin32_path == sspath) {
                          debug_printf("returning: %s", arg);
                          return ((char *)arg);
                      }
                      sspath = (char *)spath;
                      sspath++;
                      *sspath = '\0';
                      retpathcpy (spath);
                      *sspath = '/';
                      retpathcat (swin32_path);
                      free (swin32_path);
                      return ScrubRetpath (retpath);
                  } else {
                      debug_printf("returning: %s", arg);
                      return ((char *)arg);
                  }
                  break;
              case '"':
                  //
                  // Handle a double quote case.
                  //
                  debug_printf ("spath: %s", spath);
                  if (spath[1] == '/') {
                      retpathcpy ("\"");
                      char *tpath = strchr(&spath[1], '"');
                      if (tpath)
                          *tpath = (char)NULL;
                      char *swin32_path = arg_heuristic (&spath[1]);
                      if (swin32_path == &spath[1]) {
                          debug_printf("returning: %s", arg);
                          return ((char *)arg);
                      }
                      retpathcat (swin32_path);
                      free (swin32_path);
                      if (tpath)
                          retpathcat ("\"");
                      return ScrubRetpath (retpath);
                  }
                  debug_printf("returning: %s", arg);
                  return ((char *)arg);
              case '\'':
                  //
                  // Handle a single quote case.
                  //
                  debug_printf ("spath: %s", spath);
                  if (spath[1] == '/') {
                      retpathcpy ("'");
                      char *tpath = strchr(&spath[1], '\'');
                      if (tpath)
                          *tpath = (char)NULL;
                      char *swin32_path = arg_heuristic (&spath[1]);
                      if (swin32_path == &spath[1]) {
                          debug_printf("returning: %s", arg);
                          return ((char *)arg);
                      }
                      retpathcat (swin32_path);
                      free (swin32_path);
                      if (tpath)
                          retpathcat ("'");
                      return ScrubRetpath (retpath);
                  }
                  debug_printf("returning: %s", arg);
                  return ((char *)arg);
              default:
                  //
                  // This takes care of variable_foo=/bar/baz
                  //
                  if ((sspath = strchr(spath, '=')) && (sspath[1] == '/')) {
                      sspath[1] = '\0';
                      retpathcpy (spath);
                      sspath[1] = '/';
                      char *swin32_path = arg_heuristic (&sspath[1]);
                      if (swin32_path == &sspath[1]) {
                          debug_printf("returning: %s", arg);
                          return ((char *)arg);
                      }
                      retpathcat (swin32_path);
                      free (swin32_path);
                      return ScrubRetpath (retpath);
                  }
                  //
                  // Oh well, nothing special found, set win32_path same as path.
                  //
                  debug_printf("returning: %s", arg);
                  return ((char *)arg);
              }
          }
  }
  // I should not get to this point.
  assert (false);
  debug_printf("returning: %s", arg);
  return ScrubRetpath (retpath);
}

