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
 *  @file   dev_flash.c
 *
 *  @brief  Flash memory device driver.
 */

/// TODO: Rework to use new SPI driver.

/// TODO: Flash access is very slow.
//          Possible issue with SPI clock.
//              Check if fixed now cache enabled.

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ft_error.h"
#include "soc_AM1808.h" // Only needed for SPI1_BASE.

#include "per_spi.h"

#include "dev_flash.h"

/// TODO: Use device layer delay, or SPI peripheral timing.
#include "svc_delay.h"

/*----- Macros -------------------------------------------------------*/

#define SPI_FLASH_BASE SPI1_BASE
#define SPI_FLASH_CS 0x1

/// TODO: typedef enum {} t_flash_command;
#define WRITE_IN_PROGRESS 0x01
#define WRITE_ENABLE_LATCH 0x02
#define PROGRAM_FAIL 0x20
#define ERASE_FAIL 0x40

#define FLASH_PAGE_PROGRAM 0x02
#define FLASH_READ 0x03
#define FLASH_WRITE_DISABLE 0x04
#define FLASH_WRITE_ENABLE 0x06
#define FLASH_SECTOR_ERASE 0x20
#define FLASH_BLOCK_32_ERASE 0x52
#define FLASH_BLOCK_64_ERASE 0xd8

#define FLASH_READ_STATUS 0x05
#define FLASH_READ_CONFIG 0x15
#define FLASH_READ_SECURITY 0x2b
#define FLASH_READ_LOCK 0x2d
#define FLASH_READ_SPB_LOCK 0xa7
#define FLASH_READ_SPB 0xe2
#define FLASH_ERASE_SPB 0xe4
#define FLASH_READ_DPB 0xe0
#define FLASH_GANG_BLOCK_UNLOCK 0x98

#define PAGE_LENGTH 0x100
#define SECTOR_LENGTH 0x1000
#define PAGES_PER_SECTOR 0x10

#define SECTOR_OFFSET_MASK 0xfff
#define SECTOR_INDEX_SHIFT 0xc

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static uint8_t g_sector_buffer[SECTOR_LENGTH] = {0};

/*----- Static function prototypes -----------------------------------*/

void _flash_command(uint8_t cmd);
void _flash_address(uint32_t address);
void _flash_address_32bit(uint32_t address);
void _flash_tx(uint8_t *p_tx, uint32_t len);
void _flash_rx(uint8_t *p_rx, uint32_t len);
void _sector_erase(uint32_t sector_addr);
void _sector_write(uint32_t dest, uint8_t *p_src);
void _page_program(uint32_t dest, uint8_t *p_src);
bool _write_enable(void);
uint8_t _read_status(void);
uint8_t _read_security(void);
uint16_t _read_lock(void);
uint8_t _read_spb_lock(void);
bool _read_spb(uint32_t addr);
bool _read_dpb(uint32_t addr);
void _erase_spb(void);
void _write_disable(void);
void _gang_block_unlock(void);
void _flash_chip_select(bool state);
bool _flash_busy(void);

/*----- Extern variable definitions ----------------------------------*/

/*----- Extern function implementations ------------------------------*/

t_status dev_flash_init(void) {

    // per_spi1_init(); // Flash

    return SUCCESS;
}

void flash_read(uint32_t src, uint8_t *p_dest, uint32_t len) {

    // Wait for any write operations to complete.
    while (_flash_busy())
        ;

    // Assert chip select.
    _flash_chip_select(true);

    // Send read command.
    _flash_command(FLASH_READ);

    // Send source address.
    _flash_address(src);

    // Receive data.
    _flash_rx(p_dest, len);

    // Release chip select.
    _flash_chip_select(false);
}

void flash_write(uint32_t dest, uint8_t *p_src, uint32_t len) {

    uint16_t sector_offset = dest & SECTOR_OFFSET_MASK;

    // If destination address not sector aligned.
    if (sector_offset) {

        // Start address of target sector.
        dest -= sector_offset;

        // Number of bytes to copy.
        uint16_t copy_length = SECTOR_LENGTH - sector_offset;

        // Read target sector.
        flash_read(dest, g_sector_buffer, SECTOR_LENGTH);

        // Copy source data to sector buffer.
        memcpy(g_sector_buffer + sector_offset, p_src, copy_length);

        _sector_erase(dest);

        _sector_write(dest, g_sector_buffer);

        p_src += copy_length;
        dest += SECTOR_LENGTH;
        len -= copy_length;

        // Handle underflow.
        if ((int32_t)len < 0) {
            len = 0;
        }
    }

    // Write complete sectors.
    while (len >> SECTOR_INDEX_SHIFT) {

        _sector_erase(dest);

        _sector_write(dest, p_src);

        p_src += SECTOR_LENGTH;
        dest += SECTOR_LENGTH;
        len -= SECTOR_LENGTH;
    }

    // If partial final sector.
    if (len) {

        // Read target sector.
        flash_read(dest, g_sector_buffer, SECTOR_LENGTH);

        // Copy source data to sector buffer.
        memcpy(g_sector_buffer, p_src, len);

        _sector_erase(dest);

        _sector_write(dest, g_sector_buffer);
    }
}

