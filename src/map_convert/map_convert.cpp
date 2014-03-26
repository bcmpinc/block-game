#include <cstring>
#include <cmath>
#include <string>
#include <fstream>
#include <iomanip>
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
    else filename++;
    strncpy(out, filename, n);
    out[n-1]=0;
    char * point = strrchr(out,'.');
    if (point) {
        point[0]=0;
    }
}

struct num {
    int head, fraction;
    num(float v) {
        if (v>=0) {
            int x = v*1000+0.5;
            head=x/1000;
            fraction=x%1000;
        } else {
            int x = v*-1000+0.5;
            head=-(x/1000);
            fraction=1000 - x%1000;
        }
    }
};
std::ostream& operator<<(std::ostream& o, const num &n) {
    o<<n.head;
    if (n.fraction) {
        o<<'.';
        if (n.fraction<100) o<<'0';
        if (n.fraction<10) o<<'0';
        int r = n.fraction;
        while (r%10==0) r/=10;
        o<<r;
    }
    return o;
};

struct position {
    float x,y,z;
    position(const position &p) : x(p.x), y(p.y), z(p.z) {}
    position(float x, float y, float z) : x(x), y(y), z(z) {}
};
std::ostream& operator<<(std::ostream& o, const position &p) {
    o << '{'<<num(p.x)<<','<<num(p.y)<<','<<num(p.z)<<'}';
    return o;
};

struct place_block {
    position pos;
    position size;
    int color;
    place_block(position pos, position size, int color) : pos(pos), size(size), color(color) {}  
};
std::ostream& operator<<(std::ostream& o, const place_block &b) {
    char c[8];
    snprintf(c, 8, "%06x", b.color);
    o << "place_block({pos="<<b.pos<<", size="<<b.size<<", color=0x"<<c<<"})";
    return o;
};

/**
 * Program to convert the legacy maps into lua format.
 */
int main(int argc, const char ** argv) {
    if (argc!=3) {
        printf("Usage: %s inputfile outputfile\n", argv[0]);
        return 1;
    }
    std::ofstream out(argv[2]);
    assert(out.is_open());
    
    char basename[64];
    get_basename(basename, argv[2], 64);

    filemap<block> blockfile(argv[1]);
    uint blocks = blockfile.length;
    
    out << "function next()"<<std::endl;
    out << "  load_map(\""<<basename<<".map\")"<<std::endl;
    out << "end"<<std::endl;
        
    for (uint i=0; i<blocks; i++) {
        const block &b = blockfile.list[i];
        float sizey = b.height/2;
        float posy = b.bottom + b.height/2;
        int color = ((b.color>>16)&0xff) | (b.color&0xff00) | ((b.color&0xff)<<16);
        if (i==0) {
            float gem_pos_y = posy + sizey + GEM_HEIGHT;
            out<<"place_gem({pos=";
            out<<position(b.posx,gem_pos_y,b.posz);
            out<<", record=\""<<basename<<".rec\", action=next})";
            out << std::endl;
        }
        float rotation = fmodf(b.rotation,180);
        if (rotation<0) rotation+=180;
        if (abs(rotation-90.0)<0.1) {
            place_block pb(position(b.posz, posy, b.posx), position(b.sizez, sizey, b.sizex), color);
            out << pb << std::endl;
        } else {
            place_block pb(position(b.posx, posy, b.posz), position(b.sizex, sizey, b.sizez), color);
            if (rotation>0.1 && rotation<179.9) {
                out << "block = " << pb << "; rotate_block(block, {angle=" << num(rotation) << "})" << std::endl;
            } else {
                out << pb << std::endl;
            }
        }
    }
}
