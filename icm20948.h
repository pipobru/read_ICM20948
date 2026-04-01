/* icm20948.h */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "spi_custom.h"
//#include "AK09916.h"

/* ================================================================== */
/* REGISTRES                                                           */
/* ================================================================== */

/* Bank 0 */
#define ICM20948_WHO_AM_I      0x00
#define ICM20948_USER_CTRL     0x03
#define ICM20948_PWR_MGMT_1    0x06
#define ICM20948_PWR_MGMT_2    0x07
#define ICM20948_FIFO_EN_1     0x66
#define ICM20948_FIFO_EN_2     0x67
#define ICM20948_FIFO_RST      0x68
#define ICM20948_FIFO_MODE     0x69
#define ICM20948_FIFO_COUNTH   0x70
#define ICM20948_FIFO_COUNTL   0x71
#define ICM20948_FIFO_R_W      0x72
#define REG_BANK_SEL           0x7F
#define ICM20948_I2C_MST_STATUS     0x17
#define ICM20948_EXT_SLV_SENS_DATA_00  0x3B

/* Bank 2 */
#define ICM20948_GYRO_SMPLRT_DIV   0x00
#define ICM20948_GYRO_CONFIG_1     0x01
#define ICM20948_GYRO_CONFIG_2     0x02
#define ICM20948_ACCEL_SMPLRT_DIV1 0x10
#define ICM20948_ACCEL_SMPLRT_DIV2 0x11
#define ICM20948_ACCEL_CONFIG      0x14
#define ICM20948_ACCEL_CONFIG_2    0x15
#define ICM20948_TEMP_CONFIG       0x53



/* ================================================================== */
/* CONSTANTES                                                          */
/* ================================================================== */

#define ICM20948_WHO_AM_I_VAL  0xEA
#define SPI_READ               0x80
#define SPI_WRITE              0x00

/* Banks */
#define BANK_0   (0 << 4)
#define BANK_1   (1 << 4)
#define BANK_2   (2 << 4)
#define BANK_3   (3 << 4)

/* PWR_MGMT_1 */
#define PWR_MGMT1_RESET        (1 << 7)
#define PWR_MGMT1_SLEEP        (1 << 6)
#define PWR_MGMT1_CLKSEL_AUTO  0x01     /* Horloge auto (recommandé) */

/* PWR_MGMT_2 */
#define PWR_MGMT2_ALL_ON       0x00     /* Accel + Gyro actifs */

/* USER_CTRL */
#define USER_CTRL_FIFO_EN      (1 << 6)
#define USER_CTRL_FIFO_RST     (1 << 2)
#define USER_CTRL_I2C_MST_EN    (1 << 5)
#define USER_CTRL_I2C_IF_DIS    (1 << 4)  /* obligatoire en mode SPI */
#define USER_CTRL_I2C_MST_RST   (1 << 1)

/* FIFO_EN_2 */
/*
#define FIFO_EN2_GYRO_X        (1 << 4)
#define FIFO_EN2_GYRO_Y        (1 << 3)
#define FIFO_EN2_GYRO_Z        (1 << 2)
#define FIFO_EN2_ACCEL         (1 << 1)
*/
#define FIFO_EN2_ACCEL         (1 << 4)
#define FIFO_EN2_GYRO_Z        (1 << 3)
#define FIFO_EN2_GYRO_Y        (1 << 2)
#define FIFO_EN2_GYRO_X        (1 << 1)
#define FIFO_EN2_TEMP_FIFO     (1 << 0)
#define FIFO_EN1_SLV0          (1 << 0) /* compass via SLV0 → FIFO */

/* GYRO_CONFIG_1 */
#define GYRO_FCHOICE_ENABLE    (1 << 0)
#define GYRO_FS_250DPS         (0b00 << 1)
#define GYRO_FS_500DPS         (0b01 << 1)
#define GYRO_FS_1000DPS        (0b10 << 1)
#define GYRO_FS_2000DPS        (0b11 << 1)
#define GYRO_DLPF_196HZ        (0 << 3)
#define GYRO_DLPF_152HZ        (1 << 3)
#define GYRO_DLPF_120HZ        (2 << 3)
#define GYRO_DLPF_51HZ         (3 << 3)
#define GYRO_DLPF_24HZ         (4 << 3)
#define GYRO_DLPF_12HZ         (5 << 3)
#define GYRO_DLPF_6HZ          (6 << 3)

/* ACCEL_CONFIG */
#define ACCEL_FCHOICE_ENABLE   (1 << 0)
#define ACCEL_FS_2G            (0b00 << 1)
#define ACCEL_FS_4G            (0b01 << 1)
#define ACCEL_FS_8G            (0b10 << 1)
#define ACCEL_FS_16G           (0b11 << 1)
#define ACCEL_DLPF_246HZ       (0 << 3)
#define ACCEL_DLPF_111HZ       (2 << 3)
#define ACCEL_DLPF_50HZ        (3 << 3)
#define ACCEL_DLPF_24HZ        (4 << 3)
#define ACCEL_DLPF_12HZ        (5 << 3)
#define ACCEL_DLPF_6HZ         (6 << 3)

/* Taille paquet FIFO */
/*
 * Paquet FIFO avec 9 octets compass :
 *  [0 - 5]  accel X/Y/Z      big-endian
 *  [6 -11]  gyro  X/Y/Z      big-endian
 *  [12]     ST1
 *  [13]     MAG_XL  ─┐
 *  [14]     MAG_XH   │ little-endian
 *  [15]     MAG_YL   │
 *  [16]     MAG_YH   │
 *  [17]     MAG_ZL   │
 *  [18]     MAG_ZH  ─┘
 *  [19]     gap (0x00)        ← octet 0x17, vide
 *  [20]     ST2
 * ─────────────────────────────────────
 * Total : 21 octets
 */
#define FIFO_PACKET_SIZE       23   /* accel(6)+gyro(6)+ST1(1)+mag(6)+ST2(1)+temp */
#define FIFO_MAX_PACKETS       22   /* 512 / 22           */

