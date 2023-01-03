#include <linux/gpio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <iostream>
#include <fmt/format.h>

#include "gpio_lib.hpp"


struct FD {
private:
    int fd{-1};

public:
    //default constructor
    FD() = delete;

    //one of many constructors
    FD(int filedescriptor) : fd{filedescriptor} {
        fmt::print("{} {}\n", __PRETTY_FUNCTION__, fmt::ptr(this));
    }

    //Copy constructor
    FD(FD const& other) = delete; //delete or default

    //Copy assignment operator
    FD& operator=(FD const& other) = delete; //delete or default

    //move constructor
    FD(FD&& other) noexcept :fd{other.fd} {
        other.fd = -1;
        //fmt::print("{} self:{} other:{}\n", __PRETTY_FUNCTION__, fmt::ptr(this), fmt::ptr(&other));
    }
    //move assignment operator
    FD& operator=(FD&& other) noexcept {
        //fmt::print("{} self:{} other:{}\n", __PRETTY_FUNCTION__, fmt::ptr(this), fmt::ptr(&other));
        //needed for move assignment operator && copy assignment operator if implmented
        if(&other != this){
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }
    //Destructor
    ~FD() noexcept {
        fmt::print("{} {}\n", __PRETTY_FUNCTION__, fmt::ptr(this));
        if(fd != -1) {
            fmt::print("File closed \n");
            close(fd);
        }
    }

    int get_fd() const {
        return fd;
    };
};



GPIO::Pin::Pin(std::string const& gpio_chip_, int offset_)
:offset{offset_},gpio_chip{gpio_chip_}
{}

GPIO::Pin::~Pin(){

}

void GPIO::list(std::string const& gpio_chip)
{
    struct gpiochip_info info;
    struct gpio_v2_line_info line_info;

    int ret;
    FD fd{open(gpio_chip.c_str(), O_RDONLY)};
    if (fd.get_fd() < 0)
    {
        fmt::print("Unabled to open {}: {} \n", gpio_chip, strerror(errno));
        return;
    }
    ret = ioctl(fd.get_fd(), GPIO_GET_CHIPINFO_IOCTL, &info);
    if (ret == -1)
    {
        fmt::print("Unable to get chip info from ioctl: {} \n", strerror(errno));
        return;
    }
    fmt::print("Chip name: {}\n", info.name);
    fmt::print("Chip label: {} \n", info.label);
    fmt::print("Number of lines: {}\n",info.lines);

    for (int i = 0; i < info.lines; i++)
    {
        memset(&line_info, 0, sizeof(line_info));
        line_info.offset = i;

        ret = ioctl(fd.get_fd(), GPIO_V2_GET_LINEINFO_IOCTL, &line_info);
        if (ret == -1)
        {
            fmt::print("Unable to get line info from offset {} : {} \n", i, strerror(errno));
        }
        else
        {
            fmt::print("offset: {}, name: {}, consumer: {}. Flags:\t[{}]\t[{}]\t[{}]\t[{}]\t[{}]\t[{}]\t[{}]\t[{}]\t[{}]\t[{}]\n",
                   i,
                   line_info.name,
                   line_info.consumer,
                   (line_info.flags & GPIO_V2_LINE_FLAG_USED) ? "USED" : "",
                   (line_info.flags & GPIO_V2_LINE_FLAG_INPUT) ? "INPUT" : "",
                   (line_info.flags & GPIO_V2_LINE_FLAG_OUTPUT) ? "OUTPUT" : "",
                   (line_info.flags & GPIO_V2_LINE_FLAG_BIAS_PULL_UP) ? "PULL_UP" : "",
                   (line_info.flags & GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN) ? "PULL_DOWN" : "",
                   (line_info.flags & GPIO_V2_LINE_FLAG_BIAS_DISABLED) ? "BIAS_DISABLED" : "",
                   (line_info.flags & GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME) ? "EVENT_CLOCK" : "",
                   (line_info.flags & GPIO_V2_LINE_FLAG_ACTIVE_LOW) ? "ACTIVE_LOW" : "ACTIVE_HIGH",
                   (line_info.flags & GPIO_V2_LINE_FLAG_OPEN_DRAIN) ? "OPEN_DRAIN" : "...",
                   (line_info.flags & GPIO_V2_LINE_FLAG_OPEN_SOURCE) ? "OPENSOURCE" : "...");
        }
    }
}

//TODO return value should indicate sucessfull writing
void GPIO::Pin::write(bool newValue)
{
    struct gpio_v2_line_request linereq;
    struct gpio_v2_line_config lineconf;
    struct gpio_v2_line_values value;
    int retval;

    FD fd{open(gpio_chip.c_str(), O_RDONLY)};
    if (fd.get_fd() < 0)
    {
        fmt::print("Unabled to open {}: {} \n", gpio_chip, strerror(errno));
        return;
    }

    memset(&linereq, 0, sizeof(linereq));
    memset(&lineconf, 0, sizeof(lineconf));

    lineconf.flags = GPIO_V2_LINE_FLAG_OUTPUT;

    linereq.offsets[0] = offset;
    linereq.config = lineconf;
    linereq.num_lines = 1;


    retval = ioctl(fd.get_fd(), GPIO_V2_GET_LINE_IOCTL, &linereq);
    if (retval == -1)
    {
        fmt::print("Unable to get line handle from ioctl : {} \n", strerror(errno));
        return;
    }

    memset(&value, 0, sizeof(value));
    value.mask = linereq.offsets[0];
    value.bits = newValue;

    retval = ioctl(linereq.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &value);
    if (retval == -1)
    {
        fmt::print("Unable to get line value using ioctl : {} \n", strerror(errno));
        close(linereq.fd);
        return;
    }
    else
    {
        fmt::print("Value of GPIO at offset {} (OUTPUT mode) on chip {} set to: {}\n", offset, gpio_chip, value.bits);
        close(linereq.fd);
        return;
    }
}

//TODO Better Error Handling
int GPIO::Pin::read()
{
    struct gpio_v2_line_request linereq;
    struct gpio_v2_line_config lineconf;
    struct gpio_v2_line_values value;
    int retval;

    FD fd{open(gpio_chip.c_str(), O_RDONLY)};
    if (fd.get_fd() < 0)
    {
        fmt::print("Unabled to open {}: {} \n", gpio_chip, strerror(errno));
        return -1;
    }

    memset(&linereq, 0, sizeof(linereq));
    memset(&lineconf, 0, sizeof(lineconf));

    lineconf.flags = GPIO_V2_LINE_FLAG_INPUT;

    linereq.offsets[0] = offset;
    linereq.config = lineconf;
    linereq.num_lines = 1;


    retval = ioctl(fd.get_fd(), GPIO_V2_GET_LINE_IOCTL, &linereq);
    if (retval == -1)
    {
        fmt::print("Unable to get line handle from ioctl : {} \n", strerror(errno));
        return -2;
    }

    memset(&value, 0, sizeof(value));
    value.mask = linereq.offsets[0];

    retval = ioctl(linereq.fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &value);
    if (retval == -1)
    {
        fmt::print("Unable to get line value using ioctl : {} \n", strerror(errno));
        close(linereq.fd);
        return -3;
    }
    else
    {
        fmt::print("Value of GPIO at offset {} (INPUT mode) on chip {}: {}\n", offset, gpio_chip, value.bits);
        close(linereq.fd);
        return value.bits;
    }
}

void GPIO::Pin::poll_falling()
{
    struct gpio_v2_line_request linereq;
    struct gpio_v2_line_config lineconf;
    //struct gpio_v2_line_config_attribute lineconfattr;
    //struct gpio_v2_line_attribute lineattr;
    struct pollfd pfd;

    int retval;

    FD fd{open(gpio_chip.c_str(), O_RDONLY)};
    if (fd.get_fd() < 0)
    {
        fmt::print("Unabled to open {}: {} \n", gpio_chip, strerror(errno));
        return;
    }

    memset(&linereq, 0, sizeof(linereq));
    memset(&lineconf, 0, sizeof(lineconf));

    lineconf.num_attrs = 0;
    lineconf.flags = lineconf.flags | (GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_EDGE_FALLING);

    linereq.offsets[0] = offset;
    linereq.config = lineconf;
    linereq.num_lines = 1;

    retval = ioctl(fd.get_fd(), GPIO_V2_GET_LINE_IOCTL, &linereq);

    if (retval == -1)
    {
        fmt::print("Unable to get line event from ioctl : {} \n", strerror(errno));
        return;
    }
    pfd.fd = linereq.fd;
    close(linereq.fd);
    pfd.events = POLLIN;
    retval = poll(&pfd, 1, -1);
    if (retval == -1)
    {
        fmt::print("Error while polling event from GPIO: {}\n", strerror(errno));
    }
    else if (pfd.revents & POLLIN)
    {
        fmt::print("Falling edge event on GPIO offset: {}, of {}\n", offset, gpio_chip);
    }
    close(pfd.fd);
}

void GPIO::Pin::poll_rising()
{
    struct gpio_v2_line_request linereq;
    struct gpio_v2_line_config lineconf;
    //struct gpio_v2_line_config_attribute lineconfattr;
    //struct gpio_v2_line_attribute lineattr;
    struct pollfd pfd;

    int retval;

    FD fd{open(gpio_chip.c_str(), O_RDONLY)};
    if (fd.get_fd() < 0)
    {
        fmt::print("Unabled to open {}: {} \n", gpio_chip, strerror(errno));
        return;
    }

    memset(&linereq, 0, sizeof(linereq));
    memset(&lineconf, 0, sizeof(lineconf));

    lineconf.num_attrs = 0;
    lineconf.flags = lineconf.flags | (GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_EDGE_RISING);

    linereq.offsets[0] = offset;
    linereq.config = lineconf;
    linereq.num_lines = 1;

    retval = ioctl(fd.get_fd(), GPIO_V2_GET_LINE_IOCTL, &linereq);

    if (retval == -1)
    {
        fmt::print("Unable to get line event from ioctl : {} \n", strerror(errno));
        return;
    }
    pfd.fd = linereq.fd;
    close(linereq.fd);
    pfd.events = POLLIN;
    retval = poll(&pfd, 1, -1);
    if (retval == -1)
    {
        fmt::print("Error while polling event from GPIO: {}\n", strerror(errno));
    }
    else if (pfd.revents & POLLIN)
    {
        fmt::print("Falling edge event on GPIO offset: {}, of {}\n", offset, gpio_chip);
    }
    close(pfd.fd);
}
