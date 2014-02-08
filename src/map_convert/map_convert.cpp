#include <cstring>
#include <cmath>
#include <string>
#include "filemap.h"

static const double GEM_HEIGHT = 0.4; 

struct block {
    float posz;
    float posx;
    float sizex;
    float sizez;
    float height;
    float bottom;
    float rotation; // this is in degrees.
    uint32_t color;
};

void get_basename(char * out, const char * path, size_t n) {
    const char * filename = strrchr(path,'/');
    if (!filename) filename = path;
    strncpy(out, filename, n);
    out[n]=0;
    char * point = strrchr(out,'.');
    if (point) {
        point[0]=0;
    }
}

#define PLACE_BLOCK "place_block({pos={%5.1f,%5.1f,%5.1f}, size={%4.1f,%4.1f,%4.1f}, color=0x%06x})"        

/**
 * Program to convert the legacy maps into lua format.
 */
int main(int argc, const char ** argv) {
    if (argc!=3) {
        printf("Usage: %s inputfile outputfile", argv[0]);
        return 1;
    }
    FILE * out = fopen(argv[2], "w");
    char basename[64];
    get_basename(basename, argv[2], 64);

    filemap<block> blockfile(argv[1]);
    uint blocks = blockfile.length;
    
    for (uint i=0; i<blocks; i++) {
        const block &b = blockfile.list[i];
        float sizey = b.height/2;
        float posy = b.bottom + b.height/2;
        int color = ((b.color>>16)&0xff) | (b.color&0xff00) | ((b.color&0xff)<<16);
        // printf("%6.3f %6.3f %6.3f %06x\n", b.posx, posy, b.posz, b.color);
        if (i==0) {
            float gem_pos_y = posy + sizey + GEM_HEIGHT;
            fprintf(out, "place_gem({pos={%.1f,%.1f,%.1f}, record=\"%s.rec\"})\n", b.posx, gem_pos_y, b.posz, basename);
        }
        float rotation = fmodf(b.rotation,180);
        if (rotation<0) rotation+=180;
        if (abs(rotation-90.0)<0.1) {
            fprintf(out, PLACE_BLOCK "\n", b.posz, posy, b.posx, b.sizez, sizey, b.sizex, color);
        } else {
            if (rotation>0.1 && rotation<179.9) {
                fprintf(out, "block = " PLACE_BLOCK "; ", b.posx, posy, b.posz, b.sizex, sizey, b.sizez, color);
                fprintf(out, "rotate_block(block, {angle=%5.1f})\n", rotation);
            } else {
                fprintf(out, PLACE_BLOCK "\n", b.posx, posy, b.posz, b.sizex, sizey, b.sizez, color);
            }
        }
    }
}
