/* icm20948.c */
#include "icm20948.h"

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
    if (!spi || spi->fd < 0) {
        fprintf(stderr, "read_burst: fd invalide (%d) reg=0x%02X\n",
                spi ? spi->fd : -999, reg);
        return -1;
    }
    uint8_t tx[SPI_BURST_MAX + 1];
    uint8_t rx[SPI_BURST_MAX + 1];

    if (len > SPI_BURST_MAX) {
        fprintf(stderr, "read_burst: len %zu > max %d\n",
                len, SPI_BURST_MAX);
        return -1;
    }

    memset(tx, 0, len + 1);
    tx[0] = SPI_READ | reg;

    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = (unsigned long)rx,
        .len           = len + 1,
        .speed_hz      = spi->speed_hz,
        .bits_per_word = spi->bits_per_word,
        .delay_usecs   = 0,
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

#if 0
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
#endif

#if 0
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
#endif

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
//                       FIFO_EN2_TEMP_FIFO |
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
uint8_t  h, l;
    uint16_t count;

    *nb_read = 0;
/* Garde-fous */
    if (!spi || spi->fd < 0) {
        fprintf(stderr, "fifo_read: spi invalide\n");
        return -1;
    }
    if (!samples || max_samples <= 0) {
        fprintf(stderr, "fifo_read: paramètres invalides\n");
        return -1;
    }
    icm20948_set_bank(spi, BANK_0);
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTH, &h);
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTL, &l);
    count = ((uint16_t)h << 8) | l;

    if (count == 0)
        return 0;

    /* Désalignement → reset */
    if (count % FIFO_PACKET_SIZE != 0) {
        fprintf(stderr, "FIFO désaligné %u/%d reste %d → reset\n",
                count, FIFO_PACKET_SIZE, count % FIFO_PACKET_SIZE);
        uint8_t val;
        icm20948_read_reg(spi, ICM20948_USER_CTRL, &val);
        icm20948_write_reg(spi, ICM20948_USER_CTRL,
                           val | USER_CTRL_FIFO_RST);
        usleep(1000);
        icm20948_write_reg(spi, ICM20948_USER_CTRL,
                           val & ~USER_CTRL_FIFO_RST);
        return 0;
    }

    int n = count / FIFO_PACKET_SIZE;
    if (n > max_samples) n = max_samples;

    for (int i = 0; i < n; i++) {
        uint8_t buf[FIFO_PACKET_SIZE];   /* taille fixe, pas de VLA */

        if (icm20948_read_burst(spi, ICM20948_FIFO_R_W,
                                buf, FIFO_PACKET_SIZE) < 0)
            return -1;

        /*
         * Layout paquet (21 octets, TEMP désactivé) :
         *  [ 0- 5] Accel XYZ   big-endian
         *  [ 6-11] Gyro  XYZ   big-endian
         *  [12]    ST1
         *  [13]    MAG_XL ─┐
         *  [14]    MAG_XH  │ little-endian
         *  [15]    MAG_YL  │
         *  [16]    MAG_YH  │
         *  [17]    MAG_ZL  │
         *  [18]    MAG_ZH ─┘
         *  [19]    0x17 dummy (réservé AK09916)
         *  [20]    ST2  (bit3=HOFL, bits4-7 réservés=ignorés)
         */
        samples[i].accel_x = (int16_t)((buf[0]  << 8) | buf[1]);
        samples[i].accel_y = (int16_t)((buf[2]  << 8) | buf[3]);
        samples[i].accel_z = (int16_t)((buf[4]  << 8) | buf[5]);

        samples[i].gyro_x  = (int16_t)((buf[6]  << 8) | buf[7]);
        samples[i].gyro_y  = (int16_t)((buf[8]  << 8) | buf[9]);
        samples[i].gyro_z  = (int16_t)((buf[10] << 8) | buf[11]);

        samples[i].mag_st1 = buf[12];
        samples[i].mag_x   = (int16_t)((buf[14] << 8) | buf[13]);
        samples[i].mag_y   = (int16_t)((buf[16] << 8) | buf[15]);
        samples[i].mag_z   = (int16_t)((buf[18] << 8) | buf[17]);
        samples[i].mag_st2 = buf[20];

        /* Valide si DRDY=1 et HOFL=0 (ignorer bits réservés) */
        samples[i].mag_valid = ((samples[i].mag_st1 & 0x01) &&
                                !(samples[i].mag_st2 & 0x08)) ? 1 : 0;
    }

    *nb_read = n;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Conversion en unités physiques                                      */