bool flash_verify(uint32_t flash_addr, uint8_t *p_ram_data, uint32_t len) {

    bool verified = true;
    uint16_t i;

    while (len > SECTOR_LENGTH) {

        // Read data from flash (does not need to be sector aligned).
        flash_read(flash_addr, g_sector_buffer, SECTOR_LENGTH);

        for (i = 0; i < SECTOR_LENGTH; i++) {

            if (g_sector_buffer[i] != p_ram_data[i]) {
                verified = false;
            }
        }

        p_ram_data += SECTOR_LENGTH;
        flash_addr += SECTOR_LENGTH;
        len -= SECTOR_LENGTH;
    }

    flash_read(flash_addr, g_sector_buffer, len);

    for (i = 0; i < len; i++) {

        if (g_sector_buffer[i] != p_ram_data[i]) {
            verified = false;
        }
    }

    return verified;
}

void flash_erase(uint32_t address, uint32_t len) {

    uint16_t sector_offset = address & SECTOR_OFFSET_MASK;

    // If destination address not sector aligned.
    if (sector_offset) {

        // Start address of target sector.
        address -= sector_offset;

        // Number of bytes to set.
        uint16_t set_length = SECTOR_LENGTH - sector_offset;

        // Read target sector.
        flash_read(address, g_sector_buffer, SECTOR_LENGTH);

        memset(g_sector_buffer + sector_offset, 0xff, set_length);

        _sector_erase(address);

        _sector_write(address, g_sector_buffer);

        address += SECTOR_LENGTH;
        len -= set_length;
    }

    while (address >> SECTOR_INDEX_SHIFT) {

        _sector_erase(address);

        address += SECTOR_LENGTH;
        len -= SECTOR_LENGTH;
    }

    if (len) {

        flash_read(address, g_sector_buffer, SECTOR_LENGTH);

        memset(g_sector_buffer, 0xff, len);

        _sector_erase(address);

        _sector_write(address, g_sector_buffer);
    }
}

void flash_unlock(void) { _gang_block_unlock(); }

/*----- Static function implementations ------------------------------*/

void _sector_write(uint32_t dest, uint8_t *p_src) {

    uint8_t i;

    for (i = 0; i < PAGES_PER_SECTOR; i++) {

        _page_program(dest, p_src);

        dest += PAGE_LENGTH;
        p_src += PAGE_LENGTH;
    }
}

void _sector_erase(uint32_t sector_addr) {

    while (!_write_enable())
        ;

    _flash_chip_select(true);

    _flash_command(FLASH_SECTOR_ERASE);

    _flash_address(sector_addr);

    _flash_chip_select(false);

    while (_flash_busy())
        ;

    while (_read_status() & WRITE_ENABLE_LATCH)
        ;

    if (_read_security() & ERASE_FAIL) {
        /// TODO: Handle error.
    }
}

void _page_program(uint32_t dest, uint8_t *p_src) {

    while (!_write_enable())
        ;

    _flash_chip_select(true);

    _flash_command(FLASH_PAGE_PROGRAM);

    _flash_address(dest);

    _flash_tx(p_src, PAGE_LENGTH);

    _flash_chip_select(false);

    while (_flash_busy())
        ;

    while (_read_status() & WRITE_ENABLE_LATCH)
        ;

    if (_read_security() & PROGRAM_FAIL) {
        /// TODO: Handle error.
    }
}

bool _flash_busy(void) { return _read_status() & WRITE_IN_PROGRESS; }

bool _write_enable(void) {

    _flash_chip_select(true);
    _flash_command(FLASH_WRITE_ENABLE);
    _flash_chip_select(false);

    return _read_status() & WRITE_ENABLE_LATCH;
}

