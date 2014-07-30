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

bool traverse(const string filepath, const int maxdepth, int curdepth, vector<string> &list)
{
    DIR*            dr;
    struct dirent*  entry;
    struct stat     st;
    struct tm*      calendertime;
    string            curdir;
    string            curpath;
    bool            canread = true;

    if ( maxdepth != 0 && curdepth > maxdepth ) { return true; }
    curdepth++;
    if (filepath.length()  > SH_PATH_MAX )
    {
        fprintf(stderr, "Length of File Path exceeded max value !");
        canread = false;
    }
    else
    {
        curdir = filepath;
        if ( ( dr = opendir(filepath.c_str()) ) != NULL )
        {
            while ( ( entry = readdir( dr ) ) != NULL )
            {
                curpath = curdir;
                curpath.append("/");
                curpath.append(entry->d_name);
                if ( stat( curpath.c_str(), &st ) == -1 )
                {
                    perror("stat failed!");
                    canread = false;
                    return canread;
                }

                if ( S_ISDIR( st.st_mode ) )
                {
                    if ( strcmp(".", entry->d_name) == 0 ||
                        strcmp("..", entry->d_name) == 0 )
                    {
                        continue;
                    }
                    canread = traverse( curpath, maxdepth, curdepth, list );
                    if ( !canread ) {
                        return canread;
                    }
                }
                else {
                    list.push_back(curpath);
                }
            }
        }
        closedir( dr );
    }
    return canread;
}

int main(int argc, char* argv[])
{
    std::vector<std::string> list;
    int maxdepth = 0;
    if ( argc == 1 )
    {
        traverse(".", maxdepth, 1, list);    
    }
    else if ( argc == 2 )
    {
        traverse(argv[1], maxdepth, 1, list);    
    }
    else
    {
        fprintf(stdout,"Usage:\n");
        fprintf(stdout,"    crossls [filepath]\n");
    }
    for( vector<string>::iterator it = list.begin(); it != list.end(); ++it) {
      string tmp = *it;
      string a = tmp.substr(strlen(argv[1])+1);
      cout << a << endl;
    }
    return 0;
}
