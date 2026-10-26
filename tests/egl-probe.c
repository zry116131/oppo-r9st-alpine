#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
static void fail(const char *s){fprintf(stderr,"%s EGL=%x GL=%x\n",s,eglGetError(),glGetError());exit(1);}
static GLuint shader(GLenum type,const char *src){GLuint s=glCreateShader(type);glShaderSource(s,1,&src,NULL);glCompileShader(s);GLint ok;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){char log[2048];glGetShaderInfoLog(s,sizeof(log),NULL,log);fprintf(stderr,"%s\n",log);fail("shader");}return s;}
int main(void){
 PFNEGLGETPLATFORMDISPLAYEXTPROC get=(void*)eglGetProcAddress("eglGetPlatformDisplayEXT");
 if(!get)fail("getPlatformDisplay");
 EGLDisplay d=get(EGL_PLATFORM_SURFACELESS_MESA,EGL_DEFAULT_DISPLAY,NULL);EGLint major,minor;
 if(!eglInitialize(d,&major,&minor))fail("initialize");
 printf("EGL=%d.%d vendor=%s\n",major,minor,eglQueryString(d,EGL_VENDOR));
 EGLint attrs[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};EGLConfig cfg;EGLint n;
 if(!eglChooseConfig(d,attrs,&cfg,1,&n)||!n)fail("config");
 EGLint ca[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE},pa[]={EGL_WIDTH,64,EGL_HEIGHT,64,EGL_NONE};
 eglBindAPI(EGL_OPENGL_ES_API);EGLContext c=eglCreateContext(d,cfg,EGL_NO_CONTEXT,ca);EGLSurface surf=eglCreatePbufferSurface(d,cfg,pa);
 if(c==EGL_NO_CONTEXT||surf==EGL_NO_SURFACE||!eglMakeCurrent(d,surf,surf,c))fail("context");
 const char *renderer=(const char*)glGetString(GL_RENDERER);printf("renderer=%s GL=%s\n",renderer,glGetString(GL_VERSION));
 if(!renderer||(!strstr(renderer,"FD506")&&!strstr(renderer,"Adreno (TM) 506")))fail("not A506 hardware renderer");
 GLuint p=glCreateProgram(),v=shader(GL_VERTEX_SHADER,"attribute vec2 pos; void main(){gl_Position=vec4(pos,0.0,1.0);}"),f=shader(GL_FRAGMENT_SHADER,"precision mediump float; void main(){gl_FragColor=vec4(0.25,0.5,0.75,1.0);}");
 glAttachShader(p,v);glAttachShader(p,f);glBindAttribLocation(p,0,"pos");glLinkProgram(p);GLint ok;glGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok)fail("link");glUseProgram(p);
 const GLfloat verts[]={-1,-1,3,-1,-1,3};glViewport(0,0,64,64);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,verts);glEnableVertexAttribArray(0);
 for(int i=0;i<30;i++){glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);glDrawArrays(GL_TRIANGLES,0,3);glFinish();if(glGetError()!=GL_NO_ERROR)fail("draw");}
 const int xy[][2]={{0,0},{63,0},{32,32},{0,63},{63,63}};
 for(int i=0;i<5;i++){unsigned char px[4]={0};glReadPixels(xy[i][0],xy[i][1],1,1,GL_RGBA,GL_UNSIGNED_BYTE,px);printf("pixel[%d]=%u,%u,%u,%u\n",i,px[0],px[1],px[2],px[3]);if(abs(px[0]-64)>2||abs(px[1]-128)>2||abs(px[2]-191)>2||px[3]!=255)fail("pixel mismatch");}
 eglMakeCurrent(d,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);eglDestroySurface(d,surf);eglDestroyContext(d,c);eglTerminate(d);puts("PASS: 30 shader draws and five pixel checks");return 0;
}
