#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

#define SH_PATH_MAX     255

bool listFiles(const string filepath, int curdepth, vector<string> &list) {
	bool canread = true;
	curdepth++;
	if (filepath.length() > SH_PATH_MAX) {
		fprintf(stderr, "Length of File Path exceeded max value !");
		canread = false;
	} else {
		string curdir = filepath;
		DIR* dr;
		if ((dr = opendir(filepath.c_str())) != NULL ) {
			struct dirent* entry;
			while ((entry = readdir(dr)) != NULL) {
				string curpath = curdir;
				curpath.append("/");
				curpath.append(entry->d_name);
				struct stat st;
				if (stat(curpath.c_str(), &st) == -1) {
					perror("stat failed!");
					canread = false;
					return canread;
				}

				if (S_ISDIR(st.st_mode)) {
					if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) {
						continue;
					}
					canread = listFiles(curpath, curdepth, list);
					if (!canread) {
						return canread;
					}
				}
				else {
					list.push_back(curpath);
				}
			}
		}
		closedir(dr);
	}
	return canread;
}