/* ================================================================== */
/* AK09916  (La boussole intégrée)                                    */
/* ================================================================== */
#define AK09916_I2C_ADDR        0x0C
#define AK09916_WIA1            0x00   /* Company ID = 0x48 */
#define AK09916_WIA2            0x01   /* Device ID  = 0x09 */
#define AK09916_ST1             0x10   /* Status 1 (DRDY)   */
#define AK09916_HXL             0x11   /* Mag X LSB         */
#define AK09916_ST2             0x18   /* Status 2 (HOFL)   */
#define AK09916_CNTL2           0x31   /* Mode de mesure    */
#define AK09916_CNTL3           0x32   /* Reset soft        */

#define AK09916_MODE_POWERDOWN  0x00
#define AK09916_MODE_CONT_10HZ  0x02
#define AK09916_MODE_CONT_20HZ  0x04
#define AK09916_MODE_CONT_50HZ  0x06
#define AK09916_MODE_CONT_100HZ 0x08

/* SLV0 lit ST1+HX+HY+HZ+ST2 = 8 octets depuis AK09916_ST1 */
#define AK09916_SLV0_LEN        8

/* Bank 3 */
#define ICM20948_I2C_MST_ODR_CFG    0x00
#define ICM20948_I2C_MST_CTRL       0x01
#define ICM20948_I2C_MST_DELAY_CTRL 0x02
#define ICM20948_I2C_SLV0_ADDR      0x03
#define ICM20948_I2C_SLV0_REG       0x04
#define ICM20948_I2C_SLV0_CTRL      0x05
#define ICM20948_I2C_SLV0_DO        0x06

/*
* Declaration des valeurs pour les differents filtres
*/
/* ================================================================== */
/* GYRO DLPF — FCHOICE=1 requis                                       */
/* ================================================================== */
/*
 *  CFG  Bandwidth   Noise   Délai
 *   0    196.6 Hz   élevé   0.6 ms
 *   1    151.8 Hz     ↓     1.1 ms
 *   2    119.5 Hz     ↓     1.9 ms
 *   3     51.2 Hz     ↓     4.5 ms   ← bon équilibre
 *   4     23.9 Hz     ↓     8.9 ms
 *   5     11.6 Hz     ↓    17.0 ms
 *   6      5.7 Hz   faible 33.5 ms   ← très filtré
 *   7    361.4 Hz   élevé   0.3 ms
 */
#define GYRO_DLPF_197HZ    (0 << 3)
#define GYRO_DLPF_152HZ    (1 << 3)
#define GYRO_DLPF_120HZ    (2 << 3)
#define GYRO_DLPF_51HZ     (3 << 3)   /* recommandé usage général */
#define GYRO_DLPF_24HZ     (4 << 3)   /* recommandé vibrations    */
#define GYRO_DLPF_12HZ     (5 << 3)
#define GYRO_DLPF_6HZ      (6 << 3)   /* très stable, latence max */
#define GYRO_DLPF_361HZ    (7 << 3)

/* ================================================================== */
/* GYRO AVERAGING — mode low power uniquement                         */
/* ================================================================== */
/*
 *  CFG  Nb moyennes
 *   0     1  (pas de moyennage)
 *   1     2
 *   2     4
 *   3     8
 *   4    16
 *   5    32
 *   6    64
 *   7   128                    ← maximum
 */
#define GYRO_AVG_1X     0
#define GYRO_AVG_2X     1
#define GYRO_AVG_4X     2
#define GYRO_AVG_8X     3
#define GYRO_AVG_16X    4
#define GYRO_AVG_32X    5
#define GYRO_AVG_64X    6
#define GYRO_AVG_128X   7

/* ================================================================== */
/* ACCEL DLPF — FCHOICE=1 requis                                      */
/* ================================================================== */
/*
 *  CFG  Bandwidth   Délai
 *   0    246.0 Hz   1.0 ms
 *   1    246.0 Hz   1.0 ms
 *   2    111.4 Hz   2.9 ms
 *   3     50.4 Hz   5.9 ms   ← bon équilibre
 *   4     23.9 Hz  11.5 ms
 *   5     11.5 Hz  23.0 ms
 *   6      5.7 Hz  45.9 ms   ← très filtré
 *   7    473.0 Hz   0.6 ms
 */
#define ACCEL_DLPF_246HZ   (0 << 3)
#define ACCEL_DLPF_111HZ   (2 << 3)
#define ACCEL_DLPF_50HZ    (3 << 3)   /* recommandé usage général */
#define ACCEL_DLPF_24HZ    (4 << 3)   /* recommandé vibrations    */
#define ACCEL_DLPF_12HZ    (5 << 3)
#define ACCEL_DLPF_6HZ     (6 << 3)
#define ACCEL_DLPF_473HZ   (7 << 3)

/* ================================================================== */
/* ACCEL DECIMATION — moyennage hardware                              */
/* ================================================================== */
/*
 *  CFG  Nb échantillons moyennés
 *   0     4  (défaut)
 *   1     8
 *   2    16
 *   3    32                    ← maximum
 */
#define ACCEL_DEC3_4X    0
#define ACCEL_DEC3_8X    1
#define ACCEL_DEC3_16X   2
#define ACCEL_DEC3_32X   3

/* ================================================================== */
/* TEMP DLPF                                                          */
/* ================================================================== */
/*
 *  CFG  Bandwidth   Délai
 *   0    7932 Hz    0.1 ms
 *   1     217 Hz    1.0 ms
 *   2     123 Hz    2.0 ms
 *   3      66 Hz    3.0 ms
 *   4      34 Hz    5.0 ms
 *   5      17 Hz    9.0 ms
 *   6       9 Hz   17.0 ms
 *   7       9 Hz   17.0 ms
 */
