#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "GL4D/gl4droid.h"
#include "image.h"
#include <assert.h>
#include <dlfcn.h>
#include <time.h>

#define  LOG_TAG    "RANDO"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define NBARBRES 20


/*
 * Prototypes des fonctions statiques contenues dans ce fichier C
 */

static void triangle_edge(GLfloat *im, int x, int y, int w, int h, int width);
static void initData(void);
static void setCamera();
static void draw(GLfloat *, GLfloat *);
static void loop(GLfloat*, GLfloat*);
//static void manageEvents(SDL_Window * win);


//static GLuint _program;
static GLuint _vPositionHandle, _vTextureHandle, _vNormalHandle;
static GLfloat _yScale = 1.0f;
static int _windowWidth, _windowHeight;
//static int _windowWidth = 800, _windowHeight = 600;

static int  _landscape_w = 256, _landscape_h = 256;//default : 513
/*!\Taille de terrain */
static GLfloat _landscape_scale_xz = 500.0;//default : 100.0
static GLfloat _landscape_scale_y = 100.0;//default : 20.0

/*!\brief identifiant des vertex array objects */
static GLuint _landscapeVao = 0;
/*!\brief identifiant des buffers de data */
static GLuint _landscapeBuffer[2] = {0};

/*!\brief identifiant du (futur) vertex array object */
static GLuint _vao[8] = {0,0,0,0,0,0,0,0};
/*!\brief identifiant du (futur) buffer de data */
static GLuint _buffer = 0;


/*!\brief identifiant du (futur) GLSL program */
static GLuint _pId = 0;
/*!\brief identifiant du (futur) GLSL program */
static GLuint _pId2[3] = {0,0};
static GLuint _pIdN[3] = {0,0};
/*!\brief identifiant de la texture */
static GLuint _tId[7] = {0,0,0,0,0,0,0};
static const GLfloat * data, dataLune[], dataSoleil[];
GLfloat camera[16], modelView[16], modelViewProjection[16];
GLfloat * eyeViews, *eyePerspectives;
GLfloat * headview, * forward, * up, * right;
float forward1, forward2;
static GLfloat _ratio_x = 1.0f, _ratio_y = 1.0f;

GLuint buffData, buffLune, buffSoleil;

GLfloat rayon, angle = M_PI/16.0;

GLuint texEau,texSable, texHerbe, texRoche, texNeige;
GLuint texMArbre, texGArbre;
GLuint texLune, texSoleil;

GLuint * viewports;

double Imax,Jmax,Id,I,Jd,J,Id1,Jd1;

static AAssetManager* asset_manager;


static GLuint _pause = 0;
static GLuint _activeToon = 0;
static GLuint _activeNight = 0;
uint32_t t0;
uint32_t t1, t2;

GLfloat pas;
GLuint pasOn = 0;

enum kyes_t {
    KLEFT = 0,
    KRIGHT,
    KUP,
    KDOWN
};
static GLuint _keys[] = {0, 0, 0, 0};

typedef struct cam_t cam_t;
struct cam_t {
    GLfloat x,y,z;
    GLfloat theta;
};

static cam_t _cam = {0, 0, 0};

static GLfloat * _hm = NULL;


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


static GLfloat hauteurMap(GLfloat x , GLfloat y){
    x = (_landscape_w /2) + (x / _landscape_scale_xz) * (_landscape_w/2.0);
    y = (_landscape_h /2) - (y / _landscape_scale_xz) * (_landscape_h/2.0);

    if(x >= 0.0 && x<_landscape_w && y>0.0 && y<_landscape_h)
        return (2.0 * _hm[((int)x) +((int)y) * _landscape_w] -1) * _landscape_scale_y;

    return 0;

}

static void reshape() {

    gl4duBindMatrix("projectionMatrix");
    gl4duLoadIdentityf();

    //gl4duFrustumf(-0.5, 0.5, -0.5 * _windowHeight / _windowWidth, 0.5 * _windowHeight / _windowWidth, 1.0, 1000.0);
}

