
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gpio_backend.h"
 
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF        64

typedef struct
{
    Gpio_t base;
    int fd;
    int mode;
    int pinValue;
}GpioSysfs_t;


static int gpioExport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) 
    {
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}


static int gpioUnexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}


static int gpioSetDir(unsigned int gpio, unsigned int mode)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		return fd;
	}
 
	if (mode == GpioMode_OUTPUT)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}


static int gpioOpen(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY | O_NONBLOCK );

	return fd;
}

int gpioClose(int fd)
{
	return close(fd);
}


// **********************************************************
// *************** Handlers *************************
// **********************************************************

static void sysfsSleep(uint32_t millis)
{
    usleep(millis * 1000);
}

static int sysfsGpioRead(Gpio_t *gpio)
{
    char ch;
    if(gpio == NULL) return -1;
    GpioSysfs_t *g = (GpioSysfs_t *)gpio;

    read(g->fd, &ch, 1);

	if (ch != '0') {
        g->pinValue = 1;
	} else {
		g->pinValue = 0;
	}

    return g->pinValue;
}


static GpioErrorStatus sysfsGpioWrite(Gpio_t *gpio, int value)
{
    if(gpio == NULL) return GpioStatus_ERROR;
    GpioSysfs_t *g = (GpioSysfs_t *)gpio;

    if (value)
		write(g->fd, "1", 2);
	else
		write(g->fd, "0", 2);
    
    return GpioStatus_OK;
}

static Gpio_t * sysfsGpioInit(int gpioNumber, int mode)
{
    if(gpioExport(gpioNumber) < 0) return NULL;
    if(gpioSetDir(gpioNumber, mode) < 0) return NULL;

    int fd = gpioOpen(gpioNumber);
    if(fd < 0)
    {
        gpioUnexport(gpioNumber);
        return NULL;
    }

    GpioSysfs_t *g = calloc(1, sizeof(GpioSysfs_t));
    g->fd = fd;
    g->base.gpio = gpioNumber;
    g->mode = mode;
    g->pinValue = 0;

    return &g->base;
}

static void sysfsGpioDestroy(Gpio_t *gpio)
{
    if(gpio == NULL) return;
    GpioSysfs_t *g = (GpioSysfs_t *)gpio;

    gpioUnexport(g->base.gpio);
    gpioClose(g->fd);
    free(gpio);
}

static int sysfsGpioGetFd(Gpio_t *gpio)
{
    if(gpio == NULL) return -1;
    GpioSysfs_t *g = (GpioSysfs_t *)gpio;
    return g->fd;
}

static void sysfsLog(int type, char *msg, ...)
{
    //TODO: manage log type

    va_list args;
    va_start (args, msg);
    vfprintf(stderr, msg, args);
    va_end (args);
}


// ******************************************************************
// ************************** Pulic Methods *************************
// ******************************************************************

void gpioBackendInitSysfs(Interface_t *i)
{
    if(i== NULL) return;

    i->sleep = sysfsSleep;
    
    i->gpioWrite = sysfsGpioWrite;
    i->gpioRead = sysfsGpioRead;
    i->gpioDestroy = sysfsGpioDestroy;
    i->gpioGetFd = sysfsGpioGetFd;
    i->gpioInit = sysfsGpioInit;
    
    i->log = sysfsLog;

}