#define TEMP_DLPF_7932HZ   0
#define TEMP_DLPF_217HZ    1
#define TEMP_DLPF_123HZ    2
#define TEMP_DLPF_66HZ     3
#define TEMP_DLPF_34HZ     4
#define TEMP_DLPF_17HZ     5
#define TEMP_DLPF_9HZ      6

/* ================================================================== */
/* PRESETS de configurations    
## Guide de choix
```
Symptôme                    → Solution
──────────────────────────────────────────────────────
Valeurs gyro qui tremblent  → GYRO_DLPF_24HZ + AVG_8X
Vibrations mécaniques       → ACCEL_DLPF_24HZ + DEC3_16X
Latence trop élevée         → DLPF plus élevé (120Hz+)
Données gyro saturent       → GYRO_FS plus grand
Données accel saturent      → ACCEL_FS plus grand
Bruit résiduel après DLPF   → augmenter AVG ou DEC3                                      */
/* ================================================================== */
typedef struct {
    const char *name;
    uint8_t  gyro_fs;
    uint8_t  gyro_dlpf;
    uint8_t  gyro_avg;
    uint8_t  gyro_smplrt_div;
    uint8_t  accel_fs;
    uint8_t  accel_dlpf;
    uint8_t  accel_dec3;
    uint16_t accel_smplrt_div;
    uint8_t  temp_dlpf;

    /* FIFO */
    uint8_t  fifo_mode;        /* 0=stream, 1=stop-on-full */
} icm20948_filter_preset_t;

/*
 * PRESET 1 — Drone / robot rapide
 *   Priorité : faible latence, bande passante large
 */
static const icm20948_filter_preset_t PRESET_FAST = {
    .name             = "FAST (drone/robot)",
    .gyro_fs          = GYRO_FS_500DPS,
    .gyro_dlpf        = GYRO_DLPF_120HZ,
    .gyro_avg         = GYRO_AVG_1X,
    .gyro_smplrt_div  = 0x04,          /* ODR = 225 Hz */
    .accel_fs         = ACCEL_FS_4G,
    .accel_dlpf       = ACCEL_DLPF_111HZ,
    .accel_dec3       = ACCEL_DEC3_4X,
    .accel_smplrt_div = 0x04,          /* ODR = 225 Hz */
    .temp_dlpf        = TEMP_DLPF_123HZ,
    .fifo_mode        = 0,                    /* stream (overwrite) */
};

/*
 * PRESET 2 — Navigation / stabilisation
 *   Priorité : équilibre latence/bruit
 */
static const icm20948_filter_preset_t PRESET_BALANCED = {
    .name             = "BALANCED (navigation)",
    .gyro_fs          = GYRO_FS_500DPS,
    .gyro_dlpf        = GYRO_DLPF_51HZ,
    .gyro_avg         = GYRO_AVG_4X,
    .gyro_smplrt_div  = 0x09,          /* ODR = 112.5 Hz */
    .accel_fs         = ACCEL_FS_4G,
    .accel_dlpf       = ACCEL_DLPF_50HZ,
    .accel_dec3       = ACCEL_DEC3_8X,
    .accel_smplrt_div = 0x09,          /* ODR = 112.5 Hz */
    .temp_dlpf        = TEMP_DLPF_66HZ,
    .fifo_mode        = 0,                    /* stream (overwrite) */
};

/*
 * PRESET 3 — Données statiques / faible bruit
 *   Priorité : minimum de bruit, latence non critique
 */
static const icm20948_filter_preset_t PRESET_STABLE = {
    .name             = "STABLE (faible bruit)",
    .gyro_fs          = GYRO_FS_250DPS,
    .gyro_dlpf        = GYRO_DLPF_24HZ,
    .gyro_avg         = GYRO_AVG_32X,
    .gyro_smplrt_div  = 0x15,          /* ODR = 50 Hz */
    .accel_fs         = ACCEL_FS_2G,
    .accel_dlpf       = ACCEL_DLPF_24HZ,
    .accel_dec3       = ACCEL_DEC3_32X,
    .accel_smplrt_div = 0x15,          /* ODR = 50 Hz */
    .temp_dlpf        = TEMP_DLPF_17HZ,
    .fifo_mode        = 0,                    /* stream (overwrite) */
};

/* ── Structure d'un sample ───────────────────────────────────────── */
typedef struct {
    int16_t accel_x, accel_y, accel_z;  //Accelerateur en g
    int16_t gyro_x,  gyro_y,  gyro_z;   //Gyroscope en vitesse angulaire en °/s
    int16_t mag_x,   mag_y,   mag_z;    //Magnétomètre en µT
    uint8_t mag_st1;               /* bit0=DRDY                  */
    uint8_t mag_st2;               /* bit3=HOFL (overflow)       */
    uint8_t mag_valid;             /* 1 si DRDY=1 et HOFL=0      */
    int16_t temp;
} icm20948_sample_t;

typedef struct {
    int fd;           /* descripteur /dev/spidevX.Y */
} ICM20948;

/*
 * Declaration des fonctions d'accès au capteur ICM-20948
 */
static int icm20948_set_bank(spi_device_t *spi, uint8_t bank);
static int icm20948_read_reg(spi_device_t *spi, uint8_t reg, uint8_t *val);
static int icm20948_read_burst(spi_device_t *spi, uint8_t reg,
                               uint8_t *buf, size_t len);
static int icm20948_write_reg(spi_device_t *spi, uint8_t reg, uint8_t val);
static int init_compass(spi_device_t *spi);
int icm20948_who_am_i(spi_device_t *spi);
int icm20948_init(spi_device_t *spi, const icm20948_filter_preset_t *cfg);
int icm20948_filter_apply(spi_device_t *spi,
                          const icm20948_filter_preset_t *p);
void icm20948_convert(const icm20948_sample_t *sample,
                        float *gx, float *gy, float *gz,
                        float *ax, float *ay, float *az,
                        float *compx, float *compy, float *compz,
                        float *cap, float *temp);