static int init(const char * vs, const char * fs, const char * toons, const char * fnights, const char * fnighttoons) {
    gl4dInitTime0();

    LOGD("Version d'OpenGL : %s", glGetString(GL_VERSION));
    LOGD("Version de shaders supportes : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    LOGD("Init c");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    _pId = gl4droidCreateProgram(vs, toons);

    if(!_pId) return 0;


    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
  //  glClearDepthf(1.0f);
    /* Enables Depth Testing */
   // glEnable(GL_DEPTH_TEST);
    /* The Type Of Depth Test To Do */
   // glDepthFunc(GL_LEQUAL);
    gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
    gl4duGenMatrix(GL_FLOAT, "projectionMatrix");

    reshape();

    _vPositionHandle = glGetAttribLocation(_pId, "vsiPosition");
    _vNormalHandle = glGetAttribLocation(_pId, "vsiNormal");
    _vTextureHandle = glGetAttribLocation(_pId, "vsiTexCoord");

    LOGD("created programs");

    LOGD("_pId: %d   _vPositionHandle: %d   _vNormalHandle: %d    _vTextureHandle: %d",
         _pId, _vPositionHandle, _vNormalHandle, _vTextureHandle);


    initData();

    return 1;
}



GLuint load_texture(GLuint texture_object_id, const GLsizei width, const GLsizei height,const GLenum type, const GLvoid* pixels) {

    glGenTextures(1, &texture_object_id);
    assert(texture_object_id != 0);

    glBindTexture(GL_TEXTURE_2D, texture_object_id);

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(
            GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture_object_id;
}

GLuint load_png_asset_into_texture(const char* relative_path, GLuint texture_id) {
    assert(relative_path != NULL);
    LOGD("init loading image %s", relative_path);
    const FileData png_file = get_asset_data(asset_manager, relative_path);
    LOGD("getting raw image");
    const RawImageData raw_image_data =
            get_raw_image_data_from_png(png_file.data, png_file.data_length);
    const GLuint texture_object_id = load_texture(
            texture_id,
            raw_image_data.width, raw_image_data.height,
            raw_image_data.gl_color_format, raw_image_data.data);

    release_raw_image_data(&raw_image_data);
    release_asset_data(&png_file);

    LOGD("fini loading image %s", relative_path);
    return texture_object_id;
}

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

static void initData(void) {
    GLfloat * data = NULL;
    GLuint * idata = NULL;

    srand(time(NULL));
    _hm = calloc(_landscape_w * _landscape_h, sizeof *_hm);
    assert(_hm);
    triangle_edge(_hm, 0, 0, _landscape_w - 1, _landscape_h - 1, _landscape_w);
    data = heightMap2Data(_hm, _landscape_w, _landscape_h);
    idata = heightMapIndexedData(_landscape_w, _landscape_h);
    glGenVertexArrays(1, &_landscapeVao);
    glBindVertexArray(_landscapeVao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glGenBuffers(2, _landscapeBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _landscapeBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * _landscape_w * _landscape_h * sizeof *data, data, GL_STATIC_DRAW);
    glVertexAttribPointer(_vPositionHandle, 3, GL_FLOAT, GL_FALSE, 6 * sizeof *data, (const void *)0);
    glVertexAttribPointer(_vNormalHandle, 3, GL_FLOAT, GL_FALSE, 6 * sizeof *data, (const void *)(3 * sizeof *data));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _landscapeBuffer[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * (_landscape_w - 1) * (_landscape_h - 1) * sizeof *idata, idata, GL_STATIC_DRAW);




    LOGD("sizehm: %d",  sizeof *_hm);

//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
//    glBindVertexArray(0);

    free(data);
    free(idata);


    //TEXTURE EAU///////////////////////////////////////////////////


    if( (texEau = load_png_asset_into_texture("image/eau.png", texEau)) == 0){
        LOGD("Impossible d'ouvrir le fichier : %s", "image/eau.jpg");
        exit(1);
    }

    //TEXTURE SABLE////////////////////////////////////////////

    if( (texSable = load_png_asset_into_texture("image/sable.png", texSable)) == 0) {
        LOGD("Impossible d'ouvrir le fichier : %s", "image/sable.png");
        exit(1);
    }

    //TEXTURE HERBE////////////////////////////////////////////////

    if( (texHerbe = load_png_asset_into_texture("image/herbe.png", texHerbe)) == 0){
        LOGD("Impossible d'ouvrir le fichier : %s", "image/herbe.png");
        exit(1);
    }

    //TEXTURE ROCHE////////////////////////////////////////////////

    if( (texRoche = load_png_asset_into_texture("image/roche.png", texRoche)) == 0){
        LOGD("Impossible d'ouvrir le fichier : %s", "image/roche.png");
        exit(1);
    }

    //TEXTURE NEIGE////////////////////////////////////////////////

    if( (texNeige = load_png_asset_into_texture("image/neige.png", texNeige)) == 0){
        LOGD("Impossible d'ouvrir le fichier : %s", "image/neige.png");
        exit(1);
    }
}


/*!\brief Boucle infinie principale : g�re les �v�nements, dessine,
 * imprime le FPS et swap les buffers.
 *
 * \param win le pointeur vers la fen�tre SDL pour laquelle nous avons
 * attach� le contexte OpenGL.
 */
static void draw(GLfloat * eyeViews, GLfloat * eyePerspectives) {

    setCamera();

    loop(eyeViews, eyePerspectives);
    gl4duPrintFPS(stderr);

    gl4duUpdateShaders();
}


void printMat(GLfloat * mat, char* name) {
    LOGD("%s0: %.2f  %s1: %.2f  %s2: %.2f "
                 " %s3: %.2f  %s4: %.2f  %s5: %.2f "
                 " %s6: %.2f  %s7: %.2f  %s8: %.2f "
                 " %s9: %.2f  %s10: %.2f %s11: %.2f "
                 " %s12: %.2f  %s13: %.2f %s14: %.2f "
                 "  %s15: %.2f",
         name,mat[0],name,mat[1],name, mat[2],
         name,mat[3],name,mat[4],name,mat[5],
         name,mat[6],name,mat[7],name,mat[8],
         name,mat[9],name, mat[10],name,mat[11],
         name,mat[12], name,mat[13],name,mat[14],
         name,mat[15]);
}


void setViewport() {
    glViewport(viewports[0],viewports[1], viewports[2], viewports[3]);
    //  LOGD("Viewport : x= %d  y=%d  width=%d  height=%d", viewports[0],viewports[1], viewports[2], viewports[3]);
    glScissor(viewports[0],viewports[1], viewports[2], viewports[3]);

}


void setCamera() {
    double dt, dtheta = M_PI, pas = 5.0;
    static double t0 = 0, t;
    dt = ((t = gl4dGetElapsedTime()) - t0) / 1000.0;
    t0 = t;
   // LOGD("gl4dGetElapsedTime: %f",t);

   // LOGD("forx: %.2f   forz: %.2f", (GLfloat) forward1, (GLfloat) forward2);


    if(_keys[KLEFT]) {
        _cam.theta += dt * dtheta;
    }
    if(_keys[KRIGHT]) {
        _cam.theta -= dt * dtheta;
    }

   // LOGD("theta: %.2f   sin(_cam.theta):  %.2f       cos(_cam.theta):  %.2f ", _cam.theta, sin(_cam.theta), cos(_cam.theta));
   if(_keys[KUP]) {
       _cam.x += -dt * pas * forward1;
       _cam.z += -dt * pas * -forward2;
    }
    if(_keys[KDOWN]) {
        _cam.x += dt * pas * forward1;
        _cam.z += dt * pas * -forward2;
    }

}


#define BUFFER_OFFSET(i) ((void*)(i))
static void loop(GLfloat * eyeViews, GLfloat * eyePerspectives) {
    int xm, ym;


    static GLfloat temps = 0.0f;
    GLfloat * mv, temp[4] = {1.0, 100*sin(temps), 1.0, 1.0};
    temps += 0.01;

    GLfloat lumpos[4] = {100, 1000, 100, 1.0};

//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_BACK);
    //  glClearDepthf(1.0f);
    /* Enables Depth Testing */
    // glEnable(GL_DEPTH_TEST);
    /* The Type Of Depth Test To Do */
    // glDepthFunc(GL_LEQUAL);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(_pId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texEau);
    glUniform1i(glGetUniformLocation(_pId, "myTexture0"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,texSable);
    glUniform1i(glGetUniformLocation(_pId, "myTexture1"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,texHerbe);
   glUniform1i(glGetUniformLocation(_pId, "myTexture2"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D,texRoche);
   glUniform1i(glGetUniformLocation(_pId, "myTexture3"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D,texNeige);
    glUniform1i(glGetUniformLocation(_pId, "myTexture4"), 4);


    gl4duBindMatrix("modelViewMatrix");
    gl4duLoadIdentityf();

   mv = gl4duGetMatrixData();
    MMAT4XVEC4(lumpos, mv, temp);
    glUniform4fv(glGetUniformLocation(_pId, "lumpos"), 1, lumpos);
    glUniformMatrix4fv(glGetUniformLocation(_pId, "mv"), 1, GL_FALSE, mv);

   MMAT4INVERSE(eyeViews);

    GLfloat altitude = hauteurMap(_cam.x , _cam.z)+3.0;

   // eyeViews[]


    gl4duMultMatrixf(eyeViews);

   gl4duTranslatef(-_cam.x, -altitude, -_cam.z);


   // gl4duRotatef(-90,1, 0, 0);
    LOGD("x: %f  alt: %f   z: %f",-_cam.x,-altitude,-_cam.z);


  //  gl4duPopMatrix();

//    gl4duLookAtf(_cam.x, altitude, _cam.z,
//                 _cam.x + forward1, (altitude + 3.0) - forward[1], _cam.z + forward2,
//                 eyeViews[1], eyeViews[5],eyeViews[9]);


    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    gl4duScalef(_landscape_scale_xz,_landscape_scale_y,_landscape_scale_xz);



    gl4duSendMatrices();

    glUniformMatrix4fv(glGetUniformLocation(_pId, "perspective"), 1, GL_FALSE, eyePerspectives);


    glBindVertexArray(_landscapeVao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _landscapeBuffer[1]);
    glDrawElements(GL_TRIANGLES, 2 * 3 * (_landscape_w - 1) * (_landscape_h - 1), GL_UNSIGNED_INT,0);
   // gl4duPopMatrix();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


static void quit() {
    if(_vao[0]) {
        glDeleteBuffers(8, _vao);
        _vao[0] = 0;
    }
    if(_tId[0]) {
        glDeleteTextures(sizeof _tId / sizeof *_tId, _tId);
        _tId[0] = 0;
    }

    if(_hm) {
        free(_hm);
        _hm = NULL;
    }
    if(_landscapeVao)
        glDeleteVertexArrays(1, &_landscapeVao);
    if(_landscapeBuffer[0])
        glDeleteBuffers(2, _landscapeBuffer);
    gl4duClean(GL4DU_ALL);
    // _pId[0] = _pId[1] = 0;
    _pId = 0;
}


JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_init(JNIEnv * env, jobject obj
        , jobject assetManager, jstring vshader, jstring fshader, jstring toonshader, jstring fnightbasicshader, jstring fnightbasictoonshader) {
    LOGD("Init java");

    asset_manager = AAssetManager_fromJava(env, assetManager);

    char * vs = (*env)->GetStringUTFChars(env, vshader, NULL);
    char * fs = (*env)->GetStringUTFChars(env, fshader, NULL);
    char * toonss = (*env)->GetStringUTFChars(env, toonshader, NULL);
    char * fnights = (*env)->GetStringUTFChars(env, fnightbasicshader, NULL);
    char * fnighttoons = (*env)->GetStringUTFChars(env, fnightbasictoonshader, NULL);

    init(vs, fs, toonss, fnights, fnighttoons);
    (*env)->ReleaseStringUTFChars(env, vshader, vs);
    (*env)->ReleaseStringUTFChars(env, fshader, fs);
    (*env)->ReleaseStringUTFChars(env, toonshader, toonss);
    (*env)->ReleaseStringUTFChars(env, fnightbasicshader, fnights);
    (*env)->ReleaseStringUTFChars(env, fnightbasictoonshader, fnighttoons);
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_reshape(JNIEnv * env, jobject obj,  jint width, jint height) {
    _windowWidth  = width;
    _windowHeight = height;

    reshape();
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_setviewport(JNIEnv * env, jobject obj,  jintArray viewport) {
    viewports = (*env)->GetIntArrayElements(env, viewport, NULL);

    setViewport();
    (*env)->ReleaseIntArrayElements(env, viewport, viewports, 0);

}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_draw(JNIEnv * env, jobject obj, jfloatArray eyeView, jfloatArray eyePerspective) {
    eyeViews = (*env)->GetFloatArrayElements(env, eyeView, NULL);
    eyePerspectives = (*env)->GetFloatArrayElements(env, eyePerspective, NULL);

    draw(eyeViews, eyePerspectives);

    (*env)->ReleaseFloatArrayElements(env, eyeView, eyeViews, 0);
    (*env)->ReleaseFloatArrayElements(env, eyePerspective, eyePerspectives, 0);
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_setcamera(JNIEnv * env, jobject obj
        , jfloatArray headviewv,jfloatArray forwardv, jfloatArray upv, jfloatArray rightv, jfloat forward1v, jfloat forward2v) {

    forward1 = forward1v; forward2 = forward2v;

    headview = (*env)->GetFloatArrayElements(env, headviewv, NULL);
    forward = (*env)->GetFloatArrayElements(env, forwardv, NULL);
    up = (*env)->GetFloatArrayElements(env, upv, NULL);
    right = (*env)->GetFloatArrayElements(env, rightv, NULL);

    (*env)->ReleaseFloatArrayElements(env, headviewv, headview, 0);
    (*env)->ReleaseFloatArrayElements(env, forwardv, forward, 0);
    (*env)->ReleaseFloatArrayElements(env, upv, up, 0);
    (*env)->ReleaseFloatArrayElements(env, rightv, right, 0);

}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_upKeyEvent(JNIEnv * env, jobject obj, jint up) {

     _keys[KUP] = up;

   // LOGD("upkey: %d",_keys[KUP]);
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_downKeyEvent(JNIEnv * env, jobject obj, jint down) {

     _keys[KDOWN] = down;

   // LOGD("downkey: %d",_keys[KDOWN]);
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_leftKeyEvent(JNIEnv * env, jobject obj, jint left) {

    _keys[KLEFT] = left;

   // LOGD("leftkey: %d",_keys[KLEFT]);
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_rightKeyEvent(JNIEnv * env, jobject obj, jint right) {

    _keys[KRIGHT] = right;

   // LOGD("rightkey: %d",_keys[KRIGHT]);
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_quit(JNIEnv * env, jobject obj) {
    quit();
}