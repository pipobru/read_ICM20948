#include "icm20948.h"
#include "spi_custom.h"



int main(void)
{
    spi_device_t spi;
     /* Config typique drone / robot */
    icm20948_config_t cfg = {
        .gyro_fs         = GYRO_FS_500DPS,
        .gyro_dlpf       = GYRO_DLPF_51HZ,
        .gyro_smplrt_div = 0x09,          /* ODR = 112.5 Hz */

        .accel_fs         = ACCEL_FS_4G,
        .accel_dlpf       = ACCEL_DLPF_50HZ,
        .accel_smplrt_div = 0x09,          /* ODR = 112.5 Hz */

        .fifo_mode = 0,                    /* stream (overwrite) */
    };

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

    if(icm20948_init(&spi, &cfg) < 0) {
        spi_close(&spi);
        return 1;
    }
    printf("ICM-20948 initialisé ✓\n");

    int nb_read = 0;
    icm20948_sample_t samples[nb_read];

      /* Boucle de lecture */
    while (1) {
        usleep(10000);  /* 10 ms */

        if (icm20948_fifo_read(&spi, samples,
                               FIFO_MAX_PACKETS, &nb_read) < 0)
            break;

        for (int i = 0; i < nb_read; i++) {
            float gx, gy, gz, ax, ay, az, temp;
            icm20948_convert(&samples[i],
                             &gx, &gy, &gz,
                             &ax, &ay, &az, &temp);
            fflush(stdout);
            printf("\r[%2d] Gyro: %7.2f %7.2f %7.2f °/s  "
                   "Accel: %6.3f %6.3f %6.3f g %7.2f °C",
                   i, gx, gy, gz, ax, ay, az, temp  );
        }
    }

    spi_close(&spi);

    return 0;
}