/*
    * Applique un preset de filtrage (DLPF, moyennage, ODR) sur le capteur.
    * Retourne 0 en cas de succès, -1 en cas d'erreur.  
*/
int icm20948_filter_apply(spi_device_t *spi,
                          const icm20948_filter_preset_t *p)
{
    uint8_t val;
    int     ret = 0;

    printf("[FILTER] Application preset : %s\n", p->name);

    /* ── Bank 2 ── */
    ret = icm20948_set_bank(spi, BANK_2);
    if (ret < 0) return -1;

    /* GYRO_CONFIG_1 : FCHOICE + FS + DLPF */
    val = GYRO_FCHOICE_ENABLE | p->gyro_fs | p->gyro_dlpf;
    ret = icm20948_write_reg(spi, ICM20948_GYRO_CONFIG_1, val);
    if (ret < 0) return -1;
    printf("[FILTER] GYRO_CONFIG_1  = 0x%02X "
           "(FCHOICE=1 FS=0x%02X DLPF=0x%02X)\n",
           val, p->gyro_fs >> 1, p->gyro_dlpf >> 3);

    /* GYRO_CONFIG_2 : averaging */
    val = p->gyro_avg & 0x07;
    ret = icm20948_write_reg(spi, ICM20948_GYRO_CONFIG_2, val);
    if (ret < 0) return -1;
    printf("[FILTER] GYRO_CONFIG_2  = 0x%02X (AVG=%dx)\n",
           val, 1 << p->gyro_avg);

    /* GYRO_SMPLRT_DIV */
    ret = icm20948_write_reg(spi, ICM20948_GYRO_SMPLRT_DIV,
                             p->gyro_smplrt_div);
    if (ret < 0) return -1;
    printf("[FILTER] GYRO ODR       = %.1f Hz\n",
           1125.0f / (1 + p->gyro_smplrt_div));

    /* ACCEL_CONFIG : FCHOICE + FS + DLPF */
    val = ACCEL_FCHOICE_ENABLE | p->accel_fs | p->accel_dlpf;
    ret = icm20948_write_reg(spi, ICM20948_ACCEL_CONFIG, val);
    if (ret < 0) return -1;
    printf("[FILTER] ACCEL_CONFIG   = 0x%02X "
           "(FCHOICE=1 FS=0x%02X DLPF=0x%02X)\n",
           val, p->accel_fs >> 1, p->accel_dlpf >> 3);

    /* ACCEL_CONFIG_2 : decimation */
    val = p->accel_dec3 & 0x07;
    ret = icm20948_write_reg(spi, ICM20948_ACCEL_CONFIG_2, val);
    if (ret < 0) return -1;
    printf("[FILTER] ACCEL_CONFIG_2 = 0x%02X (DEC3=%dx)\n",
           val, 4 << p->accel_dec3);

    /* ACCEL_SMPLRT_DIV sur 12 bits */
    ret = icm20948_write_reg(spi, ICM20948_ACCEL_SMPLRT_DIV1,
                             (p->accel_smplrt_div >> 8) & 0x0F);
    if (ret < 0) return -1;
    ret = icm20948_write_reg(spi, ICM20948_ACCEL_SMPLRT_DIV2,
                             p->accel_smplrt_div & 0xFF);
    if (ret < 0) return -1;
    printf("[FILTER] ACCEL ODR      = %.1f Hz\n",
           1125.0f / (1 + p->accel_smplrt_div));

    /* TEMP_CONFIG */
    ret = icm20948_write_reg(spi, ICM20948_TEMP_CONFIG,
                             p->temp_dlpf & 0x07);
    if (ret < 0) return -1;
    printf("[FILTER] TEMP_CONFIG    = 0x%02X\n", p->temp_dlpf);

    icm20948_set_bank(spi, BANK_0);

    printf("[FILTER] Preset appliqué ✓\n");
    return 0;
}

/* ------------------------------------------------------------------ */
/* Permet de changer de bank, toujours revenir sur la bank 0          */
/* ------------------------------------------------------------------ */
/*
Bank 0: Données capteurs, FIFO, WHO_AM_I, interruptions
Bank 1: Calibration gyro/accel (offset, trim)
Bank 2: Configuration gyro, accél, DLPF, ODR
Bank 3: Configuration I2C master (pour le magnéto)
*/
static int icm20948_set_bank(spi_device_t *spi, uint8_t bank)
{
    uint8_t tx[2] = { SPI_WRITE | REG_BANK_SEL, bank };
    uint8_t rx[2];

    return spi_transfer(spi, tx, rx, 2);
}

/* ------------------------------------------------------------------ */
/* Lecture d'un registre ICM-20948                                     */
/* ------------------------------------------------------------------ */
static int icm20948_read_reg(spi_device_t *spi, uint8_t reg, uint8_t *val)
{
    uint8_t tx[2] = { SPI_READ | reg, 0x00 };
    uint8_t rx[2] = { 0 };

    if (spi_transfer(spi, tx, rx, 2) < 0)
        return -1;

    *val = rx[1];
    return 0;
}
/* ------------------------------------------------------------------ */
/* Lecture multi-octets depuis un registre                             */
/* ------------------------------------------------------------------ */
static int icm20948_read_burst(spi_device_t *spi, uint8_t reg,
                               uint8_t *buf, size_t len)
{
    uint8_t tx[len + 1];
    uint8_t rx[len + 1];

    memset(tx, 0, sizeof(tx));
    tx[0] = SPI_READ | reg;

    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = (unsigned long)rx,
        .len           = len + 1,
        .speed_hz      = spi->speed_hz,
        .bits_per_word = spi->bits_per_word,
    };

    if (ioctl(spi->fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("icm20948_read_burst");
        return -1;
    }

    memcpy(buf, &rx[1], len);
    return 0;
}
/* ------------------------------------------------------------------ */
/* Ecriture d'un registre ICM-20948                                     */
/* ------------------------------------------------------------------ */
static int icm20948_write_reg(spi_device_t *spi, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = { SPI_WRITE | reg, val };
    uint8_t rx[2];

    return spi_transfer(spi, tx, rx, 2);
}

