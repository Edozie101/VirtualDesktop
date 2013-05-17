//
//  Filesystem.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 5/5/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "Filesystem.h"

#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <pwd.h>

#include <vector>
#include <string>

std::string Filesystem::getFullPath(std::string from, std::string to) {
    if(to.size() > 0 && to[0] == '*') {
        to = to.substr(1);
    }
    std::string path = from+"/"+to;
    return path;
}

bool Filesystem::isFile(std::string path) {
    return access(path.c_str(), R_OK) != -1;
}
bool Filesystem::isDirectory(std::string path) {
    DIR *dir;
    if((dir = opendir(path.c_str())) != NULL) {
        closedir(dir);
        return true;
    }
    return false;
}
std::string Filesystem::navigate(std::string from, std::string to) {
    std::string path = getFullPath(from, to);
    if(!isDirectory(path)) {
        path = from;
    }
    
    return path;
}

std::string Filesystem::getHomeDirectory() {
    struct passwd *pwd = getpwuid(geteuid());
    const char *home = (pwd != 0) ? pwd->pw_dir : ".";
    return std::string(home);
}

std::vector<std::string> Filesystem::listDirectory(const char *directory) {
    std::vector<std::string> result;
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
    return result;
}