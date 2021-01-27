#ifndef __GPIO_BACKEND_H__
#define _GPIO_BACKEND_H__

#include <stdint.h>

#define GPIO_LOG_DEBUG 1
#define GPIO_LOG_WARN  2

enum GpioMode
{
    GpioMode_OUTPUT,
    GpioMode_INPUT
};

typedef enum 
{
    GpioStatus_OK = 0,
    GpioStatus_ERROR = -1,
}GpioErrorStatus;

typedef struct
{
    int gpio;           //Gpio Number

}Gpio_t;

/**
 * Interface to glue application to backend.
 */
typedef struct
{
    void (*sleep)(uint32_t millis);

    Gpio_t * (*gpioInit)(int gpioNumber, int mode);
    void (*gpioDestroy)(Gpio_t *);
    int (*gpioGetFd)(Gpio_t *);

    int (*gpioRead)(Gpio_t *gpio);
    GpioErrorStatus (*gpioWrite)(Gpio_t *gpio, int gpioState);
    
    void (*log)(int type, char *msg, ...);
}Interface_t;

/**
 * Initialize application with Dummy backend
 * @param i interface pointer.
 * @return
 */
void gpioBackendInitDummy(Interface_t *i);

/**
 * Initialize application with sysfs backend
 * @param i interface pointer.
 * @return
 */
void gpioBackendInitSysfs(Interface_t *i);

#endif