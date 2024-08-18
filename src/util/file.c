#include "file.h"

#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>

int rmfn(const char *fpath, const struct stat *sb, int typeflag,
         struct FTW *ftwbuf) {
  int res = typeflag == FTW_DNR;
  if (typeflag == FTW_F) {
    res = unlink(fpath);
  } else if (typeflag == FTW_DP) {
    res = rmdir(fpath);
  }

  return res;
}

int remove_dir(const char *directory) {
  return nftw(directory, rmfn, 64, FTW_DEPTH | FTW_PHYS);
}

int create_dir(const char *directory) { return mkdir(directory, 0777); }