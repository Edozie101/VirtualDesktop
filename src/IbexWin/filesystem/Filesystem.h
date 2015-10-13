//
//  Filesystem.h
//  IbexMac
//
//  Created by Hesham Wahba on 5/5/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__Filesystem__
#define __IbexMac__Filesystem__

#include <iostream>
#include <vector>
#include <string>

//namespace Ibex {
class Filesystem {
public:
    static bool isFile(std::string path);
    static bool isDirectory(std::string path);
    static std::string getFullPath(std::string from, std::string to);
    static std::string navigate(std::string from, std::string to);
    static std::string getHomeDirectory();
    static std::vector<std::string> listDirectory(const char *directory);
};
//}

#endif /* defined(__IbexMac__Filesystem__) */
