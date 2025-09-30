/*----------------------------------------------------------------------
MIT License

Copyright (c) 2013 - turmary@126.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


Original work by turmary@126.com, modified by novictim 2025 for freetribe.
----------------------------------------------------------------------*/

/**
 * @file    sd_protocol.h
 *
 * @brief   MMC/SD protocol specific code.
 */

#ifndef SD_PROTOCOL_H
#define SD_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

// Physical Layer Simplified Specification Version 2.0

#define MMC_MAX_CLK              20000000 //Hz
#define SD_MAX_CLK               25000000 //Hz

// 4.3.13
#define CMD8_VHS_MASK            0x00000F00
#define CMD8_VHS_27to36          0x00000100
#define CMD8_CHECK_PATTERN_MASK  0x000000FF
#define CMD8_CHECK_PATTERN       0x000000AA

// 4.7.4 Detailed Command Description
typedef enum {
    SDCARD_NONE_RSP,
    SDCARD_48BIT_RSP,
    SDCARD_136BIT_RSP,
} sdcard_response_t;

// Basic Commands (class 0)
#define CMD0_GO_IDLE_STATE            0
#define CMD1R3_SEND_OP_COND           1
#define CMD2R2_ALL_SEND_CID           2
#define CMD3R6_SEND_RELATIVE          3
#define CMD4_SEND_DSR                 4
#define CMD7R1b_SEL_UNSEL_CARD        7
#define CMD8R7_SEND_IF_COND           8
#define CMD9R2_SEND_CSD               9
#define CMD10R2_SEND_CID              10
#define CMD12R1b_STOP_TRANSMISSION    12
#define CMD13R1_SEND_STATUS           13
#define CMD15_GO_INACTIVE_STATE       15
// Block-Oriented Read Commands (class 2)
#define CMD16R1_SET_BLOCKLEN          16
#define CMD17R1_READ_SINGLE_BLOCK     17
#define CMD18R1_READ_MULTIPLE_BLOCK   18
// Block-Oriented Write Commands (class 4)
#define CMD24R1_WRITE_BLOCK           24
#define CMD25R1_WRITE_MULTIPLE_BLOCK  25
#define CMD27R1_PROGRAM_CSD           27
// Block-Oriented Write Protection Commands (class 6)
#define CMD28R1b_SET_WRITE_PROT       28
#define CMD29R1b_CLR_WRITE_PROT       29
#define CMD30R1_SEND_WRITE_PROT       30
// Erase Commands (class 5)
#define CMD32R1_ERASE_WR_BLK_START    32
#define CMD33R1_ERASE_WR_BLK_END      33
#define CMD38R1b_ERASE                38
// Lock Card (class 7)
#define CMD42R1_LOCK_UNLOCK           42
// Application-specific Commands (class 8)
#define CMD55R1_APP_CMD               55
#define CMD56R1_GEN_CMD               56

// Application Specific Commands used/reserved by SD Memory Card
#define ACMD6R1_SET_BUS_WIDTH            6
#define ACMD6_BW_1BIT                 0x00
#define ACMD6_BW_4BIT                 0x02
#define ACMD13R1_SD_STATUS              13
#define ACMD22R1_SEND_NUM_WR_BLOCKS     22
#define ACMD23R1_SET_WR_BLK_ERASE_COUNT 23
#define ACMD41R3_SD_SEND_OP_COND        41
#define ACMD42R1_SET_CLR_CARD_DETECT    42
#define ACMD51R1_SEND_SCR               51

// 4.9 Responses
#define R1_CodeLength           48
#define R1b_CodeLength          48
// CID, CSD Register
#define R2_CodeLength           136
// OCR Register
#define R3_CodeLength           48
// Published RCA response
#define R6_CodeLength           48
// Card interface condition
#define R7_CodeLength           48

// 4.10 Two Status Information of SD Memory Card
// 4.10.1 Card Status
// The response format R1 contains a 32-bit named card status.
typedef enum {
    SDP_IDLE = 0,
    SDP_READY,
    SDP_IDENT,
    SDP_STBY,
    SDP_TRAN,
    SDP_DATA,
    SDP_RCV,
    SDP_PRG,
    SDP_DIS,
    SDP_CNT,
    SDP_INA = 16,
    SDP_INV = 255,
} sdp_cur_stat_t;

typedef struct {
    uint8_t RES_TEST_MODE       :2;
    uint8_t RES_APP_CMD         :1;
    uint8_t AKE_SEQ_ERROR       :1;
    uint8_t RES_SDIO_CARD       :1;
    uint8_t APP_CMD             :1;
    uint8_t RES1                :2;
    uint8_t READY_FOR_DATA      :1;
    uint8_t CURRENT_STATE       :4;
    uint8_t ERASE_RESET         :1;
    uint8_t CARD_ECC_DISABLED   :1;
    uint8_t WP_ERASE_SKIP       :1;
    uint8_t CSD_OVERWRITE       :1;
    uint8_t RES2                :2;
    uint8_t ERROR               :1;
    uint8_t CC_ERROR            :1;
    uint8_t CARD_ECC_FAILED     :1;
    uint8_t ILLEGAL_COMMAND     :1;
    uint8_t COM_CRC_ERROR       :1;
    uint8_t CARD_UNLOCK_FAILED  :1;
    uint8_t CARD_IS_LOCKED      :1;
    uint8_t WP_VIOLATION        :1;
    uint8_t ERASE_PARAM         :1;
    uint8_t ERASE_SEQ_ERROR     :1;
    uint8_t BLOCK_LEN_ERROR     :1;
    uint8_t ADDRESS_ERROR       :1;
    uint8_t OUT_OF_RANGE        :1;
} sdp_r1_stat_t;


