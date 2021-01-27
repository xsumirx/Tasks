#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>

#include <pthread.h>

#include "gpio_backend.h"

#define STATE_IDLE      0
#define STATE_TOOGLE    1
#define STATE_ERROR     2

typedef struct
{
    Gpio_t *input;      
    Gpio_t *output;

    //Current State
    struct 
    {
        pthread_t id;
        pthread_mutex_t lock;

        int pinValue;
        int appState;
        
    }state;

    Interface_t i;
    struct
    {
        int gpioY;
        int gpioX;
        int logEnable;
        int backend;
    }settings;
}Application_t;



void print_usage() {
    printf("Usage: gpio [l,b] -i <input pin Y> -o <output pin X>\n");
    printf("-l | --log      : Enable logging\n");
    printf("-i | --input    : Gpio number eg. 1, 2 etc\n");
    printf("-o | --output   : Gpio number eg. 1, 2 etc\n");
    printf("-b | --backend  : Backend <default is sysfs>\n");
    printf("                  0 - sysfs, 1 - console\n");
}

void LOGD(Application_t *app, char *msg, ...)
{
    if(app->settings.logEnable)
    {
        va_list args;
        va_start (args, msg);
        app->i.log(GPIO_LOG_DEBUG,msg, args);
        va_end (args);
    }
}

void gpioLogPin(Application_t *app, int gpio, int state)
{
    time_t rawtime = time(NULL);
    struct tm *l = localtime(&rawtime);
    // LOGD(app, "%d-%02d-%02dT%02d:%02d:%02d gpio %d %s\n", l->tm_year,
    //     l->tm_mon, l->tm_mday, l->tm_hour, l->tm_min, l->tm_sec, gpio, state?"HIGH":"LOW");
    
    LOGD(app, "gpio %d %s\n",gpio, state?"HIGH\0":"LOW\0");
}

int gpioProcessCommandline(Application_t *app,int argc, char **argv)
{
    app->settings.gpioX = -1;
    app->settings.gpioY = -1;
    app->settings.logEnable = 0;
    app->settings.backend = 0;

    //Prepare and Read Args
    struct option options[] =
    {
        {"log",     no_argument,            0,  'l'},
        {"input",   required_argument,      0,  'i'},
        {"output",  required_argument,      0,  'o'},
        {"backend",  required_argument,     0,  'b'},
        {0,         0,                      0,  0}
    };

    int long_index =0;
    int opt= 0;
    while ((opt = getopt_long(argc, argv,"li:o:b:", 
                   options, &long_index )) != -1) {
        switch (opt) {
             case 'l' : app->settings.logEnable = 1;
                 break;
             case 'i' : app->settings.gpioY = atoi(optarg);
                 break;
             case 'o' : app->settings.gpioX = atoi(optarg); 
                 break;
            
             case 'b' : app->settings.backend = atoi(optarg);
                 break;

             default: print_usage(); 
                return -1;
        }
    }

    if( app->settings.gpioX == -1 || app->settings.gpioY == -1)
    {
        print_usage();
        return -1;
    }

    return 0;
}

static void * Looper(void *args)
{
    Application_t *app = (Application_t *)args;
    for(;;)
    {
        app->i.sleep(1000); //TODO: Make toggle timmings configurable
        pthread_mutex_lock(&app->state.lock);
        if(app->state.appState == STATE_TOOGLE){
            app->state.pinValue = app->state.pinValue == 0?1:0;
            app->i.gpioWrite(app->output, app->state.pinValue);
        }
        pthread_mutex_unlock(&app->state.lock);
    }
}

int main (int argc, char **argv)
{
    Application_t app;

    if(gpioProcessCommandline(&app, argc, argv) < 0)
        return -1;

    switch(app.settings.backend){
        case 0:
            gpioBackendInitSysfs(&app.i);
            break;
        case 1:
            gpioBackendInitDummy(&app.i);
            break;
        default:break;
    }

    //TODO: Register signal handlers

    //Initialize GPIO
    if((app.output =  app.i.gpioInit(app.settings.gpioX, GpioMode_OUTPUT)) == NULL)
    {
        LOGD(&app, "Failed to set mode output for gpio %d\n", app.settings.gpioX);
        return -1;
    }
    
    if((app.input = app.i.gpioInit(app.settings.gpioY, GpioMode_INPUT)) == NULL)
    {
        LOGD(&app, "Failed to set mode input for gpio %d\n", app.settings.gpioY);
        return -1;
    }


    //Configure state and start looper thread
    app.state.pinValue = app.i.gpioRead(app.input);
    app.state.appState = app.state.pinValue?STATE_TOOGLE:STATE_IDLE;
    pthread_mutex_init(&app.state.lock, NULL);
    pthread_create(&app.state.id, NULL, Looper, &app);

    // Select
    fd_set inputFdSet;
    int fdInput = app.i.gpioGetFd(app.input);

    while (app.state.appState != STATE_ERROR)
    {
        //Call only returns when value changes on input pin
        FD_ZERO(&inputFdSet);
        FD_SET(fdInput, &inputFdSet);
        if (select(fdInput+1, &inputFdSet, NULL,NULL,NULL) < 0)
        {
            LOGD(&app, "Problem occured with event loop\n");
            app.state.appState = STATE_ERROR;
            break;
        }

        int value = app.i.gpioRead(app.input);
        if(value < 0) 
        {
            LOGD(&app, "Problem occured with event loop\n");
            app.state.appState = STATE_ERROR;
            break;
        }

        gpioLogPin(&app, app.settings.gpioY, value);
        pthread_mutex_lock(&app.state.lock);

        if(value == 0)
        {
            app.i.gpioWrite(app.output, 0);
            app.state.pinValue = 0;
            app.state.appState = STATE_IDLE;
        }
        else
        {
            app.state.appState = STATE_TOOGLE;
        }

        pthread_mutex_unlock(&app.state.lock);
    }

    pthread_join(app.state.id, NULL);
    app.i.gpioDestroy(app.input);
    app.i.gpioDestroy(app.output);
    exit(0);
}