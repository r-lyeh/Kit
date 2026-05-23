#if 0

extern struct folder {
    text* (*list)(const char *path, const char *filter, int *count); // use ** for subdirs. examples: "*", "**", "**.png", "*woods*/**.png", etc
} folder;

extern struct content { // archive?
    bool (*mount)(const char *url);
    text (*find)(const char *url);
    text (*load)(const char *url, int *len); // .load_async() ?
    text (*cook)(const char *url, int *len); // .load_async() ?
} content;

extern struct asset {

} asset;

extern struct registry { // types, reflection

} registry;

content -> assets
registry -> types

https://wiki.libsdl.org/SDL3/CategoryStorage

extern struct storage {
    bool (*open)(text url); // Called when the storage is closed
SDL_OpenFileStorage ; local files, debug only
SDL_OpenTitleStorage ; kept open during game
SDL_OpenUserStorage ; savegames, settings
    bool (*close)(void);    // Called when the storage is closed
SDL_CloseStorage

    bool (*enumerate)(const char *path, SDL_EnumerateDirectoryCallback callback, void *callback_userdata); // Enumerate a directory, optional for write-only storage

    bool (*info)(const char *path, SDL_PathInfo *info); // Get path information, optional for write-only storage

    bool (*read_file)(const char *path, void *destination, Uint64 length); // Read a file from storage, optional for write-only storage
 SDL_ReadStorageFile - ro
    bool (*write_file)(const char *path, const void *source, Uint64 length); // Write a file to storage, optional for read-only storage
 SDL_WriteStorageFile - rw
    file_size
 SDL_GetStorageFileSize
    path_info
 SDL_GetStoragePathInfo

    bool (*md)(const char *path); // Create a directory, optional for read-only storage
 SDL_CreateStorageDirectory - md
    bool (*rm)(const char *path); // Remove a file or empty directory, optional for read-only storage
 SDL_RemoveStoragePath - rm
    bool (*mv)(const char *oldpath, const char *newpath); // Rename a path, optional for read-only storage
 SDL_RenameStoragePath - mv
    bool (*cp)(const char *oldpath, const char *newpath); // Copy a file, optional for read-only storage
 SDL_CopyStorageFile - cp

    Uint64 (*space_remaining)(void *userdata); // Get the space remaining, optional for read-only storage
 SDL_GetStorageSpaceRemaining
} SDL_StorageInterface;

#endif

