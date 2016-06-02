#include "world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>


/*
 * Prototypes des fonctions statiques contenues dans ce fichier C
 */

static void triangle_edge(GLfloat *im, int x, int y, int w, int h, int width);
static void triangleNormal(GLfloat * out, GLfloat * p0, GLfloat * p1, GLfloat * p2);
static void dataNormals(GLfloat * data, int w, int h);
static GLfloat * heightMap2Data(GLfloat * hm, int w, int h);
static GLuint * heightMapIndexedData(int w, int h);

static Chunk initData(int id);


static int  _landscape_w = 257, _landscape_h = 257;//default : 513
/*!\Taille de terrain */
static GLfloat _landscape_scale_xz = 100.0;//default : 100.0
static GLfloat _landscape_scale_y = 20.0;

static GLuint chunkRad = 0;

static GLuint _vPositionHandle, _vTextureHandle, _vNormalHandle;


#define EPSILON 0.00000001

static void triangle_edge(GLfloat *im, int x, int y, int w, int h, int width) {
    GLint v;
    GLint p[9][2], i, w_2 = w >> 1, w_21 = w_2 + (w&1), h_2 = h >> 1, h_21 = h_2 + (h&1);
    GLfloat ri = w / (GLfloat)width;
    p[0][0] = x;       p[0][1] = y;
    p[1][0] = x + w;   p[1][1] = y;
    p[2][0] = x + w;   p[2][1] = y + h;
    p[3][0] = x;       p[3][1] = y + h;
    p[4][0] = x + w_2; p[4][1] = y;
    p[5][0] = x + w;   p[5][1] = y + h_2;
    p[6][0] = x + w_2; p[6][1] = y + h;
    p[7][0] = x;       p[7][1] = y + h_2;
    p[8][0] = x + w_2; p[8][1] = y + h_2;
    for(i = 4; i < 8; i++) {
        if(im[p[i][0] + p[i][1] * width] > 0.0)
            continue;
        im[v = p[i][0] + p[i][1] * width] = (im[p[i - 4][0] + p[i - 4][1] * width] +
                                             im[p[(i - 3) % 4][0] + p[(i - 3) % 4][1] * width]) / 2.0;
        im[v] += gl4dmSURand() * ri;
        im[v] = MIN(MAX(im[v], EPSILON), 1.0);
    }
    if(im[p[i][0] + p[i][1] * width] < EPSILON) {
        im[v = p[8][0] + p[8][1] * width] = (im[p[0][0] + p[0][1] * width] +
                                             im[p[1][0] + p[1][1] * width] +
                                             im[p[2][0] + p[2][1] * width] +
                                             im[p[3][0] + p[3][1] * width]) / 4.0;
        im[v] += gl4dmSURand() * ri * sqrt(2);
        im[v] = MIN(MAX(im[v], EPSILON), 1.0);
    }
    if(w_2 > 1 || h_2 > 1)
        triangle_edge(im, p[0][0], p[0][1], w_2, h_2, width);
    if(w_21 > 1 || h_2 > 1)
        triangle_edge(im, p[4][0], p[4][1], w_21, h_2, width);
    if(w_21 > 1 || h_21 > 1)
        triangle_edge(im, p[8][0], p[8][1], w_21, h_21, width);
    if(w_2 > 1 || h_21 > 1)
        triangle_edge(im, p[7][0], p[7][1], w_2, h_21, width);
}


static void triangleNormal(GLfloat * out, GLfloat * p0, GLfloat * p1, GLfloat * p2) {
    GLfloat v0[3], v1[3];
    v0[0] = p1[0] - p0[0];
    v0[1] = p1[1] - p0[1];
    v0[2] = p1[2] - p0[2];
    v1[0] = p2[0] - p1[0];
    v1[1] = p2[1] - p1[1];
    v1[2] = p2[2] - p1[2];
    MVEC3CROSS(out, v0, v1);
    MVEC3NORMALIZE(out);
}

static void dataNormals(GLfloat * data, int w, int h) {
    int x, z, zw, i;
    GLfloat n[18];
    for(z = 1; z < h - 1; z++) {
        zw = z * w;
        for(x = 1; x < w - 1; x++) {
            triangleNormal(&n[0], &data[6 * (x + zw)], &data[6 * (x + 1 + zw)], &data[6 * (x + (z + 1) * w)]);
            triangleNormal(&n[3], &data[6 * (x + zw)], &data[6 * (x + (z + 1) * w)], &data[6 * (x - 1 + (z + 1) * w)]);
            triangleNormal(&n[6], &data[6 * (x + zw)], &data[6 * (x - 1 + (z + 1) * w)], &data[6 * (x - 1 + zw)]);
            triangleNormal(&n[9], &data[6 * (x + zw)], &data[6 * (x - 1 + zw)], &data[6 * (x + (z - 1) * w)]);
            triangleNormal(&n[12], &data[6 * (x + zw)], &data[6 * (x + (z - 1) * w)], &data[6 * (x + 1 + (z - 1) * w)]);
            triangleNormal(&n[15], &data[6 * (x + zw)], &data[6 * (x + 1 + (z - 1) * w)], &data[6 * (x + 1 + zw)]);
            data[6 * (x + zw) + 3] = 0;
            data[6 * (x + zw) + 4] = 0;
            data[6 * (x + zw) + 5] = 0;
            for(i = 0; i < 6; i++) {
                data[6 * (x + zw) + 3] += n[3 * i + 0];
                data[6 * (x + zw) + 4] += n[3 * i + 1];
                data[6 * (x + zw) + 5] += n[3 * i + 2];
            }
            data[6 * (x + zw) + 3] /= 6.0;
            data[6 * (x + zw) + 4] /= 6.0;
            data[6 * (x + zw) + 5] /= 6.0;
        }
    }
}

