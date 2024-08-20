#include "anidopch.h"
#include "file.h"

#include <ftw.h>
#include <sys/stat.h>

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