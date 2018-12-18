/**
 * Based on RIOT test for display driver pcd8544.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Riotoir_display.h"
#include "pcd8544.h"

static pcd8544_t dev;
static spi_t mySpi = SPI_DEV(1);
static gpio_t myCs = GPIO_PIN(PA, 16);
static gpio_t myReset = GPIO_PIN(PA, 14);
static gpio_t myMode = GPIO_PIN(PA, 17);

int contrast( int argc, char** argv ) {
    uint8_t val;

    if (argc < 2) {
        printf("usage: %s VAL [0-127]\n", argv[0]);
        return 1;
    }
    val = atoi(argv[1]);
    pcd8544_set_contrast(&dev, val);
    return 0;
}

int temp( int argc, char** argv ) {
    uint8_t val;

    if (argc < 2) {
        printf("usage: %s VAL [0-3]\n", argv[0]);
        return 1;
    }
    val = atoi(argv[1]);
    pcd8544_set_tempcoef(&dev, val);
    return 0;
}

int bias( int argc, char** argv ) {
    uint8_t val;

    if (argc < 2) {
        printf("usage: %s VAL [0-7]\n", argv[0]);
        return 1;
    }
    val = atoi(argv[1]);
    pcd8544_set_bias(&dev, val);
    return 0;
}

int on( int argc, char** argv ) {
    (void) argc;
    (void) argv;

    pcd8544_poweron(&dev);
    return 0;
}

int off( int argc, char** argv ) {
    (void) argc;
    (void) argv;

    pcd8544_poweroff(&dev);
    return 0;
}

int clear( int argc, char** argv ) {
    (void) argc;
    (void) argv;

    pcd8544_clear(&dev);
    return 0;
}

int invert( int argc, char** argv ) {
    (void) argc;
    (void) argv;

    pcd8544_invert(&dev);
    return 0;
}

int riot( int argc, char** argv ) {
    (void) argc;
    (void) argv;

    pcd8544_riot(&dev);
    return 0;
}

int write( int argc, char** argv ) {
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


int display_init(void) {
    if (pcd8544_init(&dev, mySpi, myCs, myReset, myMode) != 0) {
        puts("Failed to initialize PCD8544 display\n");
        return 1;
    }
    return 0;
}