/* ------------------------------------------------------------------ */
void icm20948_convert(const icm20948_sample_t *s,
                      float *gx, float *gy, float *gz,
                      float *ax, float *ay, float *az,
                      float *mx,  float *my,  float *mz,
                      float *cap,
                       float *temp)
{
 /* Adapter à GYRO_FS_SEL */
    static const float gyro_lsb[] =
        { 131.0f, 65.5f, 32.8f, 16.4f };   /* 250/500/1000/2000 °/s */

    /* Adapter à ACCEL_FS_SEL */
    static const float accel_lsb[] =
        { 16384.0f, 8192.0f, 4096.0f, 2048.0f }; /* 2/4/8/16 g */

    /* Preset BALANCED : GYRO_FS=0x01 (500°/s), ACCEL_FS=0x01 (4g) */
    const float g_scale = 1.0f / gyro_lsb[1];   /* 500 °/s */
    const float a_scale = 1.0f / accel_lsb[1];  /* 4 g      */
    const float m_scale = 0.15f;                 /* µT/LSB   */

    *gx   = s->gyro_x  * g_scale;
    *gy   = s->gyro_y  * g_scale;
    *gz   = s->gyro_z  * g_scale;

    *ax   = s->accel_x * a_scale;
    *ay   = s->accel_y * a_scale;
    *az   = s->accel_z * a_scale;

   if (s->mag_valid) {
        *mx = s->mag_x * m_scale;
        *my = s->mag_y * m_scale;
        *mz = s->mag_z * m_scale;
    } else {
        *mx = *my = *mz = 0.0f;
    }

    *temp =  (s->temp / 333.87f) + 21.0f;  /* °C */ /// 333.87f + 21.0f
    *cap = atan2f(*my, *mx) * (180.0f / M_PI); //M_PI est défini dans math.h
    if (*cap < 0) *cap += 360.0f;
}
/* ------------------------------------------------------------------ */
/* Écriture vers AK09916 via SLV0 (one-shot)                          */
/* ------------------------------------------------------------------ */
static int ak09916_write(spi_device_t *spi, uint8_t reg, uint8_t val)
{
    uint8_t dummy;

    /* Vider I2C_MST_STATUS avant de déclencher */
    icm20948_set_bank(spi, BANK_0);
    icm20948_read_reg(spi, ICM20948_I2C_MST_STATUS, &dummy);

    icm20948_set_bank(spi, BANK_3);
    icm20948_write_reg(spi, ICM20948_I2C_SLV4_ADDR, AK09916_I2C_ADDR); /* write */
    icm20948_write_reg(spi, ICM20948_I2C_SLV4_REG,  reg);
    icm20948_write_reg(spi, ICM20948_I2C_SLV4_DO,   val);
    icm20948_write_reg(spi, ICM20948_I2C_SLV4_CTRL, SLV4_EN);

    return ak09916_slv4_wait_done(spi);
}

/* ------------------------------------------------------------------ */
/* Lecture depuis AK09916 via SLV0 avec retry                         */
/* ------------------------------------------------------------------ */
static int ak09916_read(spi_device_t *spi, uint8_t reg,
                        uint8_t *buf, uint8_t len)
{
    uint8_t dummy;

    for (uint8_t i = 0; i < len; i++) {

        /* Vider I2C_MST_STATUS */
        icm20948_set_bank(spi, BANK_0);
        icm20948_read_reg(spi, ICM20948_I2C_MST_STATUS, &dummy);

        icm20948_set_bank(spi, BANK_3);
        icm20948_write_reg(spi, ICM20948_I2C_SLV4_ADDR,
                           AK09916_I2C_ADDR | 0x80);  /* read */
        icm20948_write_reg(spi, ICM20948_I2C_SLV4_REG,  reg + i);
        icm20948_write_reg(spi, ICM20948_I2C_SLV4_CTRL, SLV4_EN);

        if (ak09916_slv4_wait_done(spi) < 0)
            return -1;

        icm20948_set_bank(spi, BANK_3);
        icm20948_read_reg(spi, ICM20948_I2C_SLV4_DI, &buf[i]);
        icm20948_set_bank(spi, BANK_0);
    }

    return 0;
}

static int init_compass_slv0(spi_device_t *spi)
{
    /* SLV0 : lecture continue ST1(1)+HX(2)+HY(2)+HZ(2)+gap(1)+ST2(1) = 9 octets */
    icm20948_set_bank(spi, BANK_3);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_ADDR,
                       AK09916_I2C_ADDR | 0x80);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_REG,
                       AK09916_ST1);
    icm20948_write_reg(spi, ICM20948_I2C_SLV0_CTRL,
                       (1 << 7) | AK09916_SLV0_LEN);
    icm20948_set_bank(spi, BANK_0);

    printf("[INIT] SLV0 : lecture continue %d octets depuis ST1 ✓\n",
           AK09916_SLV0_LEN);
    return 0;
}

