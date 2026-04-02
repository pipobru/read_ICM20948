#include "icm20948.h"
#include "spi_custom.h"



int main(void)
{
    spi_device_t spi;
 
    if (spi_open(&spi, "/dev/spidev0.0", SPI_MODE_3, 8, 1000000) < 0)
        return 1;
    fprintf(
        stderr, "SPI ouvert : mode %d, %d bits/word, %d Hz\n",
        spi.mode, spi.bits_per_word, spi.speed_hz
    );     
    
    if (icm20948_who_am_i(&spi) < 0) {
        spi_close(&spi);
        return 1;
    }

    if(icm20948_init(&spi, &PRESET_BALANCED) < 0) {
        spi_close(&spi);
        return 1;
    }
    printf("ICM-20948 initialisé ✓\n");

    int nb_read = 0;
    icm20948_sample_t samples[nb_read];

    usleep(100000);
    icm20948_fifo_sources_debug(&spi);
    icm20948_fifo_dump_raw(&spi);

      /* Boucle de lecture */
    while (1) {
        usleep(10000);  /* 10 ms */

        if (icm20948_fifo_read(&spi, samples,
                               FIFO_MAX_PACKETS, &nb_read) < 0)
            break;

        for (int i = 0; i < nb_read; i++) {
            float gx, gy, gz, ax, ay, az, compx, compy, compz, cap, temp;
            icm20948_convert(&samples[i],
                             &gx, &gy, &gz,
                             &ax, &ay, &az,
                             &compx, &compy, &compz,
                             &cap,
                             &temp);
            fflush(stdout);
            printf("\n[%2d] Gyro: %7.2f %7.2f %7.2f °/s  "
                   "Accel: %6.3f %6.3f %6.3f g %7.2f °C "
                   "Compass: %6.2f %6.2f %6.2f µT "
                   "Cap: %6.1f °   ",
                   i, gx, gy, gz, ax, ay, az, temp, compx, compy, compz, cap);
        } 
    }

    spi_close(&spi);

    return 0;
}