#define _GNU_SOURCE
#include <linux/input.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
static volatile sig_atomic_t running=1, toggle=0, wake=0;
static int fb=-1,touch=-1,off=0,brightness=255; static pid_t refresh=0;
static void sig(int s){if(s==SIGUSR1)toggle=1;else if(s==SIGUSR2)wake=1;else running=0;}
static int number(const char *p){FILE*f=fopen(p,"r");int v=-1;if(f){if(fscanf(f,"%d",&v)!=1)v=-1;fclose(f);}return v;}
static int writebright(int n){char b[32];int l=snprintf(b,sizeof b,"%d\n",n);int f=open("/sys/class/leds/lcd-backlight/brightness",O_WRONLY);if(f<0)return -1;int r=write(f,b,l);close(f);return r==l?0:-1;}
static int refresh_signal(int s){int p=number("/run/r9st-fb-refresh.pid");char path[64],exe[256];if(p<=1)return -1;snprintf(path,sizeof path,"/proc/%d/exe",p);int n=readlink(path,exe,sizeof exe-1);if(n<0)return -1;exe[n]=0;if(strcmp(exe,"/usr/local/bin/r9st-fb-refresh"))return -1;refresh=p;return kill(refresh,s);}
static void screen_on(void){fprintf(stderr,"screen ON requested\n");if(ioctl(fb,FBIOBLANK,FB_BLANK_UNBLANK)<0)perror("unblank");writebright(brightness);refresh_signal(SIGCONT);if(touch>=0)ioctl(touch,EVIOCGRAB,0);off=0;unlink("/run/r9st-screen-off");fprintf(stderr,"screen ON complete\n");}
static int screen_off(void){int b=number("/sys/class/leds/lcd-backlight/brightness");if(b>0)brightness=b;if(touch<0||ioctl(touch,EVIOCGRAB,1)<0){perror("touch grab");return -1;}if(refresh_signal(SIGSTOP)<0){ioctl(touch,EVIOCGRAB,0);return -1;}fprintf(stderr,"screen OFF requested\n");if(ioctl(fb,FBIOBLANK,FB_BLANK_POWERDOWN)<0){perror("blank");screen_on();return -1;}writebright(0);off=1;int f=open("/run/r9st-screen-off",O_CREAT|O_WRONLY,0600);if(f>=0)close(f);fprintf(stderr,"screen OFF complete\n");return 0;}
int main(int argc,char**argv){int limit=argc>1?atoi(argv[1]):0;int lock=open("/run/r9st-keys.lock",O_CREAT|O_RDWR,0600);if(lock<0||flock(lock,LOCK_EX|LOCK_NB)<0)return 1;setbuf(stderr,NULL);struct pollfd p[16];int count=0;for(int i=0;i<32;i++){char path[64],name[128]={0};snprintf(path,sizeof path,"/dev/input/event%d",i);int fd=open(path,O_RDONLY|O_NONBLOCK);if(fd<0)continue;ioctl(fd,EVIOCGNAME(sizeof name),name);if(!strcmp(name,"synaptics-s3508")){touch=fd;continue;}if(strcmp(name,"qpnp_pon")&&strcmp(name,"gpio-keys")){close(fd);continue;}if(ioctl(fd,EVIOCGRAB,1)<0){perror("key grab");return 1;}p[count++]=(struct pollfd){fd,POLLIN,0};fprintf(stderr,"listening %s %s\n",path,name);}
 fb=open("/dev/fb0",O_RDWR);if(fb<0||touch<0||count!=2)return 1;struct fb_var_screeninfo v;if(ioctl(fb,FBIOGET_VSCREENINFO,&v)||v.xres!=1080||v.yres!=1920)return 1;
 signal(SIGTERM,sig);signal(SIGINT,sig);signal(SIGUSR1,sig);signal(SIGUSR2,sig);time_t since=0;
 while(running){poll(p,count,200);for(int i=0;i<count;i++){struct input_event e;while(read(p[i].fd,&e,sizeof e)==sizeof e){if(e.type!=EV_KEY)continue;fprintf(stderr,"key code=%u value=%d\n",e.code,e.value);if(e.code==KEY_POWER&&e.value==0)toggle=1;}}
 if(wake){wake=0;if(off)screen_on();}if(toggle){toggle=0;if(off)screen_on();else if(!screen_off())since=time(NULL);}if(off&&limit>0&&time(NULL)-since>=limit)screen_on();}
 if(off) { screen_on(); }
 for(int i=0;i<count;i++)close(p[i].fd);
 close(touch);close(fb);return 0;}
