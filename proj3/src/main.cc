// Copyright 2026 Jordan Weinstein
// Libraries
#include <stdio.h>
#include <string>
#include <vector>
#include <cstring>
#include <sys/stat.h>

// Headers
#include "proj3/lib/include/mmap.h"

void create(const char* path, char fill_char, size_t size) {
    int file = proj3::open(path, proj3::O_RDWR, proj3::O_CREAT);
    if (file == -1) {
        return;
    }

    proj3::ftruncate(file, size);
    void* addr = proj3::mmap(nullptr,
        size,
        proj3::PROT_READ | proj3::PROT_WRITE,
        proj3::MAP_SHARED,
        file,
        0);

    std::memset(addr, fill_char, size);
    proj3::msync(addr, size, proj3::MS_SYNC);

    proj3::munmap(addr, size);
    proj3::close(file);
}

void insert(const char* path, size_t offset, size_t bytes) {
    int file = proj3::open(path, proj3::O_RDWR);
    if (file == -1) {
        return;
    }
}

void append(const char* path, size_t bytes) {
    int file = proj3::open(path, proj3::O_RDWR, proj3::O_APPEND);
    if (file == -1) {
        return;
    }
}

int main(int argc, char* argv[]) {
    std::string mmapArg = argv[1];

    if (mmapArg == "create") {
        create(argv[2], argv[3][0], std::stoul(argv[4]));
    } else if (mmapArg == "insert") {
        insert(argv[2], std::stoul(argv[3]), std::stoul(argv[4]));
    } else if (mmapArg == "append") {
        append(argv[2], std::stoul(argv[3]));
    }

    return 0;
}
