#include <linux/fb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
static volatile sig_atomic_t running=1;
static void stop(int sig){(void)sig;running=0;}
int main(void){
 int fd=open("/dev/fb0",O_RDWR);struct fb_var_screeninfo v;
 if(fd<0||ioctl(fd,FBIOGET_VSCREENINFO,&v)){perror("fb");return 1;}
 if(v.xres!=1080||v.yres!=1920||v.bits_per_pixel!=32){fputs("Unexpected framebuffer geometry\n",stderr);return 2;}
 signal(SIGTERM,stop);signal(SIGINT,stop);
 struct timespec next;clock_gettime(CLOCK_MONOTONIC,&next);v.activate=FB_ACTIVATE_NOW;
 while(running){if(ioctl(fd,FBIOPAN_DISPLAY,&v)){perror("pan");return 1;}next.tv_nsec+=16666667;if(next.tv_nsec>=1000000000){next.tv_sec++;next.tv_nsec-=1000000000;}clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&next,0);}
 close(fd);return 0;
}
