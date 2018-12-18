#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "shell.h"
#include "pcd8544.h"

#include "receive_ir.h"
#include "send_ir.h"

#define BUFFERSIZE 32


#define MSG_BUFFER_SIZE 255


gpio_t receive_pin = GPIO_PIN(PA, 13);
gpio_t send_pin = GPIO_PIN(PA, 6);


int send_msg_handler(int argc, char **argv) {
    if (argc != 2) {
        puts("Illegal argument count");
        return 1;
    }

    if (messageSend(0, 0, (uint8_t *) argv[1], strlen(argv[1])) != EXIT_SUCCESS) {
        puts("message to long");
        return -1;
    }

    return 0;
}


static pcd8544_t dev;
static spi_t mySpi = SPI_DEV(1);
static gpio_t myCs = GPIO_PIN(PA, 16);
static gpio_t myReset = GPIO_PIN(PA, 14);
static gpio_t myMode = GPIO_PIN(PA, 17);

static int _contrast(int argc, char **argv)
{
    uint8_t val;

    if (argc < 2) {
        printf("usage: %s VAL [0-127]\n", argv[0]);
        return 1;
    }
    val = atoi(argv[1]);
    pcd8544_set_contrast(&dev, val);
    return 0;
}

static int _temp(int argc, char **argv)
{
    uint8_t val;

    if (argc < 2) {
        printf("usage: %s VAL [0-3]\n", argv[0]);
        return 1;
    }
    val = atoi(argv[1]);
    pcd8544_set_tempcoef(&dev, val);
    return 0;
}

static int _bias(int argc, char **argv)
{
    uint8_t val;

    if (argc < 2) {
        printf("usage: %s VAL [0-7]\n", argv[0]);
        return 1;
    }
    val = atoi(argv[1]);
    pcd8544_set_bias(&dev, val);
    return 0;
}

static int _on(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    pcd8544_poweron(&dev);
    return 0;
}

static int _off(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    pcd8544_poweroff(&dev);
    return 0;
}

static int _clear(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    pcd8544_clear(&dev);
    return 0;
}

static int _invert(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    pcd8544_invert(&dev);
    return 0;
}

static int _riot(int argc, char **argv) {
    (void)argc;
    (void)argv;

    pcd8544_riot(&dev);
    return 0;
}

static int _write(int argc, char **argv)
{
    uint8_t x, y;

    if (argc < 4) {
        printf("usage: %s X Y STRING\n", argv[0]);
        return -1;
    }

    x = atoi(argv[1]);
    y = atoi(argv[2]);

    pcd8544_write_s(&dev, x, y, argv[3]);
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "contrast", "set contrast", _contrast },
    { "temp", "set temperature coefficient", _temp },
    { "bias", "set BIAS value", _bias },
    { "on", "turn display on", _on },
    { "off", "turn display off", _off },
    { "clear", "clear memory", _clear },
    { "invert", "invert display", _invert },
    { "riot", "display RIOT logo", _riot },
    { "write", "write string to display", _write},
    {"sendmsg", "sends message via IR", send_msg_handler},
    { NULL, NULL, NULL }
};

int main(void){
    puts("This is the RIOTOIR project\n");
    printf("This application runs on %s\n", RIOT_BOARD);
    
    
    if( pcd8544_init(&dev, mySpi, myCs, myReset, myMode) != 0){
        puts("Failed to initialize PCD8544 display\n");
        return 1;
    }

    setup_ir_recv(receive_pin);
    setup_ir_send(send_pin);
    
    puts("All OK, running shell now");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
