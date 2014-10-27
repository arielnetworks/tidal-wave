#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "utility.h"

using namespace std;

namespace tidalwave {

  bool Utility::getFilesInDirectory(string const dirPath, vector<string> &list, int currentDepth) {
    bool canRead = true;
    currentDepth++;
    if (dirPath.length() > SH_PATH_MAX) {
      fprintf(stderr, "Length of File Path exceeded max value !");
      canRead = false;
    } else {
      string currentDir = dirPath;
      DIR *dr;
      if ((dr = ::opendir(dirPath.c_str())) != NULL) {
        struct dirent *entry;
        while ((entry = ::readdir(dr)) != NULL) {
          string currentPath = currentDir;
          currentPath.append("/");
          currentPath.append(entry->d_name);
          struct stat st;
          if (::stat(currentPath.c_str(), &st) == -1) {
            perror("stat failed!");
            canRead = false;
            return canRead;
          }

          if (S_ISDIR(st.st_mode)) {
            if (::strcmp(".", entry->d_name) == 0 || ::strcmp("..", entry->d_name) == 0) {
              continue;
            }
            canRead = getFilesInDirectory(currentPath, list, currentDepth);
            if (!canRead) {
              return canRead;
            }
          } else {
            list.push_back(currentPath);
          }
        }
      }
      closedir(dr);
    }
    return canRead;
  }
}
