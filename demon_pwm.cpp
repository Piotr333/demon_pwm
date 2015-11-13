#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
/*#include <iostream>
#include <fstream>
#include <cstdio>*/

#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include "pruio.h"
#include "pruio_pins.h"

//zestaw 1 - działający 
#define P_OUT1 P9_21 
#define P_IN1 P9_42

//zestaw 2 - ?? 
#define P_OUT2 P9_22
#define P_IN2 P9_43

//zestaw 3 - ?? 
#define P_OUT3 P9_23
#define P_IN3 P9_44

//zestaw 4 - ?? 
#define P_OUT4 P9_25 
#define P_IN4 P9_45

// wajcha
#define P_OUT P_OUT1
#define P_IN P_IN1

int main(int argc, char* argv[])
{   // const std::string logFile = "Log.txt";
    FILE *log= NULL;
    pid_t process_id = 0;
    pid_t sid = 0;
    // Create child process
    process_id = fork();
    // Indication of fork() failure
    if (process_id < 0)
    {
        printf("fork failed!\n");
        // Return failure in exit status
        exit(1);
    }
    
    // PARENT PROCESS. Need to kill it.
    if (process_id > 0)
    {
        printf("process_id of child process %d \n", process_id);
        // return success in exit status
        exit(0);
    }

    //unmask the file mode
    umask(0);
    //set new session
    sid = setsid();
    if(sid < 0)
    {   // Return failure
        exit(1);
    }

    // Change the current working directory to root.
    chdir("/");
    // Close stdin. stdout and stderr
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // DEAMON PART
    pruIo *Io = pruio_new(PRUIO_DEF_ACTIVE, 0x98, 0, 1);
    //std::remove(logFile.c_str()); // delete file
    // Open a log file in write mode.
    log = fopen ("Log.txt", "w+");
    while (1)
    {
        sleep(1);   // TO ERASE
        fprintf(log, "keep alive...\n");
        fflush(log);

        if (Io->Errr)
        {
            fprintf(log, "initialisation failed (%s)\n", Io->Errr);
            fflush(log);
            break;
        }

        if (pruio_cap_config(Io, P_IN, 2.))
        {   // configure input pin
            fprintf(log, "failed setting input @P_IN (%s)\n", Io->Errr); 
            fflush(log);
            break;
        }
 
        float_t
            f1 // Variable for calculated frequency.
            , d1; // Variable for calculated duty cycle.
 
        // pin config OK, transfer local settings to PRU and start
        if (pruio_config(Io, 1, 0x1FE, 0, 4))
        {
            fprintf(log, "config failed (%s)\n", Io->Errr);
            fflush(log);
            break;
        }

        float counter = 0.5;
        while(1) 
        {   // run endless loop
            counter+=0.0001;
            if(counter > 0.95)
                counter = 0.5;
            if (pruio_cap_Value(Io, P_IN, &f1, &d1))
            {   // get current input
                fprintf(log, "failed reading input @P_IN (%s)\n", Io->Errr);
                fflush(log);
                break;
            }
 
            fprintf(log, "counter:%.5f, freq:%.0f, duty:%.4f\n",counter, f1, d1);
            fflush(log);

            if (pruio_pwm_setValue(Io, P_OUT, 50, d1))
            {   // set new output
                fprintf(log, "failed setting PWM output (%s)\n", Io->Errr);
                fflush(log);
                break;
            }

           //if (pruio_config(Io, 1, 0x1FE, 0, 4)) 
           //{
           //    fprintf(log, "config failed (%s)\n", Io->Errr);
           //    fflush(log);
           //    break;
           //}
        }

    }
    fclose(log);
    pruio_destroy(Io); /* destroy driver structure */
    return (0);
}