static GLfloat * heightMap2Data(GLfloat * hm, int w, int h) {
    int x, z, zw, i;
    GLfloat * data = malloc(6 * w * h * sizeof *data);
    double nx, nz, pnx = 2.0 / w, pnz = 2.0 / h;
    assert(data);
    for(z = 0, nz = 1.0; z < h; z++, nz -= pnz) {
        zw = z * w;
        for(x = 0, nx = -1.0; x < w; x++, nx += pnx) {
            i = 6 * (zw + x);

            data[i++] = (GLfloat)nx;
            data[i++] = 2.0 * hm[zw + x] - 1.0;
            data[i++] = (GLfloat)nz;
            i=i+3;
//            data[i++] = gl4dmURand();
//            data[i++] = gl4dmURand();
//            data[i++] = gl4dmURand();
        }
    }
    dataNormals(data, w, h);
    return data;
}




static GLuint * heightMapIndexedData(int w, int h) {
    int x, z, zw, i;
    GLuint * data = malloc(2 * 3 * (w - 1) * (h - 1) * sizeof *data);
    assert(data);
    for(z = 0; z < h - 1; z++) {
        zw = z * w;
        for(x = 0; x < w - 1; x++) {
            i = 2 * 3 * (z * (w - 1) + x);
            data[i++] = x + zw;
            data[i++] = x + zw + 1;
            data[i++] = x + zw + w;
            data[i++] = x + zw + w;
            data[i++] = x + zw + 1;
            data[i++] = x + zw + w + 1;

        }
    }

    return data;
}




World initWorld(int chunkRadius, GLuint posHandle, GLuint normHandle) {
    int i;

    chunkRad = chunkRadius;

    GLuint nbChunk = SQUARE((chunkRadius * 2) +1);

    _vPositionHandle = posHandle;
    _vNormalHandle = normHandle;

    LOGD("nbChunk: %d", nbChunk);


    world.nbChunk = nbChunk;
    world.worldChunks = malloc(nbChunk * sizeof(Chunk*));


    for(i=0; i<nbChunk; i++) {
        world.worldChunks[i] = initData(i);
    }

    return world;

}


static GLint currentXCoord1 = 0, currentXCoord2 = 0;
static GLint currentZCoord1 = 0, currentZCoord2 = 0;

static void chunkCoordinate(int id, int originx, int * xchunk, int * zchunk){
    if(id%2==0) {
        if(currentXCoord1==-chunkRad+originx) {
            currentXCoord1=originx+chunkRad;
            currentZCoord1--;
        }
        else  currentXCoord1--;

        *xchunk = currentXCoord1;
        *zchunk = currentZCoord1;
    }
    else {
        if(currentXCoord2==chunkRad+originx) {
            currentXCoord2=originx-chunkRad;
            currentZCoord2++;
        }
        else  currentXCoord2++;

        *xchunk = currentXCoord2;
        *zchunk = currentZCoord2;
    }
}


static int chunkId(int x, int z, int originx) {
    int i;


}


static Chunk initData(int id) {
    GLfloat *data = NULL;
    GLuint *idata = NULL;
    Chunk chunk;

    srand(time(NULL));
    chunk.data = calloc(_landscape_w * _landscape_h, sizeof chunk.data);
    assert(chunk.data);
    triangle_edge(chunk.data, 0, 0, _landscape_w - 1, _landscape_h - 1, _landscape_w);
    data = heightMap2Data(chunk.data, _landscape_w, _landscape_h);
    idata = heightMapIndexedData(_landscape_w, _landscape_h);
    glGenVertexArrays(1, &chunk.landscapeVao);
    glBindVertexArray(chunk.landscapeVao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glGenBuffers(2, chunk.landscapeBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, chunk.landscapeBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * _landscape_w * _landscape_h * sizeof *data, data,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(_vPositionHandle, 3, GL_FLOAT, GL_FALSE, 6 * sizeof *data,
                          (const void *) 0);
    glVertexAttribPointer(_vNormalHandle, 3, GL_FLOAT, GL_FALSE, 6 * sizeof *data,
                          (const void *) (3 * sizeof *data));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk.landscapeBuffer[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 2 * 3 * (_landscape_w - 1) * (_landscape_h - 1) * sizeof *idata, idata,
                 GL_STATIC_DRAW);


    free(data);
    free(idata);

    chunk.id = id;

    if(id == 0) {
        chunk.x = 0;
        chunk.z = 0;
    }
    else chunkCoordinate(id, 0, &chunk.x, &chunk.z);

    LOGD("loaded chunk %d at: %d;%d",id, chunk.x,chunk.z);

    return chunk;
}


Chunk chunkAt(GLfloat x, GLfloat z) {
    int chunkx = nearbyint ((x/_landscape_scale_xz) / 2);
    int chunkz = nearbyint ((z/_landscape_scale_xz) / 2);
    LOGD("chunkx: %d   chunkz: %d",chunkx,chunkz);

    int i;

    for(i=0; i<world.nbChunk; i++) {
        if(world.worldChunks[i].x == chunkx && world.worldChunks[i].z == chunkz)
            return world.worldChunks[i];
    }

}