/*get_exitstatus.c*/
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>

#include <unistd.h>
#include <sys/reboot.h>

volatile sig_atomic_t flag = 1;

void sig_handler(int signum)
{
    if (signum == SIGINT) {
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT, sig_handler);

    if(argc == 1)
    {
        std::cout << "no program specified" << std::endl;
        exit(0);
    }

    pid_t c_pid, pid;
    int status;

    bool flag_wait;

    for(int i = 0; i < 5; i++)
    {
        flag_wait = true;
        c_pid = fork(); //duplicate

        if( c_pid == 0 ){
            //child
            pid = getpid();

            printf("Child: %s (%d) start\n", argv[1], pid, c_pid);

            execl(argv[1],"",NULL);

        }else if (c_pid > 0){
            //parent

            //waiting for child to terminate
            pid = wait(&status);

            if ( WIFEXITED(status) ){
                printf("Parent: Child exited with status: %d\n", WEXITSTATUS(status));
            }
            flag_wait = false;
        }else{
            //error: The return of fork() is negative
            perror("fork failed");
            // _exit(2); //exit failure, hard
            flag_wait = false;
        }

        while(flag_wait) {}
        sleep(1);
        std::cout << "----------\n\nrestart\n\n----------" << std::endl;
    }

    std::cout << "----------\n\ntoo main ERROR, reboot\n\n----------" << std::endl;

    sync();
    reboot(RB_AUTOBOOT);

    return 0; //success                                                                                                                                                        
}