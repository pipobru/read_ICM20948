/* icm20948.h */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h> //Bien pensser à rajouter l'option -lm lors de la compilation pour lier la librairie mathématique

#include "spi_custom.h"

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

/* Bank 3 — SLV4 one-shot */
#define ICM20948_I2C_SLV4_ADDR   0x13
#define ICM20948_I2C_SLV4_REG    0x14
#define ICM20948_I2C_SLV4_CTRL   0x15
#define ICM20948_I2C_SLV4_DO     0x16
#define ICM20948_I2C_SLV4_DI     0x17   /* donnée reçue */

/* I2C_MST_STATUS (Bank 0 0x17) */
#define I2C_MST_STATUS_SLV4_DONE  (1 << 6)
#define I2C_MST_STATUS_SLV4_NACK  (1 << 4)

/* SLV4_CTRL */
#define SLV4_EN                   (1 << 7)
#define SLV4_INT_EN               (1 << 6)
#define SLV4_REG_DIS              (1 << 5)  /* pas de sous-adresse registre */

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
 * Nouveau paquet FIFO sans TEMP :
 *  [0 - 5]  Accel XYZ   big-endian    6 octets
 *  [6 -11]  Gyro  XYZ   big-endian    6 octets
 *  [12]     ST1                       1 octet
 *  [13]     MAG_XL  ─┐
 *  [14]     MAG_XH   │ little-endian  2 octets
 *  [15]     MAG_YL   │
 *  [16]     MAG_YH   │                2 octets
 *  [17]     MAG_ZL   │
 *  [18]     MAG_ZH  ─┘                2 octets
 *  [19]     TMPS dummy                1 octet
 *  [20]     ST2                       1 octet
 *  ──────────────────────────────────────────
 *  Total : 21 octets
 */
#define FIFO_PACKET_SIZE       21   /* accel(6)+gyro(6)+ST1(1)+mag(6)+ST2(1) */
#define FIFO_MAX_PACKETS       24   /* 512 / 21           */
#define SPI_BURST_MAX   64   /* taille max buffer burst */

/* ================================================================== */
/* AK09916  (La boussole intégrée)                                    */
/* ================================================================== */
#define AK09916_I2C_ADDR        0x0C
#define AK09916_WIA1            0x00   /* Company ID = 0x48 */
#define AK09916_WIA2            0x01   /* Device ID  = 0x09 */
#define AK09916_WIA2_VAL        0x09
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

/* ── 1. SLV0_LEN = 9 pour inclure ST2 ── */
/*
 * Registres AK09916 lus par SLV0 :
 *  0x10  ST1    ← 1 octet  DRDY + DOR
 *  0x11  HXL   ─┐
 *  0x12  HXH    │ 6 octets  little-endian
 *  0x13  HYL    │
 *  0x14  HYH    │
 *  0x15  HZL    │
 *  0x16  HZH   ─┘
 *  0x17  TMPS  ← 1 octet  dummy (toujours 0)
 *  0x18  ST2   ← 1 octet  HOFL  OBLIGATOIRE pour libérer buffer
 *  ───────────────────────────────
 *  Total : 9 octets
 */
#define AK09916_SLV0_LEN        9    //* ST1+HX(2)+HY(2)+HZ(2)+TMPS(1)+ST2(1) */

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
int icm20948_filter_apply(spi_device_t *spi,
                          const icm20948_filter_preset_t *p);
static int icm20948_set_bank(spi_device_t *spi, uint8_t bank);
static int icm20948_read_reg(spi_device_t *spi, uint8_t reg, uint8_t *val);
static int icm20948_read_burst(spi_device_t *spi, uint8_t reg,
                               uint8_t *buf, size_t len);
static int icm20948_write_reg(spi_device_t *spi, uint8_t reg, uint8_t val);
int icm20948_who_am_i(spi_device_t *spi);
//int icm20948_gyro_config1_init(spi_device_t *spi,
//                                uint8_t fs_sel,
//                                uint8_t dlpf_cfg);
//int icm20948_gyro_smplrt_init(spi_device_t *spi, uint8_t divider);
static int init_reset(spi_device_t *spi);
static int init_power(spi_device_t *spi);
void icm20948_fifo_debug(spi_device_t *spi);
static int init_fifo(spi_device_t *spi, const icm20948_filter_preset_t *cfg);
static int init_compass(spi_device_t *spi);
int icm20948_init(spi_device_t *spi, const icm20948_filter_preset_t *cfg);
static int icm20948_fifo_count(spi_device_t *spi, uint16_t *count);
int icm20948_fifo_read(spi_device_t *spi,
                       icm20948_sample_t *samples,
                       int max_samples,
                       int *nb_read);
void icm20948_convert(const icm20948_sample_t *raw,
                      float *gx, float *gy, float *gz,
                      float *ax, float *ay, float *az,
                      float *compx, float *compy, float *compz,
                      float *cap,
                       float *temp);
static int ak09916_write(spi_device_t *spi, uint8_t reg, uint8_t val);
static int ak09916_read(spi_device_t *spi, uint8_t reg,
                        uint8_t *buf, uint8_t len);
static int init_compass_slv0(spi_device_t *spi);
static int init_compass(spi_device_t *spi);
void icm20948_i2c_mst_debug(spi_device_t *spi);
static int ak09916_slv4_wait_done(spi_device_t *spi);
static int init_i2c_master(spi_device_t *spi);
static int ak09916_check_whoami(spi_device_t *spi);
static int icm20948_fifo_packet_size(spi_device_t *spi);
void icm20948_fifo_sources_debug(spi_device_t *spi);
void icm20948_fifo_dump_raw(spi_device_t *spi);
