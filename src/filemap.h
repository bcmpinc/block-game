#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

template <class T>
struct filemap {
    uint32_t size; /// Number of bytes in the filemap.
    uint32_t length; /// Number of T's in the filemap.
    int32_t fd;
    const T * list;
    filemap(const char* filename);
    filemap(filemap&) = delete;
    filemap& operator=(filemap&) = delete;
    ~filemap();
};

template <class T>
filemap<T>::filemap(const char* filename) {
    /* writable
    fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) write = false;
    */
    // read-only:
    fd = open(filename, O_RDONLY);
        
    if (fd == -1) {perror("Could not open file"); exit(1);}
    size = lseek(fd, 0, SEEK_END);
    assert(size % sizeof(T) == 0);
    length = size / sizeof(T);
    list = (const T*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (list == MAP_FAILED) {perror("Could not map file to memory"); exit(1);} 
}

template <class T>
filemap<T>::~filemap() {
    if (list!=MAP_FAILED)
        munmap((void*)list, size);
    if (fd!=-1)
        close(fd);
}