/*----------------------------------------------------------------------

                     This file is part of Freetribe

                https://github.com/bangcorrupt/freetribe

                                License

                   GNU AFFERO GENERAL PUBLIC LICENSE
                      Version 3, 19 November 2007

                           AGPL-3.0-or-later

 Freetribe is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
                  (at your option) any later version.

     Freetribe is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty
        of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
          See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.

                       Copyright bangcorrupt 2023

----------------------------------------------------------------------*/

/**
 * @file    per_mmcsd.h
 *
 * @brief   Public API for MMC/SD controller peripheral.
 */

#ifndef PER_MMCSD_H
#define PER_MMCSD_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <am18x_map.h>

/*----- Macros -------------------------------------------------------*/

#define MMCSD_BLOCK_SIZE		0x200
#define TIMEOUT_RSP_MAX			0xFFUL
#define TIMEOUT_DAT_MAX			0x3FFFFFFUL

/*----- Typedefs -----------------------------------------------------*/

typedef struct {
	uint32_t	freq;
	uint32_t	timeout_rsp;
	uint32_t	timeout_dat;
} mmcsd_conf_t;

typedef enum {
	MMCSD_CMD_F_NORSP  = 0,
	MMCSD_CMD_F_RSP    = BIT(1),
	MMCSD_CMD_F_SHORT  = 0,
	MMCSD_CMD_F_LONG   = BIT(2),
	MMCSD_CMD_F_NOCRC  = 0,
	MMCSD_CMD_F_CRC    = BIT(3),
	MMCSD_CMD_F_NODATA = 0,
	MMCSD_CMD_F_DATA   = BIT(4),
	MMCSD_CMD_F_READ   = 0,
	MMCSD_CMD_F_WRITE  = BIT(5),
	MMCSD_CMD_F_NOBUSY = 0,
	MMCSD_CMD_F_BUSY   = BIT(6),
} t_mmcsd_cflags;

typedef struct {
	uint8_t		index;
	uint32_t	cflags;
	uint32_t 	arg;
} t_mmcsd_cmd;

typedef enum {
	MMCSD_SC_NONE,
	MMCSD_SC_RSP_OK,
	MMCSD_SC_CRC_ERR,
	MMCSD_SC_RSP_TOUT,
} t_mmcsd_cmd_state;

typedef enum {
	MMCSD_SD_NONE,
	MMCSD_SD_SENT,
	MMCSD_SD_RECVED,
	MMCSD_SD_CRC_ERR,
	MMCSD_SD_TOUT,
	MMCSD_SD_OK,
	MMCSD_SD_BUSY,
	MMCSD_SD_DONE,
} t_mmcsd_dat_state;

typedef struct {
	uint32_t v[4];
} t_mmcsd_resp;

typedef enum {
	MMCSD_RESP_SHORT,
	MMCSD_RESP_LONG,
} t_mmcsd_resp_type;

typedef enum {
	MMCSD_MISC_F_FIFO_RST = BIT(0),
	MMCSD_MISC_F_FIFO_32B = 0,
	MMCSD_MISC_F_FIFO_64B = BIT(1),
	MMCSD_MISC_F_READ     = 0,
	MMCSD_MISC_F_WRITE    = BIT(2),
	MMCSD_MISC_F_BUSY     = BIT(3),
	MMCSD_MISC_F_BUS4BIT  = BIT(4),
} t_mmcsd_mflags;

typedef struct {
	uint16_t	mflags;
	uint16_t	blkcnt;
} t_mmcsd_misc;

/*----- Extern function prototypes -----------------------------------*/

t_status          mmcsd_con_init(MMCSD_con_t* mcon, const mmcsd_conf_t* conf);
t_status          mmcsd_set_freq(MMCSD_con_t* mcon, uint32_t freq);
t_status          mmcsd_send_cmd(MMCSD_con_t* mcon, const t_mmcsd_cmd* cmd);
t_mmcsd_cmd_state mmcsd_cmd_state(const MMCSD_con_t* mcon, am18x_bool need_crc);
t_mmcsd_dat_state mmcsd_busy_state(const MMCSD_con_t* mcon);
t_mmcsd_dat_state mmcsd_rd_state(const MMCSD_con_t* mcon);
t_mmcsd_dat_state mmcsd_wr_state(const MMCSD_con_t* mcon);
t_status          mmcsd_get_resp(const MMCSD_con_t* mcon, t_mmcsd_resp_type type, t_mmcsd_resp* resp);
t_status          mmcsd_cntl_misc(MMCSD_con_t* mcon, const t_mmcsd_misc* misc);
uint32_t          mmcsd_read(const MMCSD_con_t* mcon);
t_status          mmcsd_write(MMCSD_con_t* mcon, uint32_t data);


#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