/* ------------------------------------------------------------------ */
/* WHO_AM_I : identifie et valide le composant                         */
/* ------------------------------------------------------------------ */
int icm20948_who_am_i(spi_device_t *spi)
{
    uint8_t who = 0;

    /* Forcer le mode SPI : envoyer un dummy byte CS bas */
    uint8_t tx[2] = { 0xFF, 0xFF };
    uint8_t rx[2] = { 0 };

    /* Premier transfert pour commuter en mode SPI */
    if(spi_transfer(spi, tx, rx, 2) < 0) {
        fprintf(stderr, "icm20948_who_am_i: échec transfert SPI\n");
        return -1;
    }
    usleep(1000);  // 1 ms


    if (icm20948_read_reg(spi, ICM20948_WHO_AM_I, &who) < 0) {
        fprintf(stderr, "icm20948_who_am_i: échec lecture SPI\n");
        return -1;
    }

    printf("WHO_AM_I = 0x%02X ", who);

    if (who == ICM20948_WHO_AM_I_VAL) {
        printf("→ ICM-20948 détecté ✓\n");
        return 0;
    } else {
        printf("→ valeur inattendue (attendu 0x%02X) ✗\n",
               ICM20948_WHO_AM_I_VAL);
        return -1;
    }
}

int icm20948_gyro_config1_init(spi_device_t *spi,
                                uint8_t fs_sel,
                                uint8_t dlpf_cfg)
{
    int     ret;
    uint8_t val;

    ret = icm20948_set_bank(spi, BANK_2);
    if (ret < 0) return -1;

    /* Activer FCHOICE + pleine échelle + DLPF */
    val = GYRO_FCHOICE_ENABLE | fs_sel | dlpf_cfg;

    ret = icm20948_write_reg(spi, ICM20948_GYRO_CONFIG_1, val);
    if (ret < 0) {
        fprintf(stderr, "gyro_config1_init: write_reg failed\n");
        icm20948_set_bank(spi, BANK_0);
        return -1;
    }

    printf("GYRO_CONFIG_1 = 0x%02X (FCHOICE=1, FS=%s, DLPF=0x%X)\n",
           val,
           (fs_sel == GYRO_FS_250DPS)  ? "±250°/s"  :
           (fs_sel == GYRO_FS_500DPS)  ? "±500°/s"  :
           (fs_sel == GYRO_FS_1000DPS) ? "±1000°/s" : "±2000°/s",
           dlpf_cfg >> 3);

    return icm20948_set_bank(spi, BANK_0);
}
int icm20948_gyro_smplrt_init(spi_device_t *spi, uint8_t divider)
{
    int ret;

    /* Aller en bank 2 */
    ret = icm20948_set_bank(spi, BANK_2);
    if (ret < 0) {
        fprintf(stderr, "gyro_smplrt_init: set_bank failed\n");
        return -1;
    }

    /* Écrire le diviseur */
    ret = icm20948_write_reg(spi, ICM20948_GYRO_SMPLRT_DIV, divider);
    if (ret < 0) {
        fprintf(stderr, "gyro_smplrt_init: write_reg failed\n");
        icm20948_set_bank(spi, BANK_0);
        return -1;
    }

    printf("GYRO_SMPLRT_DIV = 0x%02X → ODR = %.1f Hz\n",
           divider, 1125.0f / (1 + divider));

    /* Revenir en bank 0 */
    return icm20948_set_bank(spi, BANK_0);
}
/* ------------------------------------------------------------------ */
/* Initialisation.                                                    */
/* ------------------------------------------------------------------ */

static int init_reset(spi_device_t *spi)
{
    icm20948_set_bank(spi, BANK_0);

    if (icm20948_write_reg(spi, ICM20948_PWR_MGMT_1, PWR_MGMT1_RESET) < 0)
        return -1;

    usleep(100000);  /* 100 ms — attendre fin du reset */

    printf("[INIT] Reset effectué\n");
    return 0;
}

/* ── 3. Sortie du mode sleep + sélection horloge ─────────────────── */
static int init_power(spi_device_t *spi)
{
    icm20948_set_bank(spi, BANK_0);

    /* Sortir du sleep, horloge automatique */
    if (icm20948_write_reg(spi, ICM20948_PWR_MGMT_1,
                           PWR_MGMT1_CLKSEL_AUTO) < 0)
        return -1;

    /* Activer accel + gyro */
    if (icm20948_write_reg(spi, ICM20948_PWR_MGMT_2,
                           PWR_MGMT2_ALL_ON) < 0)
        return -1;

/*
     * I2C_IF_DIS (bit4) : désactive l'interface I2C principale
     * OBLIGATOIRE en mode SPI → libère le bus auxiliaire pour
     * le I2C master interne (AK09916)
     * Ce bit doit être conservé dans TOUS les écritures USER_CTRL
     */
    if (icm20948_write_reg(spi, ICM20948_USER_CTRL,
                           USER_CTRL_I2C_IF_DIS) < 0)
        return -1;
    usleep(10000);

    printf("[INIT] Power OK — USER_CTRL=0x%02X "
           "(I2C_IF_DIS actif)\n", USER_CTRL_I2C_IF_DIS);
    return 0;
}

