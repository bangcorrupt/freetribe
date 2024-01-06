/**
 *   \file  hw_i2c.h
 *
 *   \brief This file contains the Register Descriptions for I2C
 */

/*
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 */
/*
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef HW_I2C_H
#define HW_I2C_H

#define I2C_ICOAR (0x0)
#define I2C_ICIMR (0x4)
#define I2C_ICSTR (0x8)
#define I2C_ICCLKL (0xC)
#define I2C_ICCLKH (0x10)
#define I2C_ICCNT (0x14)
#define I2C_ICDRR (0x18)
#define I2C_ICSAR (0x1C)
#define I2C_ICDXR (0x20)
#define I2C_ICMDR (0x24)
#define I2C_ICIVR (0x28)
#define I2C_ICEMDR (0x2C)
#define I2C_ICPSC (0x30)
#define I2C_REVID1 (0x34)
#define I2C_REVID2 (0x38)
#define I2C_ICDMAC (0x3C)
#define I2C_ICPFUNC (0x48)
#define I2C_ICPDIR (0x4C)
#define I2C_ICPDIN (0x50)
#define I2C_ICPDOUT (0x54)
#define I2C_ICPDSET (0x58)
#define I2C_ICPDCLR (0x5C)

/**************************************************************************\
* Field Definition Macros
\**************************************************************************/

/* ICOAR */
#define I2C_ICOAR_OADDR (0x000003FFu)
#define I2C_ICOAR_OADDR_SHIFT (0x00000000u)

/* ICIMR */
#define I2C_ICIMR_AAS (0x00000040u)
#define I2C_ICIMR_AAS_SHIFT (0x00000006u)
#define I2C_ICIMR_SCD (0x00000020u)
#define I2C_ICIMR_SCD_SHIFT (0x00000005u)
#define I2C_ICIMR_ICXRDY (0x00000010u)
#define I2C_ICIMR_ICXRDY_SHIFT (0x00000004u)
#define I2C_ICIMR_ICRRDY (0x00000008u)
#define I2C_ICIMR_ICRRDY_SHIFT (0x00000003u)
#define I2C_ICIMR_ARDY (0x00000004u)
#define I2C_ICIMR_ARDY_SHIFT (0x00000002u)
#define I2C_ICIMR_NACK (0x00000002u)
#define I2C_ICIMR_NACK_SHIFT (0x00000001u)
#define I2C_ICIMR_AL (0x00000001u)
#define I2C_ICIMR_AL_SHIFT (0x00000000u)

/* ICSTR */
#define I2C_ICSTR_SDIR (0x00004000u)
#define I2C_ICSTR_SDIR_SHIFT (0x0000000Eu)
#define I2C_ICSTR_NACKSNT (0x00002000u)
#define I2C_ICSTR_NACKSNT_SHIFT (0x0000000Du)
#define I2C_ICSTR_BB (0x00001000u)
#define I2C_ICSTR_BB_SHIFT (0x0000000Cu)
#define I2C_ICSTR_RSFULL (0x00000800u)
#define I2C_ICSTR_RSFULL_SHIFT (0x0000000Bu)
#define I2C_ICSTR_XSMT (0x00000400u)
#define I2C_ICSTR_XSMT_SHIFT (0x0000000Au)
#define I2C_ICSTR_AAS (0x00000200u)
#define I2C_ICSTR_AAS_SHIFT (0x00000009u)
#define I2C_ICSTR_AD0 (0x00000100u)
#define I2C_ICSTR_AD0_SHIFT (0x00000008u)
#define I2C_ICSTR_SCD (0x00000020u)
#define I2C_ICSTR_SCD_SHIFT (0x00000005u)
#define I2C_ICSTR_ICXRDY (0x00000010u)
#define I2C_ICSTR_ICXRDY_SHIFT (0x00000004u)
#define I2C_ICSTR_ICRRDY (0x00000008u)
#define I2C_ICSTR_ICRRDY_SHIFT (0x00000003u)
#define I2C_ICSTR_ARDY (0x00000004u)
#define I2C_ICSTR_ARDY_SHIFT (0x00000002u)
#define I2C_ICSTR_NACK (0x00000002u)
#define I2C_ICSTR_NACK_SHIFT (0x00000001u)
#define I2C_ICSTR_AL (0x00000001u)
#define I2C_ICSTR_AL_SHIFT (0x00000000u)

/* ICCLKL */
#define I2C_ICCLKL_ICCL (0x0000FFFFu)
#define I2C_ICCLKL_ICCL_SHIFT (0x00000000u)

/* ICCLKH */
#define I2C_ICCLKH_ICCH (0x0000FFFFu)
#define I2C_ICCLKH_ICCH_SHIFT (0x00000000u)

/* ICCNT */
#define I2C_ICCNT_ICDC (0x0000FFFFu)
#define I2C_ICCNT_ICDC_SHIFT (0x00000000u)

/* ICDRR */
#define I2C_ICDRR_D (0x000000FFu)
#define I2C_ICDRR_D_SHIFT (0x00000000u)

/* ICSAR */
#define I2C_ICSAR_SADDR (0x000003FFu)
#define I2C_ICSAR_SADDR_SHIFT (0x00000000u)

/* ICDXR */
#define I2C_ICDXR_D (0x000000FFu)
#define I2C_ICDXR_D_SHIFT (0x00000000u)

