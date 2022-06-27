#include "global.h"
#include "scheduler/scheduler.h"
#include "display/seg_led.h"
#include "conc/semaphore.h"
#include "bit_ops/bit_ops.h"
#include "events/events.h"
#include "rs485/rs485.h"
#include "error/error.h"
#include "random/random.h"
#include "mem/mem.h"

void startup()
{
    seg_led_init();
    rs485_init(115200);
    LED_SEG_SWITCH = 1;

    LEDs = 0xFF;
    delay_ms(500);
    LEDs = 0x00;
    delay_ms(100);
    led_display_content = 0;
    seg_set_number(0);
}

void proc1()
{
    XDATA u8 buf[4];
    XDATA u32 XDATA* ptr = (u32 XDATA*)buf;

    while(1)
    {
        wait_on_evts(EVT_BTN1_DN | EVT_UART2_RECV);
        {
            if(MY_EVENTS & EVT_BTN1_DN)
            {
                *ptr = rand32();
                seg_set_number(*ptr);
                rs485_write(buf, 4);
            }
            else if(MY_EVENTS & EVT_UART2_RECV)
            {
                my_memcpy(buf, rs485_buf, 4);
                seg_set_number(*ptr);
            }
        }
    }
}

XDATA u8 current = 0;
void main() //also proc0
{
    startup();

    start_scheduler(1);
    start_process(proc1);

    //DISPLAY DRIVER
    while(1)
    {   
        ATOMIC(
            seg_led_scan_next();
            process_events();
        )
        yield(); 
    }
}