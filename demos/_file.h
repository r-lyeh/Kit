// mmap_file.h
#ifndef MMAP_FILE_H
#define MMAP_FILE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MMapFile {
    void*    data;      // pointer to mapped memory
    size_t   size;      // size in bytes
    void*    handle;    // platform-specific handle (opaque)
    void*    file;      // platform-specific file handle (opaque)
    int      readonly;  // 1 if opened read-only
} MMapFile;

/**
 * Open or create a memory-mapped file.
 * 
 * @param filename Path to the file
 * @param size     Desired size in bytes (if creating or extending)
 * @param readonly If true, open in read-only mode
 * @return Pointer to MMapFile struct, or NULL on failure
 */
MMapFile* mmap_file_open(const char* filename, size_t size, int readonly);

/**
 * Get pointer to the mapped data.
 */
static inline void* mmap_file_data(MMapFile* mf) {
    return mf ? mf->data : NULL;
}

/**
 * Get size of the mapped region.
 */
static inline size_t mmap_file_size(MMapFile* mf) {
    return mf ? mf->size : 0;
}

/**
 * Close and release the memory-mapped file.
 */
void mmap_file_close(MMapFile* mf);

#ifdef __cplusplus
}
#endif

#endif // MMAP_FILE_H


// mmap_file.c
//#include "mmap_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

MMapFile* mmap_file_open(const char* filename, size_t size, int readonly) {
    MMapFile* mf = (MMapFile*)calloc(1, sizeof(MMapFile));
    if (!mf) return NULL;

    mf->size = size;
    mf->readonly = readonly;

#ifdef _WIN32
    // ==================== Windows ====================
    DWORD access = readonly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
    DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD disposition = OPEN_ALWAYS;
    DWORD protect = readonly ? PAGE_READONLY : PAGE_READWRITE;
    DWORD map_access = readonly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;

    HANDLE hFile = CreateFileA(filename, access, share, NULL, disposition,
                               FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateFileA failed: %lu\n", GetLastError());
        free(mf);
        return NULL;
    }

    // Extend file if needed
    if (!readonly) {
        LARGE_INTEGER li;
        li.QuadPart = size;
        if (!SetFilePointerEx(hFile, li, NULL, FILE_BEGIN) || !SetEndOfFile(hFile)) {
            fprintf(stderr, "SetEndOfFile failed: %lu\n", GetLastError());
            CloseHandle(hFile);
            free(mf);
            return NULL;
        }
    }

    HANDLE hMapping = CreateFileMappingA(hFile, NULL, protect, 0, (DWORD)size, NULL);
    if (hMapping == NULL) {
        fprintf(stderr, "CreateFileMappingA failed: %lu\n", GetLastError());
        CloseHandle(hFile);
        free(mf);
        return NULL;
    }

    void* data = MapViewOfFile(hMapping, map_access, 0, 0, size);
    if (data == NULL) {
        fprintf(stderr, "MapViewOfFile failed: %lu\n", GetLastError());
        CloseHandle(hMapping);
        CloseHandle(hFile);
        free(mf);
        return NULL;
    }

    mf->data = data;
    mf->handle = hMapping;
    mf->file = hFile;

#else
    // ==================== POSIX (Linux / macOS) ====================
    int flags = readonly ? O_RDONLY : (O_RDWR | O_CREAT);
    int prot = readonly ? PROT_READ : (PROT_READ | PROT_WRITE);
    int mmap_flags = MAP_SHARED;

    int fd = open(filename, flags, 0600);
    if (fd == -1) {
        perror("open");
        free(mf);
        return NULL;
    }

    // Extend file if not read-only
    if (!readonly && ftruncate(fd, size) == -1) {
        perror("ftruncate");
        close(fd);
        free(mf);
        return NULL;
    }

    void* data = mmap(NULL, size, prot, mmap_flags, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        free(mf);
        return NULL;
    }

    mf->data = data;
    mf->handle = (void*)(intptr_t)fd;   // store fd in handle
    mf->file = NULL;

#endif

    return mf;
}

void mmap_file_close(MMapFile* mf) {
    if (!mf) return;

#ifdef _WIN32
    if (mf->data) UnmapViewOfFile(mf->data);
    if (mf->handle) CloseHandle((HANDLE)mf->handle);
    if (mf->file) CloseHandle((HANDLE)mf->file);
#else
    if (mf->data) munmap(mf->data, mf->size);
    if (mf->handle) close((int)(intptr_t)mf->handle);
#endif

    free(mf);
}



// demo

#include <stdio.h>

int main(void) {
    const size_t len = 1000 * sizeof(uint32_t);

    MMapFile* mf = mmap_file_open("numbers.u32", len, 0); // 0 = read/write

    if (!mf) {
        fprintf(stderr, "Failed to open memory-mapped file\n");
        return 1;
    }

    uint32_t* numbers = (uint32_t*)mmap_file_data(mf);

    printf("Before: numbers[42] = %u\n", numbers[42]);
    numbers[42]++;
    printf("After : numbers[42] = %u\n", numbers[42]);

    mmap_file_close(mf);
    return 0;
}
