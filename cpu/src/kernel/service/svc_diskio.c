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
 * @file    svc_io.c.
 *
 * @brief   Disk IO service layer.
 */

/*----- Includes -----------------------------------------------------*/

#include <ff.h>
#include <diskio.h>

#include "macros.h"

#include "dev_sdcard.h"
#include "svc_diskio.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

FATFS g_fatfs;
PARTITION VolToPart[FF_VOLUMES] = { { 0, 0 } };

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialize Disk Drive.
 * 
 * @param   pdrv   Physical drive number (0)
 */
DSTATUS disk_initialize(BYTE pdrv) {
    DSTATUS stat;
    int result;

    if (SDCARD_OK != dev_sdcard_init()) {
        DEBUG_LOG("disk_initialize() dev_sdcard_init() failed");
        return STA_NODISK;
    }
    
    return 0;
}

/**
 * @brief   Returns the current status of a drive.
 * 
 * @param   drv   Physical drive number (0)
 */
DSTATUS disk_status(BYTE drv) {
    return 0;
}

/**
 * @brief   Read sector(s) from the disk drive.
 * 
 * @param   drv     Physical drive number (0)
 * @param   buff    Pointer to the data buffer to store read data
 * @param   sector  Start sector in LBA
 * @param   count   Number of sectors to read
 */
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
	
    DEBUG_LOG("disk_read(%i, %p, %i, %i)", (int)pdrv, (void*)buff, (int)sector, (int)count);

    uint32_t *buf_u32 = (uint32_t*)buff; // be aware of this
    t_sdcard_status st = dev_sdcard_read(sector, count, buf_u32);

    if (SDCARD_OK != st) {
        DEBUG_LOG("disk_read->dev_sdcard_read error: %i", (int)st);
        return RES_ERROR;
    }

	return RES_OK;
}

/**
 * @brief   Write sector(s) to the disk drive.
 * 
 * @param   pdrv    Physical drive number (0)
 * @param   buff    Data to be written
 * @param   sector  Start sector in LBA
 * @param   count   Number of sectors to write
 */
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {

    DEBUG_LOG("disk_write(%i, %p, %i, %i)", (int)pdrv, (void*)buff, (int)sector, (int)count);

    const uint32_t *buf_u32 = (const uint32_t*)buff; // be aware of this
    t_sdcard_status st = dev_sdcard_write(sector, count, buf_u32);

    if (SDCARD_OK != st) {
        DEBUG_LOG("disk_write->dev_sdcard_write error: %i", (int)st);
    }

    return RES_OK;
}

/**
 * @brief   Control interface between SD card driver and FatFS. FatFS
 *          calls this function to inform itself about the SD card driver.
 * 
 * @param   pdrv    Physical drive number (0)
 * @param   cmd     Control code
 * @param   buff    Buffer to send/receive control data
 */
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    return 0;
}

/**
 * @brief   Real time clock service to be called from FatFS module.
 *          Any valid time must be returned even if the system does
 *          not support a real time clock.
 */
DWORD get_fattime(void) {
    // @TODO: implement RTC
    return   ((2007UL-1980) << 25)  // Year 2007
            | (6UL << 21)           // Month June
            | (5UL << 16)           // Day 5
            | (11U << 11)           // Hour 11
            | (38U << 5)            // Min 38
            | (0U >> 1);            // Sec 0
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
