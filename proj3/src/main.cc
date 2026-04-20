// Copyright 2026 Jordan Weinstein
// System
#include <sys/stat.h>

// Libraries
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>

// Headers
#include "proj3/lib/include/mmap.h"

/*
* Creates a new file.
* @param path - Path of file.
* @param fill_char - Character to fill file with.
* @param size - The size of the new file.
*/
void create(const char* path, char fill_char,
    size_t size) {
    // Opens (possibly truncates) and creates the new file with permission.
    int file = proj3::open(path,
        proj3::O_RDWR | proj3::O_CREAT | proj3::O_TRUNC, 0644);
    if (file == -1) {
        return;
    }

    // Resizes file to set size.
    proj3::ftruncate(file, size);
    // Assigns file to an address.
    void* addr = proj3::mmap(nullptr,
        size,
        proj3::PROT_READ | proj3::PROT_WRITE,
        proj3::MAP_SHARED,
        file,
        0);

    // Maps file to memory and synchronizes it.
    std::memset(addr, fill_char, size);
    proj3::msync(addr, size, proj3::MS_SYNC);

    // Unassigns file from their address and closes it.
    proj3::munmap(addr, size);
    proj3::close(file);
}

/*
* Inserts bytes into file.
* @param path - Path of file.
* @param offset - Where in file to insert bytes into.
* @param bytes - Bytes to insert.
*/
void insert(const char* path, size_t offset, size_t bytes) {
    // Opens the file.
    int file = proj3::open(path, proj3::O_RDWR);
    if (file == -1) {
        return;
    }

    // Obtains the file size.
    struct stat buf;
    proj3::fstat(file, &buf);
    size_t size = buf.st_size;
    if (offset > size) {
        return;
    }

    // Alters file size to include new bytes.
    proj3::ftruncate(file, size + bytes);
    // Assigns file to an address and sets a pointer to address.
    void* addr = proj3::mmap(nullptr,
        size + bytes,
        proj3::PROT_READ | proj3::PROT_WRITE,
        proj3::MAP_SHARED,
        file,
        0);
    uint8_t* ptr = static_cast<uint8_t*>(addr);

    // Creates room in file based off offset.
    std::memmove(ptr + offset + bytes,
        ptr + offset,
        size - offset);
    std::cin.read(reinterpret_cast<char*>(ptr + offset), bytes);

    // Handles if byte request is not satisfied.
    if (static_cast<size_t>(std::cin.gcount()) < bytes) {
        std::memmove(ptr + offset,
            ptr + offset + bytes,
            size - offset);
        proj3::munmap(addr, size + bytes);
        proj3::ftruncate(file, size);
        proj3::close(file);
        return;
    }

    // Clean up and closing of file.
    proj3::msync(addr, size + bytes, proj3::MS_SYNC);
    proj3::munmap(addr, size + bytes);
    proj3::close(file);
}

/*
* Appends bytes to end of file.
* @param path - Path of file.
* @param bytes - Bytes to append.
*/
void append(const char* path, size_t bytes) {
    // Opens the file.
    int file = proj3::open(path, proj3::O_RDWR);
    if (file == -1) {
        return;
    }

    // Obtains the file size and
    // the amount of bytes it has to go through.
    struct stat buf;
    proj3::fstat(file, &buf);
    size_t size0 = buf.st_size;
    size_t offset = size0;
    size_t remainingBytes = bytes;

    // Total amount of bytes to be handled in chunks.
    size_t chunkSize = (size0 == 0) ? bytes : size0;

    // Browses through all bytes until gets to end.
    while (remainingBytes > 0) {
        // Amount of bytes in a chunk to be handled.
        size_t chunk = std::min(remainingBytes, chunkSize);
        proj3::ftruncate(file, offset + chunk);

        // Assigns file to an address and finds a spot to write to.
        void* addr = proj3::mmap(nullptr,
            offset + chunk,
            proj3::PROT_READ | proj3::PROT_WRITE,
            proj3::MAP_SHARED,
            file,
            0);
        uint8_t* write = static_cast<uint8_t*>(addr) + offset;
        std::cin.read(reinterpret_cast<char*>(write), chunk);

        // Handles if chunk size request is not satisfied.
        if (static_cast<size_t>(std::cin.gcount()) < chunk) {
            proj3::munmap(addr, offset + chunk);
            proj3::ftruncate(file, size0);
            proj3::close(file);
            return;
        }

        // Clean up
        proj3::msync(addr, offset + chunk, proj3::MS_SYNC);
        proj3::munmap(addr, offset + chunk);
        // Counters
        offset += chunk;
        remainingBytes -= chunk;
    }

    // Closes the file.
    proj3::close(file);
}

int main(int argc, char* argv[]) {
    // Handles the argument given from user.
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
