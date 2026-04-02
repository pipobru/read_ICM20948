#include "spi_custom.h"

int spi_open(spi_device_t *spi, const char *device,
             uint8_t mode, uint8_t bits, uint32_t speed)
{
    spi->fd = open(device, O_RDWR);
    if (spi->fd < 0) {
        perror("spi_open: open");
        return -1;
    }

    spi->mode         = mode;
    spi->bits_per_word = bits;
    spi->speed_hz     = speed;

    /* Mode SPI (CPOL / CPHA) */
    if (ioctl(spi->fd, SPI_IOC_WR_MODE, &spi->mode) < 0) {
        perror("spi_open: SPI_IOC_WR_MODE");
        goto err;
    }

    /* Nombre de bits par mot */
    if (ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &spi->bits_per_word) < 0) {
        perror("spi_open: SPI_IOC_WR_BITS_PER_WORD");
        goto err;
    }

    /* Vitesse max en Hz */
    if (ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi->speed_hz) < 0) {
        perror("spi_open: SPI_IOC_WR_MAX_SPEED_HZ");
        goto err;
    }

    return 0;

err:
    close(spi->fd);
    spi->fd = -1;
    return -1;
}

void spi_close(spi_device_t *spi)
{
    if (spi->fd >= 0) {
        close(spi->fd);
        spi->fd = -1;
    }
}

/* ------------------------------------------------------------------ */
/* Transfert SPI full-duplex                                           */
/* ------------------------------------------------------------------ */
int spi_transfer(spi_device_t *spi,
                        const uint8_t *tx, uint8_t *rx, size_t len)
{
    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = (unsigned long)rx,
        .len           = len,
        .speed_hz      = spi->speed_hz,
        .bits_per_word = spi->bits_per_word,
        .delay_usecs   = 0,
    };

    if (ioctl(spi->fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("spi_transfer");
        return -1;
    }
    return 0;
}