#define CStatus_AKE_SEQ_ERROR       BIT(3)
#define CStatus_APP_CMD             BIT(5)
#define CStatus_READY_FOR_DATA      BIT(8)
#define CStatus_STAT_MASK           (0xF << 9)
#define CStatus_STAT_idle           (SDP_STAT_IDLE << 9)
#define CStatus_STAT_ready          (SDP_STAT_READY << 9)
#define CStatus_STAT_ident          (SDP_STAT_IDENT << 9)
#define CStatus_STAT_stby           (SDP_STAT_STBY << 9)
#define CStatus_STAT_tran           (SDP_STAT_TRAN << 9)
#define CStatus_STAT_data           (SDP_STAT_DATA << 9)
#define CStatus_STAT_rcv            (SDP_STAT_RCV << 9)
#define CStatus_STAT_prg            (SDP_STAT_PRG << 9)
#define CStatus_STAT_dis            (SDP_STAT_DIS << 9)
#define CStatus_ERASE_RESET         BIT(13)
#define CStatus_CARD_ECC_DISABLED   BIT(14)
#define CStatus_WP_ERASE_SKIP       BIT(15)
#define CStatus_CSD_OVERWRITE       BIT(16)
#define CStatus_ERROR               BIT(19)
#define CStatus_CC_ERROR            BIT(20)
#define CStatus_CARD_ECC_FAILED     BIT(21)
#define CStatus_ILLEGAL_COMMAND     BIT(22)
#define CStatus_COM_CRC_ERROR       BIT(23)
#define CStatus_CARD_UNLOCK_FAILED  BIT(24)
#define CStatus_CARD_IS_LOCKED      BIT(25)
#define CStatus_WP_VIOLATION        BIT(26)
#define CStatus_ERASE_PARAM         BIT(27)
#define CStatus_ERASE_SEQ_ERROR     BIT(28)
#define CStatus_BLOCK_LEN_ERROR     BIT(29)
#define CStatus_ADDRESS_ERROR       BIT(30)
#define CStatus_OUT_OF_RANGE        BIT(31)
// 4.10.2 SD Status
// The SD Status is sent to the host over the DAT bus
// as a response to ACMD13, have a length 512 bit.

// 5.1 OCR Register
#define OCR_VOLTAGE_WINDOW(MV)      BIT(MV / 100 - 12)
#define IS_VALID_OCR_VOLTAGE(MV)    (2700 <= MV && MV <= 3600)

#define OCR_VOLTAGE_MASK 0x00FF8000
#define OCR_CCS          0x40000000
#define OCR_PowerUpEnd   0x80000000
#define MOCR_VOLTAGE_165to195       0x80

// 5.2 CID register
typedef struct {        // width, offset
    uint8_t     MID;    // 8,  120
    uint16_t    OID;    // 16, 104
    uint8_t     PNM[6]; // 40, 64
    uint8_t     PRV;    // 8,  56
    uint32_t    PSN;    // 32, 24
    uint16_t    MDT;    // 12, 8
    uint8_t     CRC;    // 7,  1
} CID_t;

// 5.3.2 CSD Register
typedef struct {
    uint8_t     CSD_STRUCTURE;
    uint8_t     TAAC;
    uint8_t     NSAC;
    uint8_t     TRANS_SPEED;
    uint16_t    CCC;
    uint8_t     READ_BL_LEN;
    uint8_t     READ_BL_PARTIAL;
    uint8_t     WRITE_BLK_MISALIGN;
    uint8_t     READ_BLK_MISALIGN;
    uint8_t     DSR_IMP;
    uint16_t    C_SIZE;
    uint8_t     VDD_R_CURR_MIN;
    uint8_t     VDD_R_CURR_MAX;
    uint8_t     VDD_W_CURR_MIN;
    uint8_t     VDD_W_CURR_MAX;
    uint8_t     C_SIZE_MULT;
    uint8_t     ERASE_BLK_LEN;
    uint8_t     SECTOR_SIZE;
    uint8_t     WP_GRP_SIZE;
    uint8_t     WP_GRP_ENABLE;
    uint8_t     R2W_FACTOR;
    uint8_t     WRITE_BL_LEN;
    uint8_t     WRITE_BL_PARTIAL;
    uint8_t     FILE_FORMAT_GRP;
    uint8_t     COPY;
    uint8_t     PERM_WRITE_PROTECT;
    uint8_t     TMP_WRITE_PROTECT;
    uint8_t     FILE_FORMAT;
    uint8_t     CRC;
} CSD_t;

typedef enum {
    SDPROT_NO_DATA = 0,
    SDPROT_READ_DATA,
    SDPROT_WRITE_DATA,
} sdprot_datadir_t;

/*----- Extern function prototypes -----------------------------------*/

sdcard_response_t sdprot_resp_type(int cmd_nr);
int               sdprot_resp_crc(int cmd_nr);
sdprot_datadir_t  sdprot_need_data(int cmd_nr);
int               sdprot_need_busy(int cmd_nr);
int               sdprot_get_cid(CID_t* cid, const uint32_t* resp);
int               sdprot_print_cid(const CID_t* cid);
int               sdprot_get_csd(CSD_t* csd, const uint32_t* resp);
int               sdprot_print_csd(const CSD_t* csd);
uint32_t          sdprot_trans_speed(const CSD_t* csd);
uint32_t          sdprot_device_size(const CSD_t* csd);
sdp_cur_stat_t    sdprot_next_stat(int cmd_nr, uint8_t cur_stat);
const char*       sdprot_stat_name(int stat);
int               sdprot_print_r1_stat(const sdp_r1_stat_t* r1_stat);


#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