void icm20948_fifo_debug(spi_device_t *spi)
{
    uint8_t user_ctrl, fifo_en1, fifo_en2;
    uint8_t fifo_mode, fifo_rst;
    uint8_t pwr1, pwr2;
    uint8_t cnt_h, cnt_l;

    icm20948_set_bank(spi, BANK_0);

    icm20948_read_reg(spi, ICM20948_PWR_MGMT_1,  &pwr1);
    icm20948_read_reg(spi, ICM20948_PWR_MGMT_2,  &pwr2);
    icm20948_read_reg(spi, ICM20948_USER_CTRL,   &user_ctrl);
    icm20948_read_reg(spi, ICM20948_FIFO_EN_1,   &fifo_en1);
    icm20948_read_reg(spi, ICM20948_FIFO_EN_2,   &fifo_en2);
    icm20948_read_reg(spi, ICM20948_FIFO_MODE,   &fifo_mode);
    icm20948_read_reg(spi, ICM20948_FIFO_RST,    &fifo_rst);
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTH, &cnt_h);
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTL, &cnt_l);

    uint16_t count = ((uint16_t)cnt_h << 8) | cnt_l;

    printf("=== FIFO DEBUG ===\n");
    printf("PWR_MGMT_1  = 0x%02X  (sleep=%d, clk=%d)\n",
           pwr1, (pwr1>>6)&1, pwr1&7);
    printf("PWR_MGMT_2  = 0x%02X  (doit être 0x00)\n", pwr2);
    printf("USER_CTRL   = 0x%02X  (FIFO_EN bit6=%d)\n",
           user_ctrl, (user_ctrl>>6)&1);
    printf("FIFO_EN_1   = 0x%02X  (doit être 0x1E)\n", fifo_en1);
    printf("FIFO_EN_2   = 0x%02X\n", fifo_en2);
    printf("FIFO_MODE   = 0x%02X\n", fifo_mode);
    printf("FIFO_RST    = 0x%02X  (doit être 0x00)\n", fifo_rst);
    printf("FIFO_COUNT  = %u octets (%u samples)\n",
           count, count / FIFO_PACKET_SIZE);
    printf("==================\n");
}

/* ── 6. Initialisation FIFO ──────────────────────────────────────── */
static int init_fifo(spi_device_t *spi, const icm20948_filter_preset_t *cfg)
{
    uint8_t val;

    icm20948_set_bank(spi, BANK_0);

    /* ── Étape 1 : désactiver le FIFO dans USER_CTRL ── */
    icm20948_read_reg(spi, ICM20948_USER_CTRL, &val);
    val &= ~USER_CTRL_FIFO_EN;
    icm20948_write_reg(spi, ICM20948_USER_CTRL, val);

    /* ── Étape 2 : couper les sources FIFO ── */
    icm20948_write_reg(spi, ICM20948_FIFO_EN_1, 0x00);
    icm20948_write_reg(spi, ICM20948_FIFO_EN_2, 0x00);

    /* ── Étape 3 : reset propre ── */
    icm20948_write_reg(spi, ICM20948_FIFO_RST, 0x1F);
    usleep(10000);
    icm20948_write_reg(spi, ICM20948_FIFO_RST, 0x00);
    usleep(10000);

    /* Vérifier reset relâché */
    icm20948_read_reg(spi, ICM20948_FIFO_RST, &val);
    if (val != 0x00) {
        fprintf(stderr, "[INIT] FIFO_RST non relâché : 0x%02X\n", val);
        return -1;
    }

    /* ── 4. Mode stream (overwrite) ── */
    icm20948_write_reg(spi, ICM20948_FIFO_MODE,
                       cfg->fifo_mode ? 0x1F : 0x00);


    /* ── 5. Activer FIFO en préservant I2C_IF_DIS + I2C_MST_EN ── */
    icm20948_read_reg(spi, ICM20948_USER_CTRL, &val);
    val |= USER_CTRL_FIFO_EN
        |  USER_CTRL_I2C_MST_EN
        |  USER_CTRL_I2C_IF_DIS;
    icm20948_write_reg(spi, ICM20948_USER_CTRL, val);
    usleep(1000);

/* ── 6. Connecter les sources ── */
    /*
     * FIFO_EN_2 : accel + gyroX/Y/Z   (capteurs internes)
     * FIFO_EN_1 : SLV0               (compass via I2C master)
     */
    icm20948_write_reg(spi, ICM20948_FIFO_EN_2,
                       FIFO_EN2_TEMP_FIFO |
                       FIFO_EN2_GYRO_X |
                       FIFO_EN2_GYRO_Y |
                       FIFO_EN2_GYRO_Z |
                       FIFO_EN2_ACCEL);

    icm20948_write_reg(spi, ICM20948_FIFO_EN_1,
                       FIFO_EN1_SLV0);

    /* ── 7. Attendre au moins 2 samples ── */
    usleep(30000);  /* 30 ms > 2 × 1/100Hz × 1000 */

    /* ── 8. Vérifier compteur FIFO ── */
    uint8_t h, l;
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTH, &h);
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTL, &l);
    uint16_t count = ((uint16_t)h << 8) | l;

    if (count < FIFO_PACKET_SIZE) {
        fprintf(stderr, "[INIT] FIFO vide après init : %u octets\n", count);
        return -1;
    }

    /* Vérifier cohérence : multiple de FIFO_PACKET_SIZE */
    if (count % FIFO_PACKET_SIZE != 0) {
        fprintf(stderr, "[INIT] FIFO désaligné : %u octets "
                "(paquet=%d)\n", count, FIFO_PACKET_SIZE);
        /* Reset pour réaligner */
        icm20948_write_reg(spi, ICM20948_FIFO_RST, 0x1F);
        usleep(1000);
        icm20948_write_reg(spi, ICM20948_FIFO_RST, 0x00);
    }

    printf("[INIT] FIFO OK : %u octets — %u samples "
           "(%d octets/sample) ✓\n",
           count, count / FIFO_PACKET_SIZE, FIFO_PACKET_SIZE);
    return 0;
}

int icm20948_init(spi_device_t *spi, const icm20948_filter_preset_t *cfg)
{
    printf("=== ICM-20948 Init ===\n");

    uint8_t tx[2] = { 0xFF, 0xFF }, rx[2];
    spi_transfer(spi, tx, rx, 2);
    usleep(1000);
    //On se positionne sur la bank 0
    icm20948_set_bank(spi, BANK_0);

       if (init_reset(spi)         < 0) return -1;
    if (icm20948_who_am_i(spi)  < 0) return -1;  /* re-vérif post-reset */
    if (init_power(spi)         < 0) return -1;
    if (icm20948_filter_apply(spi, cfg) < 0) return -1;
    if(init_compass(spi)   < 0) return -1;
    if (init_fifo(spi, cfg)     < 0) return -1;

    printf("=== Init terminée ===\n");
    return 0;

}