static int init_compass(spi_device_t *spi)
{
    /* ── 1. Init I2C master ── */
    if (init_i2c_master(spi) < 0) return -1;

    /* ── 2. WHO_AM_I sur WIA2 uniquement ── */
    if (ak09916_check_whoami(spi) < 0) {
        icm20948_i2c_mst_debug(spi);
        return -1;
    }

    /* ── 3. Reset soft ── */
    if (ak09916_write(spi, AK09916_CNTL3, 0x01) < 0) return -1;
    usleep(10000);

    /* ── 4. Power down ── */
    if (ak09916_write(spi, AK09916_CNTL2, AK09916_MODE_POWERDOWN) < 0)
        return -1;
    usleep(10000);

    /* ── 5. Mode continu 100 Hz ── */
    if (ak09916_write(spi, AK09916_CNTL2, AK09916_MODE_CONT_100HZ) < 0)
        return -1;
    usleep(10000);

    /* ── 6. Vérifier CNTL2 ── */
    uint8_t mode = 0;
    if (ak09916_read(spi, AK09916_CNTL2, &mode, 1) < 0) return -1;

    if ((mode & 0x1F) != AK09916_MODE_CONT_100HZ) {
        fprintf(stderr, "[INIT] CNTL2 : attendu 0x%02X lu 0x%02X\n",
                AK09916_MODE_CONT_100HZ, mode);
        return -1;
    }
    printf("[INIT] AK09916 CNTL2 = 0x%02X ✓\n", mode);

    /* ── 7. Activer delay ctrl + SLV0 lecture continue ── */
    icm20948_set_bank(spi, BANK_3);
    icm20948_write_reg(spi, ICM20948_I2C_MST_DELAY_CTRL,
                       (1 << 7) | 0x01);
    icm20948_set_bank(spi, BANK_0);

    if (init_compass_slv0(spi) < 0) return -1;

    /* ── 8. Vérification données via EXT_SLV_SENS_DATA ── */
    usleep(50000);

    uint8_t buf[9] = {0};
    icm20948_read_burst(spi, ICM20948_EXT_SLV_SENS_DATA_00, buf, 9);

    int16_t mx = (int16_t)((buf[2] << 8) | buf[1]);
    int16_t my = (int16_t)((buf[4] << 8) | buf[3]);
    int16_t mz = (int16_t)((buf[6] << 8) | buf[5]);

    printf("[INIT] AK09916 raw : ST1=0x%02X  "
           "X=%-6d Y=%-6d Z=%-6d  ST2=0x%02X\n",
           buf[0], mx, my, mz, buf[8]);

    if (mx == 0 && my == 0 && mz == 0) {
        fprintf(stderr, "[INIT] valeurs nulles — SLV0 ne lit pas\n");
        icm20948_i2c_mst_debug(spi);
        return -1;
    }

    printf("[INIT] Compass AK09916 : mode continu 100 Hz ✓\n");
    return 0;
}
void icm20948_i2c_mst_debug(spi_device_t *spi)
{
    uint8_t user_ctrl, mst_ctrl, mst_odr, mst_status;
    uint8_t slv4_addr, slv4_reg, slv4_ctrl, slv4_di;

    icm20948_set_bank(spi, BANK_0);
    icm20948_read_reg(spi, ICM20948_USER_CTRL,      &user_ctrl);
    icm20948_read_reg(spi, ICM20948_I2C_MST_STATUS, &mst_status);

    icm20948_set_bank(spi, BANK_3);
    icm20948_read_reg(spi, ICM20948_I2C_MST_CTRL,   &mst_ctrl);
    icm20948_read_reg(spi, ICM20948_I2C_MST_ODR_CFG,&mst_odr);
    icm20948_read_reg(spi, ICM20948_I2C_SLV4_ADDR,  &slv4_addr);
    icm20948_read_reg(spi, ICM20948_I2C_SLV4_REG,   &slv4_reg);
    icm20948_read_reg(spi, ICM20948_I2C_SLV4_CTRL,  &slv4_ctrl);
    icm20948_read_reg(spi, ICM20948_I2C_SLV4_DI,    &slv4_di);

    icm20948_set_bank(spi, BANK_0);

    printf("=== I2C MASTER DEBUG ===\n");
    printf("USER_CTRL      = 0x%02X  MST_EN=%d IF_DIS=%d\n",
           user_ctrl, (user_ctrl>>5)&1, (user_ctrl>>4)&1);
    printf("I2C_MST_CTRL   = 0x%02X  CLK=%d MULT=%d\n",
           mst_ctrl, mst_ctrl & 0x0F, (mst_ctrl>>4)&1);
    printf("I2C_MST_ODR    = 0x%02X\n", mst_odr);
    printf("I2C_MST_STATUS = 0x%02X  SLV4_DONE=%d SLV4_NACK=%d\n",
           mst_status, (mst_status>>6)&1, (mst_status>>4)&1);
    printf("SLV4_ADDR      = 0x%02X  (att. 0x%02X = read|0x0C)\n",
           slv4_addr, AK09916_I2C_ADDR | 0x80);
    printf("SLV4_REG       = 0x%02X\n", slv4_reg);
    printf("SLV4_CTRL      = 0x%02X  EN=%d\n",
           slv4_ctrl, (slv4_ctrl>>7)&1);
    printf("SLV4_DI        = 0x%02X\n", slv4_di);
    printf("========================\n");
}
static int ak09916_slv4_wait_done(spi_device_t *spi)
{
    uint8_t status;

    /*
     * I2C_MST_STATUS est clear-on-read.
     * À 400kHz, une transaction I2C prend ~50µs.
     * Lire en boucle serrée sans délai initial.
     */
    for (int i = 0; i < 500; i++) {
        usleep(200);  /* 200µs par iteration → 100ms max */

        icm20948_set_bank(spi, BANK_0);
        icm20948_read_reg(spi, ICM20948_I2C_MST_STATUS, &status);

        if (status & (1 << 4)) {   /* SLV4_NACK */
            fprintf(stderr, "ak09916 SLV4 NACK\n");
            return -1;
        }
        if (status & (1 << 6)) {   /* SLV4_DONE */
            return 0;
        }
    }

    fprintf(stderr, "ak09916 SLV4 timeout (100ms)\n");
    icm20948_i2c_mst_debug(spi);
    return -1;
}
static int init_i2c_master(spi_device_t *spi)
{
    uint8_t val;

    /* ── Reset I2C master ── */
    icm20948_set_bank(spi, BANK_0);
    icm20948_read_reg(spi, ICM20948_USER_CTRL, &val);
    val |= USER_CTRL_I2C_IF_DIS | USER_CTRL_I2C_MST_RST;
    val &= ~USER_CTRL_I2C_MST_EN;
    icm20948_write_reg(spi, ICM20948_USER_CTRL, val);
    usleep(10000);

    /* ── Configurer clock avant d'activer ── */
    icm20948_set_bank(spi, BANK_3);

    /*
     * I2C_MST_CTRL :
     *   bits[3:0] = 7 → 345.6 kHz
     *   bit4 = 0      → MULT_MST_EN désactivé (plus simple)
     */
    icm20948_write_reg(spi, ICM20948_I2C_MST_CTRL,    0x07);

    /*
     * I2C_MST_ODR_CFG :
     *   ODR = 1100 / 2^N Hz
     *   0x03 → 137 Hz
     *   0x04 →  68 Hz  ← plus stable pour init
     */
    icm20948_write_reg(spi, ICM20948_I2C_MST_ODR_CFG, 0x04);

    /* Pas de delay ctrl pour l'instant */
    icm20948_write_reg(spi, ICM20948_I2C_MST_DELAY_CTRL, 0x00);

    icm20948_set_bank(spi, BANK_0);
    usleep(1000);

    /* ── Activer I2C master après configuration ── */
    icm20948_read_reg(spi, ICM20948_USER_CTRL, &val);
    val |= USER_CTRL_I2C_MST_EN | USER_CTRL_I2C_IF_DIS;
    icm20948_write_reg(spi, ICM20948_USER_CTRL, val);
    usleep(10000);

    /* ── Vérification ── */
    icm20948_read_reg(spi, ICM20948_USER_CTRL, &val);
    printf("[INIT] USER_CTRL = 0x%02X "
           "(MST_EN=%d IF_DIS=%d)\n",
           val, (val>>5)&1, (val>>4)&1);

    if (!((val & USER_CTRL_I2C_MST_EN) &&
          (val & USER_CTRL_I2C_IF_DIS))) {
        fprintf(stderr, "[INIT] USER_CTRL incorrect !\n");
        return -1;
    }

    return 0;
}
static int ak09916_check_whoami(spi_device_t *spi)
{
    uint8_t wia2 = 0;

    if (ak09916_read(spi, AK09916_WIA2, &wia2, 1) < 0)
        return -1;

    printf("[INIT] AK09916 WIA2=0x%02X (att. 0x%02X)",
           wia2, AK09916_WIA2_VAL);

    if (wia2 != AK09916_WIA2_VAL) {
        printf(" ✗\n");
        return -1;
    }

    printf(" ✓\n");
    return 0;
}
/* Détecter la taille réelle du paquet au runtime */
static int icm20948_fifo_packet_size(spi_device_t *spi)
{
    uint8_t en1, en2;
    int     size = 0;

    icm20948_set_bank(spi, BANK_0);
    icm20948_read_reg(spi, ICM20948_FIFO_EN_1, &en1);
    icm20948_read_reg(spi, ICM20948_FIFO_EN_2, &en2);

    if (en2 & (1 << 0)) size += 2;            /* TEMP    */
    if (en2 & (1 << 4)) size += 6;            /* ACCEL   */
    if (en2 & 0x0E)     size += 6;            /* GYRO    */
    if (en1 & (1 << 0)) size += AK09916_SLV0_LEN; /* SLV0 = 9 */

    return size;
}
void icm20948_fifo_sources_debug(spi_device_t *spi)
{
    uint8_t en1, en2;

    icm20948_set_bank(spi, BANK_0);
    icm20948_read_reg(spi, ICM20948_FIFO_EN_1, &en1);
    icm20948_read_reg(spi, ICM20948_FIFO_EN_2, &en2);

    printf("FIFO_EN_1 = 0x%02X  SLV0=%d\n",
           en1, en1 & 0x01);
    printf("FIFO_EN_2 = 0x%02X  ACCEL=%d ZG=%d YG=%d XG=%d TEMP=%d\n",
           en2,
           (en2>>4)&1, (en2>>3)&1, (en2>>2)&1,
           (en2>>1)&1, (en2>>0)&1);

    /* Calculer la taille réelle du paquet */
    int size = 0;
    if (en2 & (1<<0)) { size += 2; printf("  +2 TEMP\n"); }
    if (en2 & 0x1E)   { size += 6; printf("  +6 Accel\n"); }
    if (en2 & 0x0E)   { size += 6; printf("  +6 Gyro\n"); }
    if (en1 & 0x01)   { size += AK09916_SLV0_LEN;
                        printf("  +%d SLV0 compass\n",
                               AK09916_SLV0_LEN); }
    printf("Taille paquet calculée : %d octets\n", size);
}
void icm20948_fifo_dump_raw(spi_device_t *spi)
{
    uint8_t h, l;

    icm20948_set_bank(spi, BANK_0);
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTH, &h);
    icm20948_read_reg(spi, ICM20948_FIFO_COUNTL, &l);
    uint16_t count = ((uint16_t)h << 8) | l;

    int pkt_size = icm20948_fifo_packet_size(spi);

    printf("FIFO_COUNT = %u octets  pkt_size=%d  "
           "samples=%u\n", count, pkt_size, count / pkt_size);

    if (count < (uint16_t)pkt_size) {
        printf("Pas assez de données\n");
        return;
    }

    /* Lire exactement 1 paquet pour ne pas désaligner */
    uint8_t buf[32];
    icm20948_read_burst(spi, ICM20948_FIFO_R_W, buf, pkt_size);

    /* Décoder selon présence TEMP */
    uint8_t en2;
    icm20948_read_reg(spi, ICM20948_FIFO_EN_2, &en2);
    int has_temp = (en2 & 0x01) ? 1 : 0;
    int base     = has_temp ? 2 : 0;

    printf("FIFO raw [%d octets] :\n", pkt_size);
    for (int i = 0; i < pkt_size; i++) {
        const char *label = "";
        if      (has_temp && i < 2)            label = "TEMP";
        else if (i >= base     && i < base+6)  label = "ACCEL";
        else if (i >= base+6   && i < base+12) label = "GYRO";
        else if (i == base+12)                 label = "ST1";
        else if (i >= base+13  && i < base+19) label = "MAG";
        else if (i == base+19)                 label = "TMPS dummy";
        else if (i == base+20)                 label = "ST2";
        printf("[%2d] 0x%02X  %s\n", i, buf[i], label);
    }

    /* Vérifications rapides */
    uint8_t st1 = buf[base + 12];
    uint8_t st2 = buf[base + 20];
    printf("ST1 = 0x%02X  DRDY=%d  DOR=%d\n",
           st1, st1 & 0x01, (st1 >> 1) & 1);
    printf("ST2 = 0x%02X  HOFL=%d\n",
           st2, (st2 >> 3) & 1);

    if ((st1 >> 1) & 1)
        printf("⚠️  DOR=1 : ST2 pas lu assez vite ou SLV0_LEN trop court\n");

}