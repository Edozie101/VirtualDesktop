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

void Filesystem::listDirectory() {
    DIR *dir;
    struct dirent *ent;
    struct passwd *pwd = getpwuid(geteuid());
    const char *home = (pwd != 0) ? pwd->pw_dir : ".";
    
    if ((dir = opendir (home)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            printf ("%s\n", ent->d_name);
        }
        closedir (dir);
    } else {
        fprintf(stderr, "Error: unable to open directory: %s", home);
    }
}