/*
Lecture du FIFO
*/
/* ------------------------------------------------------------------ */
/* Lecture du nombre d'octets disponibles dans le FIFO                */
/* ------------------------------------------------------------------ */
static int icm20948_fifo_count(spi_device_t *spi, uint16_t *count)
{
    uint8_t h = 0, l = 0;

    if (icm20948_read_reg(spi, ICM20948_FIFO_COUNTH, &h) < 0) return -1;
    if (icm20948_read_reg(spi, ICM20948_FIFO_COUNTL, &l) < 0) return -1;

    *count = ((uint16_t)h << 8) | l;
    return 0;
}
/* ------------------------------------------------------------------ */
/* Lecture des samples depuis le FIFO                                 */
/* ------------------------------------------------------------------ */
int icm20948_fifo_read(spi_device_t *spi,
                       icm20948_sample_t *samples,
                       int max_samples,
                       int *nb_read)
{
uint8_t h, l;
    *nb_read = 0;

    icm20948_set_bank(spi, BANK_0);

    icm20948_read_reg(spi, ICM20948_FIFO_COUNTH, &h);
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTL, &l);
    uint16_t count = ((uint16_t)h << 8) | l;

    int n = count / FIFO_PACKET_SIZE;
    if (n == 0)  return 0;
    if (n > max_samples) n = max_samples;

    for (int i = 0; i < n; i++) {
        uint8_t buf[FIFO_PACKET_SIZE];

        if (icm20948_read_burst(spi, ICM20948_FIFO_R_W,
                                buf, FIFO_PACKET_SIZE) < 0)
            return -1;

        /*
         * Structure paquet (20 octets) :
         *  [0- 5] accel X/Y/Z  big-endian
         *  [6-11] gyro  X/Y/Z  big-endian
         *  [12]   ST1           (DRDY bit0)
         *  [13]   MAG_XL  ─┐
         *  [14]   MAG_XH   │  little-endian (AK09916)
         *  [15]   MAG_YL   │
         *  [16]   MAG_YH   │
         *  [17]   MAG_ZL   │
         *  [18]   MAG_ZH  ─┘
         *  [19]   gap
         *  [20]   st2          (HOFL bit3)
         * [21]   TEMP           (optionnel, pas dans FIFO si FIFO_EN2_TEMP_FIFO=0)
         * [22]   TEMP           (optionnel, pas dans FIFO si FIFO_EN2_TEMP_FIFO=0)
         */

        /* Accel — big-endian */
        samples[i].accel_x = (int16_t)((buf[0]  << 8) | buf[1]);
        samples[i].accel_y = (int16_t)((buf[2]  << 8) | buf[3]);
        samples[i].accel_z = (int16_t)((buf[4]  << 8) | buf[5]);

        /* Gyro — big-endian */
        samples[i].gyro_x  = (int16_t)((buf[6]  << 8) | buf[7]);
        samples[i].gyro_y  = (int16_t)((buf[8]  << 8) | buf[9]);
        samples[i].gyro_z  = (int16_t)((buf[10] << 8) | buf[11]);

        /* Compass status */
        samples[i].mag_st1 = buf[12];
        samples[i].mag_st2 = buf[20];

        /* Mag — little-endian (AK09916) */
        samples[i].mag_x   = (int16_t)((buf[14] << 8) | buf[13]);
        samples[i].mag_y   = (int16_t)((buf[16] << 8) | buf[15]);
        samples[i].mag_z   = (int16_t)((buf[18] << 8) | buf[17]);

        /* Données mag valides si DRDY=1 et pas d'overflow */
        samples[i].mag_valid = ((samples[i].mag_st1 & 0x01) &&
                                !(samples[i].mag_st2 & 0x08)) ? 1 : 0;

        /*temperature - big-endian*/
        samples[i].temp    = (int16_t)((buf[21] << 8) | buf[22]);
    }

    *nb_read = n;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Conversion en unités physiques                                      */
/* ------------------------------------------------------------------ */
void icm20948_convert(const icm20948_sample_t *raw,
                      float *gx, float *gy, float *gz,
                      float *ax, float *ay, float *az,
                      float *compx, float *compy, float *compz,
                      float *cap,
                       float *temp)
{
    /* Sensibilité gyro pour ±500°/s */
    const float gyro_scale  = 1.0f / 65.5f;   /* °/s par LSB */

    /* Sensibilité accel pour ±4g */
    const float accel_scale = 1.0f / 8192.0f;  /* g par LSB  */

    /*La sensibilité du AK09916 est 0.15 µT/LSB*/
    *compx = raw->mag_x * 0.15f;
    *compy = raw->mag_y * 0.15f;
    *compz = raw->mag_z * 0.15f;
    *cap = atan2f(*compy, *compx) * (180.0f / M_PI); //M_PI est défini dans math.h
    if (*cap < 0) *cap += 360.0f;

    *gx = raw->gyro_x  * gyro_scale;
    *gy = raw->gyro_y  * gyro_scale;
    *gz = raw->gyro_z  * gyro_scale;

    *ax = raw->accel_x * accel_scale;
    *ay = raw->accel_y * accel_scale;
    *az = raw->accel_z * accel_scale;
    *temp =  (raw->temp / 333.87f) + 21.0f;  /* °C */ /// 333.87f + 21.0f
}
/* ------------------------------------------------------------------ */
/* Écriture vers AK09916 via SLV0 (one-shot)                          */
/* ------------------------------------------------------------------ */
static int ak09916_write(spi_device_t *spi, uint8_t reg, uint8_t val)
{
    icm20948_set_bank(spi, BANK_3);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_ADDR, AK09916_I2C_ADDR);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_REG,  reg);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_DO,   val);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_CTRL, (1 << 7) | 1);

    icm20948_set_bank(spi, BANK_0);
    usleep(10000);  /* attendre transaction I2C */
    return 0;
}