/* ICMDR */
#define I2C_ICMDR_NACKMOD (0x00008000u)
#define I2C_ICMDR_NACKMOD_SHIFT (0x0000000Fu)
#define I2C_ICMDR_FREE (0x00004000u)
#define I2C_ICMDR_FREE_SHIFT (0x0000000Eu)
#define I2C_ICMDR_STT (0x00002000u)
#define I2C_ICMDR_STT_SHIFT (0x0000000Du)
#define I2C_ICMDR_STP (0x00000800u)
#define I2C_ICMDR_STP_SHIFT (0x0000000Bu)
#define I2C_ICMDR_MST (0x00000400u)
#define I2C_ICMDR_MST_SHIFT (0x0000000Au)
#define I2C_ICMDR_TRX (0x00000200u)
#define I2C_ICMDR_TRX_SHIFT (0x00000009u)
#define I2C_ICMDR_XA (0x00000100u)
#define I2C_ICMDR_XA_SHIFT (0x00000008u)
#define I2C_ICMDR_RM (0x00000080u)
#define I2C_ICMDR_RM_SHIFT (0x00000007u)
#define I2C_ICMDR_DLB (0x00000040u)
#define I2C_ICMDR_DLB_SHIFT (0x00000006u)
#define I2C_ICMDR_IRS (0x00000020u)
#define I2C_ICMDR_IRS_SHIFT (0x00000005u)
#define I2C_ICMDR_STB (0x00000010u)
#define I2C_ICMDR_STB_SHIFT (0x00000004u)
#define I2C_ICMDR_FDF (0x00000008u)
#define I2C_ICMDR_FDF_SHIFT (0x00000003u)
#define I2C_ICMDR_BC (0x00000007u)
#define I2C_ICMDR_BC_SHIFT (0x00000000u)
/*----BC Tokens----*/
#define I2C_ICMDR_BC_8BIT (0x00000000u)
#define I2C_ICMDR_BC_1BIT (0x00000001u)
#define I2C_ICMDR_BC_2BIT (0x00000002u)
#define I2C_ICMDR_BC_3BIT (0x00000003u)
#define I2C_ICMDR_BC_4BIT (0x00000004u)
#define I2C_ICMDR_BC_5BIT (0x00000005u)
#define I2C_ICMDR_BC_6BIT (0x00000006u)
#define I2C_ICMDR_BC_7BIT (0x00000007u)

/* ICIVR */
#define I2C_ICIVR_INTCODE (0x00000007u)
#define I2C_ICIVR_INTCODE_SHIFT (0x00000000u)
/*----INTCODE Tokens----*/
#define I2C_ICIVR_INTCODE_NONE (0x00000000u)
#define I2C_ICIVR_INTCODE_AL (0x00000001u)
#define I2C_ICIVR_INTCODE_NACK (0x00000002u)
#define I2C_ICIVR_INTCODE_ARDY (0x00000003u)
#define I2C_ICIVR_INTCODE_ICRRDY (0x00000004u)
#define I2C_ICIVR_INTCODE_ICXRDY (0x00000005u)
#define I2C_ICIVR_INTCODE_SCD (0x00000006u)
#define I2C_ICIVR_INTCODE_AAS (0x00000007u)

/* ICEMDR */
#define I2C_ICEMDR_IGNACK (0x00000002u)
#define I2C_ICEMDR_IGNACK_SHIFT (0x00000001u)
#define I2C_ICEMDR_BCM (0x00000001u)
#define I2C_ICEMDR_BCM_SHIFT (0x00000000u)

/* ICPSC */
#define I2C_ICPSC_IPSC (0x000000FFu)
#define I2C_ICPSC_IPSC_SHIFT (0x00000000u)

/* ICPID1 */
#define I2C_REVID1_REV (0xFFFFFFFFu)
#define I2C_REVID1_REV_SHIFT (0x00000008u)

/* ICPID2 */
#define I2C_REVID2_REV (0xFFFFFFFFu)
#define I2C_REVID2_REV_SHIFT (0x00000000u)

/* ICDMAC */
#define I2C_ICDMAC_TXDMAEN (0x00000002u)
#define I2C_ICDMAC_TXDMAEN_SHIFT (0x00000001u)
#define I2C_ICDMAC_RXDMAEN (0x00000001u)
#define I2C_ICDMAC_RXDMAEN_SHIFT (0x00000000u)

/* ICPFUNC */

/*----PFUNC Tokens----*/

/* ICPDIR */

#define I2C_ICPDIR_PDIR1 (0x00000002u)
#define I2C_ICPDIR_PDIR1_SHIFT (0x00000001u)
#define I2C_ICPDIR_PDIR0 (0x00000001u)
#define I2C_ICPDIR_PDIR0_SHIFT (0x00000000u)

/* ICPDIN */

#define I2C_ICPDIN_PDIN1 (0x00000002u)
#define I2C_ICPDIN_PDIN1_SHIFT (0x00000001u)
#define I2C_ICPDIN_PDIN0 (0x00000001u)
#define I2C_ICPDIN_PDIN0_SHIFT (0x00000000u)

/* ICPDOUT */
#define I2C_ICPDOUT_PDOUT1 (0x00000002u)
#define I2C_ICPDOUT_PDOUT1_SHIFT (0x00000001u)
#define I2C_ICPDOUT_PDOUT0 (0x00000001u)
#define I2C_ICPDOUT_PDOUT0_SHIFT (0x00000000u)

/* ICPDSET */
#define I2C_ICPDSET_PDSET1 (0x00000002u)
#define I2C_ICPDSET_PDSET1_SHIFT (0x00000001u)
#define I2C_ICPDSET_PDSET0 (0x00000001u)
#define I2C_ICPDSET_PDSET0_SHIFT (0x00000000u)

/* ICPDCLR */
#define I2C_ICPDCLR_PDCLR1 (0x00000002u)
#define I2C_ICPDCLR_PDCLR1_SHIFT (0x00000001u)
#define I2C_ICPDCLR_PDCLR0 (0x00000001u)
#define I2C_ICPDCLR_PDCLR0_SHIFT (0x00000000u)

#endif
