
#pragma once
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>


typedef struct {
    int       fd;
    uint8_t   mode;
    uint8_t   bits_per_word;
    uint32_t  speed_hz;
} spi_device_t;

int spi_open(spi_device_t *spi, const char *device,
             uint8_t mode, uint8_t bits, uint32_t speed);
void spi_close(spi_device_t *spi);
static int spi_transfer(spi_device_t *spi,
                        const uint8_t *tx, uint8_t *rx, size_t len);