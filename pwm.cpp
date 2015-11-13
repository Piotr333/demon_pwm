#define _GNU_SOURCE 1
#include "stdio.h"
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include "pruio.h"
#include "pruio_pins.h"
 
#define P_OUT P9_21
#define P_IN P9_42
 
int main(int argc, char **argv)
{
    pruIo *Io = pruio_new(PRUIO_DEF_ACTIVE, 0x98, 0, 1);
    do {
        if (Io->Errr)
        {
            printf("initialisation failed (%s)\n", Io->Errr); 
            break;
        }
 
        if (pruio_cap_config(Io, P_IN, 2.))
        { // configure input pin
            printf("failed setting input @P_IN (%s)\n", Io->Errr); 
            break;
        }
 
        float_t
            , f1 // Variable for calculated frequency.
            , d1; // Variable for calculated duty cycle.
 
        // pin config OK, transfer local settings to PRU and start
        if (pruio_config(Io, 1, 0x1FE, 0, 4))
        {
            printf("config failed (%s)\n", Io->Errr);
            break;
        }
 
        float counter = 0.5;
        while(1) { // run endless loop
            counter+=0.0001;
            if(counter > 0.95)
                counter = 0.5;
            if (pruio_cap_Value(Io, P_IN, &f1, &d1))
            { // get current input
                printf("failed reading input @P_IN (%s)\n", Io->Errr); 
                break;
            }
 
            //printf("counter:%f ,freq:%.0f, duty:%f\n",counter, f1, d1);

            if (pruio_pwm_setValue(Io, P_OUT, 50, d1))
            { // set new output
                printf("failed setting PWM output (%s)\n", Io->Errr); 
                break;
            }

           //if (pruio_config(Io, 1, 0x1FE, 0, 4)) {
           //    printf("config failed (%s)\n", Io->Errr); break;}
        }
    } while (0);

    pruio_destroy(Io); /* destroy driver structure */
    return 0;
}
