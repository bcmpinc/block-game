/*
    Block Game - A minimalistic 3D platform game
    Copyright (C) 2014  B.J. Conijn <bcmpinc@users.sourceforge.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cassert>

template <class T>
struct filemap {
    uint32_t size; /// Number of bytes in the filemap.
    uint32_t length; /// Number of T's in the filemap.
    int32_t fd;
    const T * list;
    filemap() : size(0), length(0), fd(-1), list((const T*)MAP_FAILED) {}
    filemap(const char* filename);
    filemap(filemap&) = delete;
    filemap(filemap&& src) : size(src.size), length(src.length), fd(src.fd), list(src.list) {
        src.fd = -1;
        src.list = (const T*)MAP_FAILED;
    }
    filemap& operator=(filemap&) = delete;
    void operator=(filemap&& src) {
        this->~filemap();
        size = src.size;
        length = src.length;
        fd = src.fd; 
        list = src.list;
        src.fd = -1;
        src.list = (const T*)MAP_FAILED;
    }
    ~filemap();
};

template <class T>
filemap<T>::filemap(const char* filename) : filemap() {
    /* writable
    fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) write = false;
    */
    // read-only:
    fd = open(filename, O_RDONLY);
        
    if (fd == -1) {perror("Could not open file"); return;}
    size = lseek(fd, 0, SEEK_END);
    assert(size % sizeof(T) == 0);
    length = size / sizeof(T);
    list = (const T*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (list == MAP_FAILED) {
        perror("Could not map file to memory"); 
        close(fd);
        fd = -1;
        size=0; 
        length=0;
    } 
}

template <class T>
filemap<T>::~filemap() {
    if (list!=MAP_FAILED)
        munmap((void*)list, size);
    if (fd!=-1)
        close(fd);
}