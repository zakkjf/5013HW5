#include <stdint.h>
#include <stdbool.h>
#include "drivers/pinout.h"
#include "utils/uartstdio.h"


// TivaWare includes
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#define SYSTEM_CLOCK    120000000U

// Demo Task declarations
void task1(void *pvParameters);
void task2(void *pvParameters);

void vTimerCallback1(TimerHandle_t xTimer);
void vTimerCallback2(TimerHandle_t xTimer);
TaskHandle_t th1;
TaskHandle_t th2;
/* An array to hold handles to the created timers. */
TimerHandle_t xTimers[2];

// Main function
int main(void)
{
    // Initialize system clock to 120 MHz
    uint32_t output_clock_rate_hz;
    output_clock_rate_hz = ROM_SysCtlClockFreqSet(
                               (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                               SYSTEM_CLOCK);
    ASSERT(output_clock_rate_hz == SYSTEM_CLOCK);

    //2 hz timer
    xTimers[1] = xTimerCreate("Timer1", 500, pdTRUE, ( void * ) 0, vTimerCallback2);
    //4 hz timer
    xTimers[0] = xTimerCreate("Timer0", 250, pdTRUE, ( void * ) 0, vTimerCallback1);

    if( xTimerStart( xTimers[0], 0 ) != pdPASS )
    {
        //The timer could not be set into the Active state.
    }
    if( xTimerStart( xTimers[1], 0 ) != pdPASS )
    {
        //The timer could not be set into the Active state.
    }
    // Initialize the GPIO pins for the Launchpad
    PinoutSet(false, false);

    // Create tasks
    xTaskCreate(task1, (const portCHAR *)"LED1",
                configMINIMAL_STACK_SIZE, NULL, 1, &th1);
    xTaskCreate(task2, (const portCHAR *)"LED2",
                configMINIMAL_STACK_SIZE, NULL, 1, &th2);
   // xTaskCreate(demoSerialTask, (const portCHAR *)"Serial",
    //            configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();
    return 0;
}


// Flash the LEDs on the launchpad
void task1(void *pvParameters)
{

    for (;;)
    {
        LEDWrite(0x01, 0x01);
        vTaskSuspend(th1);
        LEDWrite(0x01, 0x00);
        vTaskSuspend(th1);
    }
}

// Flash the LEDs on the launchpad
void task2(void *pvParameters)
{
    for (;;)
    {
        LEDWrite(0x02, 0x02);
        vTaskSuspend(th2);
        LEDWrite(0x02, 0x00);
        vTaskSuspend(th2);
    }
}

/* Define a callback function that will be used by multiple timer
instances.*/
void vTimerCallback1( TimerHandle_t xTimer )
{
    vTaskResume(th1);
}

void vTimerCallback2( TimerHandle_t xTimer )
{
    vTaskResume(th2);
}


/*  ASSERT() Error function
 *
 *  failed ASSERTS() from driverlib/debug.h are executed in this function
 */
void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}