/* ------------------------------------------------------------------ */
/* Lecture depuis AK09916 via SLV0 avec retry                         */
/* ------------------------------------------------------------------ */
static int ak09916_read(spi_device_t *spi, uint8_t reg,
                        uint8_t *buf, uint8_t len)
{
    uint8_t status;

    icm20948_set_bank(spi, BANK_3);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_ADDR,
                       AK09916_I2C_ADDR | 0x80);   /* bit7=1 → lecture */
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_REG,  reg);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_CTRL, (1 << 7) | len);
    icm20948_set_bank(spi, BANK_0);

    /* Retry : attendre que le I2C master remonte les données */
    for (int i = 0; i < 10; i++) {
        usleep(20000);  /* 20 ms */

        icm20948_read_reg(spi, ICM20948_I2C_MST_STATUS, &status);
        if (status & (1 << 4)) {
            fprintf(stderr, "ak09916_read: NACK sur reg=0x%02X\n", reg);
            return -1;
        }

        icm20948_read_burst(spi, ICM20948_EXT_SLV_SENS_DATA_00, buf, len);

        /* Valide si au moins un octet non nul */
        for (int j = 0; j < len; j++) {
            if (buf[j] != 0x00)
                return 0;
        }
    }

    fprintf(stderr, "ak09916_read: données toujours nulles après 10 essais\n");
    return -1;
}

static int init_compass_slv0(spi_device_t *spi)
{
    icm20948_set_bank(spi, BANK_3);

    /* Lecture depuis ST1 (0x10), 9 octets */
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_ADDR,
                       AK09916_I2C_ADDR | 0x80);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_REG,  AK09916_ST1);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_CTRL,
                       (1 << 7) | AK09916_SLV0_LEN);

    icm20948_set_bank(spi, BANK_0);

    printf("[INIT] SLV0 : lecture %d octets depuis ST1 (0x10) "
           "→ ST2 inclus ✓\n", AK09916_SLV0_LEN);
    return 0;
}

static int init_compass(spi_device_t *spi)
{
    uint8_t val, d[2];

    /* ── 1. Activer I2C master en préservant I2C_IF_DIS ── */
    icm20948_set_bank(spi, BANK_0);
    icm20948_read_reg(spi, ICM20948_USER_CTRL, &val);

    val |= USER_CTRL_I2C_MST_EN
        |  USER_CTRL_I2C_IF_DIS
        |  USER_CTRL_I2C_MST_RST;   /* reset bus I2C */
    icm20948_write_reg(spi, ICM20948_USER_CTRL, val);
    usleep(10000);

    val &= ~USER_CTRL_I2C_MST_RST;  /* relâcher reset */
    icm20948_write_reg(spi, ICM20948_USER_CTRL, val);
    usleep(10000);

    /* ── 2. Configurer I2C master ── */
    icm20948_set_bank(spi, BANK_3);

    /* 0x17 : 400 kHz + MULT_MST_EN */
    icm20948_write_reg(spi, ICM20948_I2C_MST_CTRL, 0x17);

    /* ODR I2C master : 1100 / 2^4 = 68 Hz */
    icm20948_write_reg(spi, ICM20948_I2C_MST_ODR_CFG, 0x04);

    /* Activer delay sur SLV0 */
    icm20948_write_reg(spi, ICM20948_I2C_MST_DELAY_CTRL, (1 << 7) | 1);

    icm20948_set_bank(spi, BANK_0);
    usleep(10000);

    /* ── 3. Vérifier WHO_AM_I AK09916 (WIA1+WIA2) ── */
    if (ak09916_read(spi, AK09916_WIA1, d, 2) < 0)
        return -1;

    printf("[INIT] AK09916 WIA1=0x%02X (att. 0x48)  "
                        "WIA2=0x%02X (att. 0x09)",
           d[0], d[1]);

    if (d[0] != 0x48 || d[1] != 0x09) {
        printf(" ✗\n");
        return -1;
    }
    printf(" ✓\n");

    /* ── 4. Reset soft AK09916 ── */
    ak09916_write(spi, AK09916_CNTL3, 0x01);
    usleep(10000);

    /* ── 5. Power down explicite avant de changer de mode ── */
    ak09916_write(spi, AK09916_CNTL2, AK09916_MODE_POWERDOWN);
    usleep(10000);

    /* ── 6. Mode continu 100 Hz ── */
    ak09916_write(spi, AK09916_CNTL2, AK09916_MODE_CONT_100HZ);
    usleep(10000);

    /* -- 7. Vérifier que le mode a bien été écrit */
    uint8_t mode = 0;
    ak09916_read(spi, AK09916_CNTL2, &mode, 1);
    if ((mode & 0x1F) != AK09916_MODE_CONT_100HZ) {
        fprintf(stderr, "[INIT] AK09916 CNTL2: attendu 0x%02X, lu 0x%02X\n",
                AK09916_MODE_CONT_100HZ, mode);
        return -1;
    }
    printf("[INIT] AK09916 CNTL2 = 0x%02X ✓\n", mode);

    /* ── 8. Configurer SLV0 lecture continue ── */
    if (init_compass_slv0(spi) < 0) return -1;

    /* ── 9. Attendre 2 cycles avant de vérifier ── */
    usleep(30000);

icm20948_set_bank(spi, BANK_3);
    /* ── 10. Vérifier que DRDY remonte ── */
    uint8_t buf[9] = {0};
    ak09916_read(spi, AK09916_ST1, buf, 9);

    if (!(buf[0] & 0x01)) {
        fprintf(stderr, "[INIT] AK09916 DRDY=0 — capteur ne mesure pas\n");
//        return -1;
    }

    printf("[INIT] Compass AK09916 : mode continu 100 Hz  "
           "DRDY=%d  HOFL=%d ✓\n",
           buf[0] & 0x01, (buf[8] >> 3) & 1);
icm20948_set_bank(spi, BANK_0);

           return 0;
}
