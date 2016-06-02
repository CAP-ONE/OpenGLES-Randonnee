#include "GL4D/gl4droid.h"


#define  LOG_TAG    "WORLD"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef struct {
    int x;
    int z;
    int id;
    GLfloat * data;
    GLuint landscapeVao;
    GLuint landscapeBuffer[2];
} Chunk;


typedef struct {
    Chunk * worldChunks;
    GLuint nbChunk;
} World;

World world;


World initWorld(int,GLuint,GLuint);
Chunk chunkAt(GLfloat, GLfloat);