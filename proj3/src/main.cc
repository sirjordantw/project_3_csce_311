// Copyright 2026 Jordan Weinstein
// Libraries
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <sys/stat.h>

// Headers
#include "proj3/lib/include/mmap.h"

void create(const char* path, char fill_char, size_t size) {
    int file = proj3::open(path, proj3::O_RDWR | proj3::O_CREAT | proj3::O_TRUNC, 0644);
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

    struct stat buf;
    proj3::fstat(file, &buf);
    size_t size = buf.st_size;

    if (offset > size) {
        return;
    }

    proj3::ftruncate(file, size + bytes);

    void* addr = proj3::mmap(nullptr,
        size + bytes,
        proj3::PROT_READ | proj3::PROT_WRITE,
        proj3::MAP_SHARED,
        file,
        0);
    uint8_t* ptr = static_cast<uint8_t*>(addr);

    std::memmove(ptr + offset + bytes,
        ptr + offset,
        size - offset);
    
    std::cin.read(reinterpret_cast<char*>(ptr + offset), bytes);

    if (static_cast<size_t>(std::cin.gcount()) < bytes) {
        std::memmove(ptr + offset,
            ptr + offset + bytes,
            size - offset);
        proj3::munmap(addr, size + bytes);
        proj3::ftruncate(file, size);
        proj3::close(file);
        return;
    }

    proj3::msync(addr, size + bytes, proj3::MS_SYNC);
    proj3::munmap(addr, size + bytes);
    proj3::close(file);
}

void append(const char* path, size_t bytes) {
    int file = proj3::open(path, proj3::O_RDWR);
    if (file == -1) {
        return;
    }

    struct stat buf;
    proj3::fstat(file, &buf);
    size_t size0 = buf.st_size;
    size_t offset = size0;
    size_t remainingBytes = bytes;

    size_t chunkSize = (size0 == 0) ? bytes : size0;

    while (remainingBytes > 0) {
        size_t chunk = std::min(remainingBytes, chunkSize);
        proj3::ftruncate(file, offset + chunk);

        void* addr = proj3::mmap(nullptr,
            offset + chunk,
            proj3::PROT_READ | proj3::PROT_WRITE,
            proj3::MAP_SHARED,
            file,
            0);
        
        uint8_t* write = static_cast<uint8_t*>(addr) + offset;
        std::cin.read(reinterpret_cast<char*>(write), chunk);

        if (static_cast<size_t>(std::cin.gcount()) < chunk) {
            proj3::munmap(addr, offset + chunk);
            proj3::ftruncate(file, size0);
            proj3::close(file);
            return;
        }

        proj3::msync(addr, offset + chunk, proj3::MS_SYNC);
        proj3::munmap(addr, offset + chunk);
        offset += chunk;
        remainingBytes -= chunk;
    }

    proj3::close(file);
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
