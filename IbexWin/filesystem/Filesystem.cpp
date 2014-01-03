//
//  Filesystem.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 5/5/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "Filesystem.h"

#include <sys/types.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <sys/stat.h>
#include <io.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#endif

#include <vector>
#include <string>

std::string Filesystem::getFullPath(std::string from, std::string to) {
#ifdef WIN32
	if(to.size() > 0 && to[0] == '*') {
		to = to.substr(1);
    }
    std::string path = from+"\\"+to;
    return path;
#else
    if(to.size() > 0 && to[0] == '*') {
        to = to.substr(1);
    }
    std::string path = from+"/"+to;
    return path;
#endif
}

bool Filesystem::isFile(std::string path) {
#ifdef WIN32
	struct stat s;
	if(stat(path.c_str(),&s) == 0) {
		if(s.st_mode & S_IFDIR)
			return true; // dir
		else if(s.st_mode & S_IFREG)
			return true; // file
		else
			return false;
	}
else
    return false;
#else
    return access(path.c_str(), R_OK) != -1;
#endif
}
bool Filesystem::isDirectory(std::string path) {
#ifdef WIN32
	struct stat s;
	if(stat(path.c_str(),&s) == 0 && (s.st_mode & S_IFDIR))
		return true; // dir
	else
		return false;
#else
    DIR *dir;
    if((dir = opendir(path.c_str())) != NULL) {
        closedir(dir);
        return true;
    }
    return false;
#endif
}
std::string Filesystem::navigate(std::string from, std::string to) {
    std::string path = getFullPath(from, to);
    if(!isDirectory(path)) {
        path = from;
    }
    
    return path;
}

std::string Filesystem::getHomeDirectory() {
#ifdef WIN32
	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return "C:\\";

	return std::string(path);
#else
    struct passwd *pwd = getpwuid(geteuid());
    const char *home = (pwd != 0) ? pwd->pw_dir : ".";
    return std::string(home);
#endif
}

std::vector<std::string> Filesystem::listDirectory(const char *directory) {
    std::vector<std::string> result;

#ifdef WIN32
	struct _finddata_t c_file;
    long hFile;

	std::string dir(directory);
	dir += "\\*";
    /* Find first .c file in current directory */
    if((hFile = _findfirst(dir.c_str(), &c_file )) == -1L)
       printf("No files in current directory: %s\n", directory);
	else {
        /* Find the rest of the .c files */
        while( _findnext( hFile, &c_file ) == 0 ) {
            if(c_file.attrib & _A_HIDDEN )
				continue;

			std::string path(c_file.name);
			if(c_file.attrib & _A_SUBDIR) {
                path = "*"+path;
            }
            result.push_back(path);
        }

       _findclose( hFile );
	}
#else
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir(directory)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            printf ("%s\n", ent->d_name);
            std::string path = ent->d_name;
            if(ent->d_type == DT_DIR) {
                path = "*"+path;
            }
            result.push_back(path);
        }
        closedir (dir);
    } else {
        fprintf(stderr, "Error: unable to open directory: %s", directory);
    }
#endif

    return result;
}