uint8_t _read_reg_byte(uint8_t cmd) {

    uint8_t flash_reg = 0;

    _flash_chip_select(true);
    _flash_command(cmd);

    _flash_rx(&flash_reg, 1);
    _flash_chip_select(false);

    return flash_reg;
}

uint16_t _read_reg_short(uint8_t cmd) {

    uint8_t flash_reg[2] = {0};

    _flash_chip_select(true);
    _flash_command(cmd);

    _flash_rx(&flash_reg[0], 2);
    _flash_chip_select(false);

    return (flash_reg[0] << 8) | flash_reg[1];
}

uint8_t _read_status(void) { return _read_reg_byte(FLASH_READ_STATUS); }

uint8_t _read_config(void) { return _read_reg_byte(FLASH_READ_CONFIG); }

uint8_t _read_security(void) { return _read_reg_byte(FLASH_READ_SECURITY); }

uint16_t _read_lock(void) { return _read_reg_short(FLASH_READ_LOCK); }

uint8_t _read_spb_lock(void) { return _read_reg_byte(FLASH_READ_SPB_LOCK); }

bool _read_spb(uint32_t addr) {

    uint8_t spb = 0;

    _flash_chip_select(true);
    _flash_command(FLASH_READ_SPB);
    _flash_address_32bit(addr);

    _flash_rx(&spb, 1);
    _flash_chip_select(false);

    return (bool)spb;
}

bool _read_dpb(uint32_t addr) {

    uint8_t dpb = 0;

    _flash_chip_select(true);
    _flash_command(FLASH_READ_DPB);
    _flash_address_32bit(addr);

    _flash_rx(&dpb, 1);
    _flash_chip_select(false);

    return (bool)dpb;
}

void _erase_spb(void) {

    while (!_write_enable())
        ;

    _flash_chip_select(true);

    _flash_command(FLASH_ERASE_SPB);

    _flash_chip_select(false);

    while (_flash_busy())
        ;

    while (_read_status() & WRITE_ENABLE_LATCH)
        ;

    if (_read_security() & ERASE_FAIL) {
        /// TODO: Handle error.
    }

    if (_read_security() & PROGRAM_FAIL) {
        /// TODO: Handle error.
    }
}

void _gang_block_unlock(void) {

    while (!_write_enable())
        ;

    _flash_chip_select(true);

    _flash_command(FLASH_GANG_BLOCK_UNLOCK);
    _flash_chip_select(false);

    while (_flash_busy())
        ;

    while (_read_status() & WRITE_ENABLE_LATCH)
        ;
}

void _write_disable(void) {

    _flash_chip_select(true);

    _flash_command(FLASH_WRITE_DISABLE);

    _flash_chip_select(false);

    while (_read_status() & WRITE_ENABLE_LATCH)
        ;
}

/// TODO: SPI chip select delay.
void _flash_chip_select(bool state) {

    if (!state) {
        delay_cycles(4);
    }

    /// TODO: Use per_spi_chip_format.
    //
    // per_spi_chip_select(SPI_FLASH_BASE, SPI_FLASH_CS, state);

    if (state) {
        delay_cycles(4);
    }
}

void _flash_command(uint8_t cmd) { _flash_tx(&cmd, 1); }

/// TODO: Maybe typedef flash_address.
void _flash_address(uint32_t address) {

    // Source address array.
    uint8_t addr[3];

    // 24 bit flash address as 3 bytes.
    addr[0] = (uint8_t)(address >> 16);
    addr[1] = (uint8_t)(address >> 8);
    addr[2] = (uint8_t)address;

    _flash_tx(addr, 3);
}

void _flash_address_32bit(uint32_t address) {

    // Source address array.
    uint8_t addr[4];

    // 32 bit flash address as 4 bytes.
    addr[0] = (uint8_t)(address >> 24);
    addr[1] = (uint8_t)(address >> 16);
    addr[2] = (uint8_t)(address >> 8);
    addr[3] = (uint8_t)address;

    _flash_tx(addr, 4);
}

void _flash_tx(uint8_t *p_tx, uint32_t len) {

    /// TODO: SPI_1 needs mutex to prevent DSP and flash access collision.
    //
    // per_spi_tx(SPI_FLASH_BASE, p_tx, len);
}

void _flash_rx(uint8_t *p_rx, uint32_t len) {

    /// TODO: SPI_1 needs mutex to prevent DSP and flash access collision.
    //
    // per_spi_rx(SPI_FLASH_BASE, p_rx, len);
}
