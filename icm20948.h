/* icm20948.h */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

/* Bank 2 */
#define ICM20948_GYRO_SMPLRT_DIV   0x00
#define ICM20948_GYRO_CONFIG_1     0x01
#define ICM20948_ACCEL_SMPLRT_DIV1 0x10
#define ICM20948_ACCEL_SMPLRT_DIV2 0x11
#define ICM20948_ACCEL_CONFIG      0x14

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
#define FIFO_PACKET_SIZE       14   /* accel(6) + gyro(6) + temp*/
#define FIFO_MAX_PACKETS       36   /* 512 / 14           */


/* ── Structure d'un sample ───────────────────────────────────────── */
typedef struct {
    int16_t accel_x, accel_y, accel_z;
    int16_t gyro_x,  gyro_y,  gyro_z;
    int16_t temp;
} icm20948_sample_t;

typedef struct {
    int fd;           /* descripteur /dev/spidevX.Y */
} ICM20948;

typedef struct {
    /* Gyro */
    uint8_t  gyro_fs;          /* GYRO_FS_xxx      */
    uint8_t  gyro_dlpf;        /* GYRO_DLPF_xxxHZ  */
    uint8_t  gyro_smplrt_div;  /* 0x00 à 0xFF      */

    /* Accel */
    uint8_t  accel_fs;         /* ACCEL_FS_xG      */
    uint8_t  accel_dlpf;       /* ACCEL_DLPF_xxxHZ */
    uint16_t accel_smplrt_div; /* 0x000 à 0xFFF    */

    /* FIFO */
    uint8_t  fifo_mode;        /* 0=stream, 1=stop-on-full */
} icm20948_config_t;

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

    usleep(30000);  /* 30 ms — stabilisation */

    printf("[INIT] Power OK (accel + gyro actifs)\n");
    return 0;
}

/* ── 4. Configuration gyroscope ──────────────────────────────────── */
static int init_gyro(spi_device_t *spi, const icm20948_config_t *cfg)
{
    uint8_t val;

    icm20948_set_bank(spi, BANK_2);

    /* GYRO_CONFIG_1 : FCHOICE + FS + DLPF */
    val = GYRO_FCHOICE_ENABLE | cfg->gyro_fs | cfg->gyro_dlpf;
    if (icm20948_write_reg(spi, ICM20948_GYRO_CONFIG_1, val) < 0)
        return -1;

    /* GYRO_SMPLRT_DIV */
    if (icm20948_write_reg(spi, ICM20948_GYRO_SMPLRT_DIV,
                           cfg->gyro_smplrt_div) < 0)
        return -1;

    icm20948_set_bank(spi, BANK_0);

    printf("[INIT] Gyro : FS=0x%02X  DLPF=0x%02X  "
           "ODR=%.1f Hz\n",
           cfg->gyro_fs, cfg->gyro_dlpf,
           1125.0f / (1 + cfg->gyro_smplrt_div));
    return 0;
}

/* ── 5. Configuration accéléromètre ──────────────────────────────── */
static int init_accel(spi_device_t *spi, const icm20948_config_t *cfg)
{
    uint8_t val;

    icm20948_set_bank(spi, BANK_2);

    /* ACCEL_CONFIG : FCHOICE + FS + DLPF */
    val = ACCEL_FCHOICE_ENABLE | cfg->accel_fs | cfg->accel_dlpf;
    if (icm20948_write_reg(spi, ICM20948_ACCEL_CONFIG, val) < 0)
        return -1;

    /* ACCEL_SMPLRT_DIV sur 12 bits → 2 registres */
    uint8_t div_h = (cfg->accel_smplrt_div >> 8) & 0x0F;
    uint8_t div_l =  cfg->accel_smplrt_div & 0xFF;

    if (icm20948_write_reg(spi, ICM20948_ACCEL_SMPLRT_DIV1, div_h) < 0)
        return -1;
    if (icm20948_write_reg(spi, ICM20948_ACCEL_SMPLRT_DIV2, div_l) < 0)
        return -1;

    icm20948_set_bank(spi, BANK_0);

    printf("[INIT] Accel : FS=0x%02X  DLPF=0x%02X  "
           "ODR=%.1f Hz\n",
           cfg->accel_fs, cfg->accel_dlpf,
           1125.0f / (1 + cfg->accel_smplrt_div));
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
static int init_fifo(spi_device_t *spi, const icm20948_config_t *cfg)
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
        fprintf(stderr, "FIFO_RST non relâché\n");
        return -1;
    }

    /* ── Étape 4 : mode FIFO ── */
    icm20948_write_reg(spi, ICM20948_FIFO_MODE,
                       cfg->fifo_mode ? 0x1F : 0x00);

    /* ── Étape 5 : activer FIFO dans USER_CTRL ── */
    icm20948_write_reg(spi, ICM20948_USER_CTRL, USER_CTRL_FIFO_EN);
    usleep(1000);

    /* ── Étape 6 : connecter les sources ── */
    icm20948_write_reg(spi, ICM20948_FIFO_EN_1, 0x00);
    icm20948_write_reg(spi, ICM20948_FIFO_EN_2,
                        FIFO_EN2_TEMP_FIFO |
                       FIFO_EN2_GYRO_X |
                       FIFO_EN2_GYRO_Y |
                       FIFO_EN2_GYRO_Z |
                       FIFO_EN2_ACCEL);

    /* ── Étape 7 : attendre au moins 1 sample ── */
    usleep(20000);  /* 20 ms > 1 période à 100 Hz */

    /* ── Vérification ── */
    uint8_t h, l;
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTH, &h);
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTL, &l);
    uint16_t count = ((uint16_t)h << 8) | l;

    if (count == 0) {
        fprintf(stderr, "FIFO toujours vide après init !\n");
        icm20948_fifo_debug(spi);
        return -1;
    }

    printf("[INIT] FIFO OK : %u octets (%u samples) disponibles\n",
           count, count / FIFO_PACKET_SIZE);
    return 0;
}

