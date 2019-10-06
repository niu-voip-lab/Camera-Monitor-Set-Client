#include "connection.h"

/* standard headers */
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

/* mraa headers */
#include "mraa/common.hpp"
#include "mraa/gpio.hpp"

/* gpio declaration */
#define GPIO_PIN_1 13
#define GPIO_PIN_2 45
#define GPIO_PIN_3 46

volatile sig_atomic_t flag = 1;

void sig_handler(int signum)
{
    if (signum == SIGINT) {
        std::cout << "Exiting..." << std::endl;
        flag = 0;
    }
}

int connectToServer()
{
    mraa::Result status;

    /* install signal handler */
    signal(SIGINT, sig_handler);

    mraa::Gpio gpio_1(GPIO_PIN_1);
    mraa::Gpio gpio_2(GPIO_PIN_2);
    mraa::Gpio gpio_3(GPIO_PIN_3);

    status = gpio_1.dir(mraa::DIR_OUT);
    if (status != mraa::SUCCESS) {
        printError(status);
        return EXIT_FAILURE;
    }

    bool ff = 0;
    int fff = (int) ff;

    while (flag) {
        status = gpio_1.write(fff);
        if (status != mraa::SUCCESS) {
            printError(status);
            return EXIT_FAILURE;
        }
        sleep(1);
    }
}