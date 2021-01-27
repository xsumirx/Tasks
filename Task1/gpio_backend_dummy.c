#include "gpio_backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_PATH   "/tmp/gpio/"

typedef struct
{
    Gpio_t base;
    int fd;
    int mode;

    int pinValue;
}GpioDummy_t;

static void dummySleep(uint32_t millis)
{
    usleep(millis * 1000);
}

static int dummyGpioRead(Gpio_t *gpio)
{
    if(gpio == NULL) return -1;
    char ch;

    GpioDummy_t *g = (GpioDummy_t *)gpio;

    int ret = read(g->fd, &ch, 1);

    //Return pin value
    if(ret > 0)
    {
        if (ch == '1') {
            g->pinValue = 1;
        } else if(ch == '0'){
            g->pinValue = 0;
        }
    }

    return g->pinValue;
}


static GpioErrorStatus dummyGpioWrite(Gpio_t *gpio, int value)
{
    if(gpio == NULL) return GpioStatus_ERROR;
    GpioDummy_t *g = (GpioDummy_t *)gpio;
    g->pinValue = value;
    write(g->fd, value?"1":"0", 2);
    return GpioStatus_OK;
}

static Gpio_t * dummyGpioInit(int gpioNumber, int mode)
{
    GpioDummy_t *g = calloc(1, sizeof(GpioDummy_t));
    if(mode == GpioMode_OUTPUT)
    {
        g->fd = 1;
    }else
    {
        g->fd = 0;
        fcntl(g->fd, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    }

    g->base.gpio = gpioNumber;
    g->mode = mode;
    g->pinValue = 0;

    return &g->base;
}

static void dummyGpioDestroy(Gpio_t *gpio)
{
    if(gpio == NULL) return;
    GpioDummy_t *g = (GpioDummy_t *)gpio;
    free(gpio);
}

static int dummyGpioGetFd(Gpio_t *gpio)
{
    if(gpio == NULL) return -1;
    GpioDummy_t *g = (GpioDummy_t *)gpio;
    return g->fd;
}

static void dummyLog(int type, char *msg, ...)
{
    //TODO: manage log type

    va_list args;
    va_start (args, msg);
    vprintf(msg, args);
    va_end (args);
}


// ******************************************************************
// ************************** Pulic Methods *************************
// ******************************************************************

void gpioBackendInitDummy(Interface_t *i)
{
    if(i== NULL) return;

    i->sleep = dummySleep;
    
    i->gpioWrite = dummyGpioWrite;
    i->gpioRead = dummyGpioRead;
    i->gpioDestroy = dummyGpioDestroy;
    i->gpioGetFd = dummyGpioGetFd;
    i->gpioInit = dummyGpioInit;
    
    i->log = dummyLog;

}