int icm20948_init(spi_device_t *spi, const icm20948_config_t *cfg)
{
    //On se positionne sur la bank 0
    icm20948_set_bank(spi, BANK_0);

       if (init_reset(spi)         < 0) return -1;
//    if (init_check_whoami(spi)  < 0) return -1;  /* re-vérif post-reset */
    if (init_power(spi)         < 0) return -1;
    if (init_gyro(spi, cfg)     < 0) return -1;
    if (init_accel(spi, cfg)    < 0) return -1;
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
    uint16_t count  = 0;
    int      n      = 0;

    *nb_read = 0;

    icm20948_set_bank(spi, BANK_0);

    /* Nombre d'octets dans le FIFO */
    if (icm20948_fifo_count(spi, &count) < 0)
        return -1;

    if (count == 0 || count < FIFO_PACKET_SIZE) {
        return 0;   /* Pas encore assez de données */
    }

    /* Nombre de samples complets */
    n = count / FIFO_PACKET_SIZE;
    if (n > max_samples)
        n = max_samples;

    /* Lecture burst du FIFO */
    for (int i = 0; i < n; i++) {
        uint8_t buf[FIFO_PACKET_SIZE];

        if (icm20948_read_burst(spi, ICM20948_FIFO_R_W,
                                buf, FIFO_PACKET_SIZE) < 0)
            return -1;

        /* Reconstruction int16 big-endian */
        samples[i].accel_x = (int16_t)((buf[0]  << 8) | buf[1]);
        samples[i].accel_y = (int16_t)((buf[2]  << 8) | buf[3]);
        samples[i].accel_z = (int16_t)((buf[4]  << 8) | buf[5]);
        samples[i].gyro_x  = (int16_t)((buf[6]  << 8) | buf[7]);
        samples[i].gyro_y  = (int16_t)((buf[8]  << 8) | buf[9]);
        samples[i].gyro_z  = (int16_t)((buf[10] << 8) | buf[11]);
        samples[i].temp    = (int16_t)((buf[12] << 8) | buf[13]);
    }

    *nb_read = n;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Conversion en unités physiques                                      */
/* ------------------------------------------------------------------ */
void icm20948_convert(const icm20948_sample_t *raw,
                      float *gx, float *gy, float *gz,
                      float *ax, float *ay, float *az, float *temp)
{
    /* Sensibilité gyro pour ±500°/s */
    const float gyro_scale  = 1.0f / 65.5f;   /* °/s par LSB */

    /* Sensibilité accel pour ±4g */
    const float accel_scale = 1.0f / 8192.0f;  /* g par LSB  */

    *gx = raw->gyro_x  * gyro_scale;
    *gy = raw->gyro_y  * gyro_scale;
    *gz = raw->gyro_z  * gyro_scale;

    *ax = raw->accel_x * accel_scale;
    *ay = raw->accel_y * accel_scale;
    *az = raw->accel_z * accel_scale;
    *temp =  (raw->temp / 333.87f) + 21.0f;  /* °C */ /// 333.87f + 21.0f
}
