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

/*
 * @file    per_sport.c
 *
 * @brief   Peripheral driver for for BF523 SPORT.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include <blackfin.h>
#include <builtins.h>

#include "per_sport.h"

#include "knl_profile.h"

/*----- Macros and Definitions ---------------------------------------*/

/// TODO: Add this to defBF52x_base.h and rebuild toolchain.
//
#define DTYPE_SIGX 0x0004 /* SPORTx RCR1 Data Format Sign Extend */

/*----- Static variable definitions ----------------------------------*/

/// TODO: Do buffers need to be volatile?

// SPORT0 DMA transmit buffer
static fract32 g_codec_tx_buffer[BUFFER_LENGTH];
static fract32 g_temp_tx_buffer[BUFFER_LENGTH];
// SPORT0 DMA receive buffer
static fract32 g_codec_rx_buffer[BUFFER_LENGTH];
static fract32 g_temp_rx_buffer[BUFFER_LENGTH];

// 2 channels of input from ADC.
static t_audio_buffer g_codec_in;
// 2 channels of output to DAC.
static t_audio_buffer g_codec_out;

volatile static bool g_sport0_frame_received = false;

static uint32_t g_sport_isr_period[CYCLE_LOG_LENGTH];

// const fract32 sine_lut[128] = {
//     0x00000000, 0x0647D97C, 0x0C8BD35E, 0x12C8106F, 0x18F8B83C, 0x1F19F97B,
//     0x25280C5E, 0x2B1F34EB, 0x30FBC54D, 0x36BA2014, 0x3C56BA70, 0x41CE1E65,
//     0x471CECE7, 0x4C3FDFF4, 0x5133CC94, 0x55F5A4D2, 0x5A82799A, 0x5ED77C8A,
//     0x62F201AC, 0x66CF8120, 0x6A6D98A4, 0x6DCA0D14, 0x70E2CBC6, 0x73B5EBD1,
//     0x7641AF3D, 0x78848414, 0x7A7D055B, 0x7C29FBEE, 0x7D8A5F40, 0x7E9D55FC,
//     0x7F62368F, 0x7FD8878E, 0x7FFFFFFF, 0x7FD8878E, 0x7F62368F, 0x7E9D55FC,
//     0x7D8A5F40, 0x7C29FBEE, 0x7A7D055B, 0x78848414, 0x7641AF3D, 0x73B5EBD1,
//     0x70E2CBC6, 0x6DCA0D14, 0x6A6D98A4, 0x66CF8120, 0x62F201AC, 0x5ED77C8A,
//     0x5A82799A, 0x55F5A4D2, 0x5133CC94, 0x4C3FDFF4, 0x471CECE7, 0x41CE1E65,
//     0x3C56BA70, 0x36BA2014, 0x30FBC54D, 0x2B1F34EB, 0x25280C5E, 0x1F19F97B,
//     0x18F8B83C, 0x12C8106F, 0x0C8BD35E, 0x0647D97C, 0x00000000, 0xF9B82684,
//     0xF3742CA2, 0xED37EF91, 0xE70747C4, 0xE0E60685, 0xDAD7F3A2, 0xD4E0CB15,
//     0xCF043AB3, 0xC945DFEC, 0xC3A94590, 0xBE31E19B, 0xB8E31319, 0xB3C0200C,
//     0xAECC336C, 0xAA0A5B2E, 0xA57D8666, 0xA1288376, 0x9D0DFE54, 0x99307EE0,
//     0x9592675C, 0x9235F2EC, 0x8F1D343A, 0x8C4A142F, 0x89BE50C3, 0x877B7BEC,
//     0x8582FAA5, 0x83D60412, 0x8275A0C0, 0x8162AA04, 0x809DC971, 0x80277872,
//     0x80000000, 0x80277872, 0x809DC971, 0x8162AA04, 0x8275A0C0, 0x83D60412,
//     0x8582FAA5, 0x877B7BEC, 0x89BE50C3, 0x8C4A142F, 0x8F1D343A, 0x9235F2EC,
//     0x9592675C, 0x99307EE0, 0x9D0DFE54, 0xA1288376, 0xA57D8666, 0xAA0A5B2E,
//     0xAECC336C, 0xB3C0200C, 0xB8E31319, 0xBE31E19B, 0xC3A94590, 0xC945DFEC,
//     0xCF043AB3, 0xD4E0CB15, 0xDAD7F3A2, 0xE0E60685, 0xE70747C4, 0xED37EF91,
//     0xF3742CA2, 0xF9B82684};
/** Generated using Dr LUT - Free Lookup Table Generator
 * https://github.com/ppelikan/drlut
 **/
// Formula: sin(2*pi*t/T)
const int32_t sine_lut[1024] = {
    0x00000000, 0x00C90F88, 0x01921D20, 0x025B26D7, 0x03242ABF, 0x03ED26E6,
    0x04B6195D, 0x057F0035, 0x0647D97C, 0x0710A345, 0x07D95B9E, 0x08A2009A,
    0x096A9049, 0x0A3308BD, 0x0AFB6805, 0x0BC3AC35, 0x0C8BD35E, 0x0D53DB92,
    0x0E1BC2E4, 0x0EE38766, 0x0FAB272B, 0x1072A048, 0x1139F0CF, 0x120116D5,
    0x12C8106F, 0x138EDBB1, 0x145576B1, 0x151BDF86, 0x15E21445, 0x16A81305,
    0x176DD9DE, 0x183366E9, 0x18F8B83C, 0x19BDCBF3, 0x1A82A026, 0x1B4732EF,
    0x1C0B826A, 0x1CCF8CB3, 0x1D934FE5, 0x1E56CA1E, 0x1F19F97B, 0x1FDCDC1B,
    0x209F701C, 0x2161B3A0, 0x2223A4C5, 0x22E541AF, 0x23A6887F, 0x24677758,
    0x25280C5E, 0x25E845B6, 0x26A82186, 0x27679DF4, 0x2826B928, 0x28E5714B,
    0x29A3C485, 0x2A61B101, 0x2B1F34EB, 0x2BDC4E6F, 0x2C98FBBA, 0x2D553AFC,
    0x2E110A62, 0x2ECC681E, 0x2F875262, 0x3041C761, 0x30FBC54D, 0x31B54A5E,
    0x326E54C7, 0x3326E2C3, 0x33DEF287, 0x34968250, 0x354D9057, 0x36041AD9,
    0x36BA2014, 0x376F9E46, 0x382493B0, 0x38D8FE93, 0x398CDD32, 0x3A402DD2,
    0x3AF2EEB7, 0x3BA51E29, 0x3C56BA70, 0x3D07C1D6, 0x3DB832A6, 0x3E680B2C,
    0x3F1749B8, 0x3FC5EC98, 0x4073F21D, 0x4121589B, 0x41CE1E65, 0x427A41D0,
    0x4325C135, 0x43D09AED, 0x447ACD50, 0x452456BD, 0x45CD358F, 0x46756828,
    0x471CECE7, 0x47C3C22F, 0x4869E665, 0x490F57EE, 0x49B41533, 0x4A581C9E,
    0x4AFB6C98, 0x4B9E0390, 0x4C3FDFF4, 0x4CE10034, 0x4D8162C4, 0x4E210617,
    0x4EBFE8A5, 0x4F5E08E3, 0x4FFB654D, 0x5097FC5E, 0x5133CC94, 0x51CED46E,
    0x5269126E, 0x53028518, 0x539B2AF0, 0x5433027D, 0x54CA0A4B, 0x556040E2,
    0x55F5A4D2, 0x568A34A9, 0x571DEEFA, 0x57B0D256, 0x5842DD54, 0x58D40E8C,
    0x59646498, 0x59F3DE12, 0x5A82799A, 0x5B1035CF, 0x5B9D1154, 0x5C290ACC,
    0x5CB420E0, 0x5D3E5237, 0x5DC79D7C, 0x5E50015D, 0x5ED77C8A, 0x5F5E0DB3,
    0x5FE3B38D, 0x60686CCF, 0x60EC3830, 0x616F146C, 0x61F1003F, 0x6271FA69,
    0x62F201AC, 0x637114CC, 0x63EF3290, 0x646C59BF, 0x64E88926, 0x6563BF92,
    0x65DDFBD3, 0x66573CBB, 0x66CF8120, 0x6746C7D8, 0x67BD0FBD, 0x683257AB,
    0x68A69E81, 0x6919E320, 0x698C246C, 0x69FD614A, 0x6A6D98A4, 0x6ADCC964,
    0x6B4AF279, 0x6BB812D1, 0x6C242960, 0x6C8F351C, 0x6CF934FC, 0x6D6227FA,
    0x6DCA0D14, 0x6E30E34A, 0x6E96A99D, 0x6EFB5F12, 0x6F5F02B2, 0x6FC19385,
    0x7023109A, 0x708378FF, 0x70E2CBC6, 0x71410805, 0x719E2CD2, 0x71FA3949,
    0x72552C85, 0x72AF05A7, 0x7307C3D0, 0x735F6626, 0x73B5EBD1, 0x740B53FB,
    0x745F9DD1, 0x74B2C884, 0x7504D345, 0x7555BD4C, 0x75A585CF, 0x75F42C0B,
    0x7641AF3D, 0x768E0EA6, 0x76D94989, 0x77235F2D, 0x776C4EDB, 0x77B417DF,
    0x77FAB989, 0x78403329, 0x78848414, 0x78C7ABA2, 0x7909A92D, 0x794A7C12,
    0x798A23B1, 0x79C89F6E, 0x7A05EEAD, 0x7A4210D8, 0x7A7D055B, 0x7AB6CBA4,
    0x7AEF6323, 0x7B26CB4F, 0x7B5D039E, 0x7B920B89, 0x7BC5E290, 0x7BF88830,
    0x7C29FBEE, 0x7C5A3D50, 0x7C894BDE, 0x7CB72724, 0x7CE3CEB2, 0x7D0F4218,
    0x7D3980EC, 0x7D628AC6, 0x7D8A5F40, 0x7DB0FDF8, 0x7DD6668F, 0x7DFA98A8,
    0x7E1D93EA, 0x7E3F57FF, 0x7E5FE493, 0x7E7F3957, 0x7E9D55FC, 0x7EBA3A39,
    0x7ED5E5C6, 0x7EF05860, 0x7F0991C4, 0x7F2191B4, 0x7F3857F6, 0x7F4DE451,
    0x7F62368F, 0x7F754E80, 0x7F872BF3, 0x7F97CEBD, 0x7FA736B4, 0x7FB563B3,
    0x7FC25596, 0x7FCE0C3E, 0x7FD8878E, 0x7FE1C76B, 0x7FE9CBC0, 0x7FF09478,
    0x7FF62182, 0x7FFA72D1, 0x7FFD885A, 0x7FFF6216, 0x7FFFFFFF, 0x7FFF6216,
    0x7FFD885A, 0x7FFA72D1, 0x7FF62182, 0x7FF09478, 0x7FE9CBC0, 0x7FE1C76B,
    0x7FD8878E, 0x7FCE0C3E, 0x7FC25596, 0x7FB563B3, 0x7FA736B4, 0x7F97CEBD,
    0x7F872BF3, 0x7F754E80, 0x7F62368F, 0x7F4DE451, 0x7F3857F6, 0x7F2191B4,
    0x7F0991C4, 0x7EF05860, 0x7ED5E5C6, 0x7EBA3A39, 0x7E9D55FC, 0x7E7F3957,
    0x7E5FE493, 0x7E3F57FF, 0x7E1D93EA, 0x7DFA98A8, 0x7DD6668F, 0x7DB0FDF8,
    0x7D8A5F40, 0x7D628AC6, 0x7D3980EC, 0x7D0F4218, 0x7CE3CEB2, 0x7CB72724,
    0x7C894BDE, 0x7C5A3D50, 0x7C29FBEE, 0x7BF88830, 0x7BC5E290, 0x7B920B89,
    0x7B5D039E, 0x7B26CB4F, 0x7AEF6323, 0x7AB6CBA4, 0x7A7D055B, 0x7A4210D8,
    0x7A05EEAD, 0x79C89F6E, 0x798A23B1, 0x794A7C12, 0x7909A92D, 0x78C7ABA2,
    0x78848414, 0x78403329, 0x77FAB989, 0x77B417DF, 0x776C4EDB, 0x77235F2D,
    0x76D94989, 0x768E0EA6, 0x7641AF3D, 0x75F42C0B, 0x75A585CF, 0x7555BD4C,
    0x7504D345, 0x74B2C884, 0x745F9DD1, 0x740B53FB, 0x73B5EBD1, 0x735F6626,
    0x7307C3D0, 0x72AF05A7, 0x72552C85, 0x71FA3949, 0x719E2CD2, 0x71410805,
    0x70E2CBC6, 0x708378FF, 0x7023109A, 0x6FC19385, 0x6F5F02B2, 0x6EFB5F12,
    0x6E96A99D, 0x6E30E34A, 0x6DCA0D14, 0x6D6227FA, 0x6CF934FC, 0x6C8F351C,
    0x6C242960, 0x6BB812D1, 0x6B4AF279, 0x6ADCC964, 0x6A6D98A4, 0x69FD614A,
    0x698C246C, 0x6919E320, 0x68A69E81, 0x683257AB, 0x67BD0FBD, 0x6746C7D8,
    0x66CF8120, 0x66573CBB, 0x65DDFBD3, 0x6563BF92, 0x64E88926, 0x646C59BF,
    0x63EF3290, 0x637114CC, 0x62F201AC, 0x6271FA69, 0x61F1003F, 0x616F146C,
    0x60EC3830, 0x60686CCF, 0x5FE3B38D, 0x5F5E0DB3, 0x5ED77C8A, 0x5E50015D,
    0x5DC79D7C, 0x5D3E5237, 0x5CB420E0, 0x5C290ACC, 0x5B9D1154, 0x5B1035CF,
    0x5A82799A, 0x59F3DE12, 0x59646498, 0x58D40E8C, 0x5842DD54, 0x57B0D256,
    0x571DEEFA, 0x568A34A9, 0x55F5A4D2, 0x556040E2, 0x54CA0A4B, 0x5433027D,
    0x539B2AF0, 0x53028518, 0x5269126E, 0x51CED46E, 0x5133CC94, 0x5097FC5E,
    0x4FFB654D, 0x4F5E08E3, 0x4EBFE8A5, 0x4E210617, 0x4D8162C4, 0x4CE10034,
    0x4C3FDFF4, 0x4B9E0390, 0x4AFB6C98, 0x4A581C9E, 0x49B41533, 0x490F57EE,
    0x4869E665, 0x47C3C22F, 0x471CECE7, 0x46756828, 0x45CD358F, 0x452456BD,
    0x447ACD50, 0x43D09AED, 0x4325C135, 0x427A41D0, 0x41CE1E65, 0x4121589B,
    0x4073F21D, 0x3FC5EC98, 0x3F1749B8, 0x3E680B2C, 0x3DB832A6, 0x3D07C1D6,
    0x3C56BA70, 0x3BA51E29, 0x3AF2EEB7, 0x3A402DD2, 0x398CDD32, 0x38D8FE93,
    0x382493B0, 0x376F9E46, 0x36BA2014, 0x36041AD9, 0x354D9057, 0x34968250,
    0x33DEF287, 0x3326E2C3, 0x326E54C7, 0x31B54A5E, 0x30FBC54D, 0x3041C761,
    0x2F875262, 0x2ECC681E, 0x2E110A62, 0x2D553AFC, 0x2C98FBBA, 0x2BDC4E6F,
    0x2B1F34EB, 0x2A61B101, 0x29A3C485, 0x28E5714B, 0x2826B928, 0x27679DF4,
    0x26A82186, 0x25E845B6, 0x25280C5E, 0x24677758, 0x23A6887F, 0x22E541AF,
    0x2223A4C5, 0x2161B3A0, 0x209F701C, 0x1FDCDC1B, 0x1F19F97B, 0x1E56CA1E,
    0x1D934FE5, 0x1CCF8CB3, 0x1C0B826A, 0x1B4732EF, 0x1A82A026, 0x19BDCBF3,
    0x18F8B83C, 0x183366E9, 0x176DD9DE, 0x16A81305, 0x15E21445, 0x151BDF86,
    0x145576B1, 0x138EDBB1, 0x12C8106F, 0x120116D5, 0x1139F0CF, 0x1072A048,
    0x0FAB272B, 0x0EE38766, 0x0E1BC2E4, 0x0D53DB92, 0x0C8BD35E, 0x0BC3AC35,
    0x0AFB6805, 0x0A3308BD, 0x096A9049, 0x08A2009A, 0x07D95B9E, 0x0710A345,
    0x0647D97C, 0x057F0035, 0x04B6195D, 0x03ED26E6, 0x03242ABF, 0x025B26D7,
    0x01921D20, 0x00C90F88, 0x00000000, 0xFF36F078, 0xFE6DE2E0, 0xFDA4D929,
    0xFCDBD541, 0xFC12D91A, 0xFB49E6A3, 0xFA80FFCB, 0xF9B82684, 0xF8EF5CBB,
    0xF826A462, 0xF75DFF66, 0xF6956FB7, 0xF5CCF743, 0xF50497FB, 0xF43C53CB,
    0xF3742CA2, 0xF2AC246E, 0xF1E43D1C, 0xF11C789A, 0xF054D8D5, 0xEF8D5FB8,
    0xEEC60F31, 0xEDFEE92B, 0xED37EF91, 0xEC71244F, 0xEBAA894F, 0xEAE4207A,
    0xEA1DEBBB, 0xE957ECFB, 0xE8922622, 0xE7CC9917, 0xE70747C4, 0xE642340D,
    0xE57D5FDA, 0xE4B8CD11, 0xE3F47D96, 0xE330734D, 0xE26CB01B, 0xE1A935E2,
    0xE0E60685, 0xE02323E5, 0xDF608FE4, 0xDE9E4C60, 0xDDDC5B3B, 0xDD1ABE51,
    0xDC597781, 0xDB9888A8, 0xDAD7F3A2, 0xDA17BA4A, 0xD957DE7A, 0xD898620C,
    0xD7D946D8, 0xD71A8EB5, 0xD65C3B7B, 0xD59E4EFF, 0xD4E0CB15, 0xD423B191,
    0xD3670446, 0xD2AAC504, 0xD1EEF59E, 0xD13397E2, 0xD078AD9E, 0xCFBE389F,
    0xCF043AB3, 0xCE4AB5A2, 0xCD91AB39, 0xCCD91D3D, 0xCC210D79, 0xCB697DB0,
    0xCAB26FA9, 0xC9FBE527, 0xC945DFEC, 0xC89061BA, 0xC7DB6C50, 0xC727016D,
    0xC67322CE, 0xC5BFD22E, 0xC50D1149, 0xC45AE1D7, 0xC3A94590, 0xC2F83E2A,
    0xC247CD5A, 0xC197F4D4, 0xC0E8B648, 0xC03A1368, 0xBF8C0DE3, 0xBEDEA765,
    0xBE31E19B, 0xBD85BE30, 0xBCDA3ECB, 0xBC2F6513, 0xBB8532B0, 0xBADBA943,
    0xBA32CA71, 0xB98A97D8, 0xB8E31319, 0xB83C3DD1, 0xB796199B, 0xB6F0A812,
    0xB64BEACD, 0xB5A7E362, 0xB5049368, 0xB461FC70, 0xB3C0200C, 0xB31EFFCC,
    0xB27E9D3C, 0xB1DEF9E9, 0xB140175B, 0xB0A1F71D, 0xB0049AB3, 0xAF6803A2,
    0xAECC336C, 0xAE312B92, 0xAD96ED92, 0xACFD7AE8, 0xAC64D510, 0xABCCFD83,
    0xAB35F5B5, 0xAA9FBF1E, 0xAA0A5B2E, 0xA975CB57, 0xA8E21106, 0xA84F2DAA,
    0xA7BD22AC, 0xA72BF174, 0xA69B9B68, 0xA60C21EE, 0xA57D8666, 0xA4EFCA31,
    0xA462EEAC, 0xA3D6F534, 0xA34BDF20, 0xA2C1ADC9, 0xA2386284, 0xA1AFFEA3,
    0xA1288376, 0xA0A1F24D, 0xA01C4C73, 0x9F979331, 0x9F13C7D0, 0x9E90EB94,
    0x9E0EFFC1, 0x9D8E0597, 0x9D0DFE54, 0x9C8EEB34, 0x9C10CD70, 0x9B93A641,
    0x9B1776DA, 0x9A9C406E, 0x9A22042D, 0x99A8C345, 0x99307EE0, 0x98B93828,
    0x9842F043, 0x97CDA855, 0x9759617F, 0x96E61CE0, 0x9673DB94, 0x96029EB6,
    0x9592675C, 0x9523369C, 0x94B50D87, 0x9447ED2F, 0x93DBD6A0, 0x9370CAE4,
    0x9306CB04, 0x929DD806, 0x9235F2EC, 0x91CF1CB6, 0x91695663, 0x9104A0EE,
    0x90A0FD4E, 0x903E6C7B, 0x8FDCEF66, 0x8F7C8701, 0x8F1D343A, 0x8EBEF7FB,
    0x8E61D32E, 0x8E05C6B7, 0x8DAAD37B, 0x8D50FA59, 0x8CF83C30, 0x8CA099DA,
    0x8C4A142F, 0x8BF4AC05, 0x8BA0622F, 0x8B4D377C, 0x8AFB2CBB, 0x8AAA42B4,
    0x8A5A7A31, 0x8A0BD3F5, 0x89BE50C3, 0x8971F15A, 0x8926B677, 0x88DCA0D3,
    0x8893B125, 0x884BE821, 0x88054677, 0x87BFCCD7, 0x877B7BEC, 0x8738545E,
    0x86F656D3, 0x86B583EE, 0x8675DC4F, 0x86376092, 0x85FA1153, 0x85BDEF28,
    0x8582FAA5, 0x8549345C, 0x85109CDD, 0x84D934B1, 0x84A2FC62, 0x846DF477,
    0x843A1D70, 0x840777D0, 0x83D60412, 0x83A5C2B0, 0x8376B422, 0x8348D8DC,
    0x831C314E, 0x82F0BDE8, 0x82C67F14, 0x829D753A, 0x8275A0C0, 0x824F0208,
    0x82299971, 0x82056758, 0x81E26C16, 0x81C0A801, 0x81A01B6D, 0x8180C6A9,
    0x8162AA04, 0x8145C5C7, 0x812A1A3A, 0x810FA7A0, 0x80F66E3C, 0x80DE6E4C,
    0x80C7A80A, 0x80B21BAF, 0x809DC971, 0x808AB180, 0x8078D40D, 0x80683143,
    0x8058C94C, 0x804A9C4D, 0x803DAA6A, 0x8031F3C2, 0x80277872, 0x801E3895,
    0x80163440, 0x800F6B88, 0x8009DE7E, 0x80058D2F, 0x800277A6, 0x80009DEA,
    0x80000000, 0x80009DEA, 0x800277A6, 0x80058D2F, 0x8009DE7E, 0x800F6B88,
    0x80163440, 0x801E3895, 0x80277872, 0x8031F3C2, 0x803DAA6A, 0x804A9C4D,
    0x8058C94C, 0x80683143, 0x8078D40D, 0x808AB180, 0x809DC971, 0x80B21BAF,
    0x80C7A80A, 0x80DE6E4C, 0x80F66E3C, 0x810FA7A0, 0x812A1A3A, 0x8145C5C7,
    0x8162AA04, 0x8180C6A9, 0x81A01B6D, 0x81C0A801, 0x81E26C16, 0x82056758,
    0x82299971, 0x824F0208, 0x8275A0C0, 0x829D753A, 0x82C67F14, 0x82F0BDE8,
    0x831C314E, 0x8348D8DC, 0x8376B422, 0x83A5C2B0, 0x83D60412, 0x840777D0,
    0x843A1D70, 0x846DF477, 0x84A2FC62, 0x84D934B1, 0x85109CDD, 0x8549345C,
    0x8582FAA5, 0x85BDEF28, 0x85FA1153, 0x86376092, 0x8675DC4F, 0x86B583EE,
    0x86F656D3, 0x8738545E, 0x877B7BEC, 0x87BFCCD7, 0x88054677, 0x884BE821,
    0x8893B125, 0x88DCA0D3, 0x8926B677, 0x8971F15A, 0x89BE50C3, 0x8A0BD3F5,
    0x8A5A7A31, 0x8AAA42B4, 0x8AFB2CBB, 0x8B4D377C, 0x8BA0622F, 0x8BF4AC05,
    0x8C4A142F, 0x8CA099DA, 0x8CF83C30, 0x8D50FA59, 0x8DAAD37B, 0x8E05C6B7,
    0x8E61D32E, 0x8EBEF7FB, 0x8F1D343A, 0x8F7C8701, 0x8FDCEF66, 0x903E6C7B,
    0x90A0FD4E, 0x9104A0EE, 0x91695663, 0x91CF1CB6, 0x9235F2EC, 0x929DD806,
    0x9306CB04, 0x9370CAE4, 0x93DBD6A0, 0x9447ED2F, 0x94B50D87, 0x9523369C,
    0x9592675C, 0x96029EB6, 0x9673DB94, 0x96E61CE0, 0x9759617F, 0x97CDA855,
    0x9842F043, 0x98B93828, 0x99307EE0, 0x99A8C345, 0x9A22042D, 0x9A9C406E,
    0x9B1776DA, 0x9B93A641, 0x9C10CD70, 0x9C8EEB34, 0x9D0DFE54, 0x9D8E0597,
    0x9E0EFFC1, 0x9E90EB94, 0x9F13C7D0, 0x9F979331, 0xA01C4C73, 0xA0A1F24D,
    0xA1288376, 0xA1AFFEA3, 0xA2386284, 0xA2C1ADC9, 0xA34BDF20, 0xA3D6F534,
    0xA462EEAC, 0xA4EFCA31, 0xA57D8666, 0xA60C21EE, 0xA69B9B68, 0xA72BF174,
    0xA7BD22AC, 0xA84F2DAA, 0xA8E21106, 0xA975CB57, 0xAA0A5B2E, 0xAA9FBF1E,
    0xAB35F5B5, 0xABCCFD83, 0xAC64D510, 0xACFD7AE8, 0xAD96ED92, 0xAE312B92,
    0xAECC336C, 0xAF6803A2, 0xB0049AB3, 0xB0A1F71D, 0xB140175B, 0xB1DEF9E9,
    0xB27E9D3C, 0xB31EFFCC, 0xB3C0200C, 0xB461FC70, 0xB5049368, 0xB5A7E362,
    0xB64BEACD, 0xB6F0A812, 0xB796199B, 0xB83C3DD1, 0xB8E31319, 0xB98A97D8,
    0xBA32CA71, 0xBADBA943, 0xBB8532B0, 0xBC2F6513, 0xBCDA3ECB, 0xBD85BE30,
    0xBE31E19B, 0xBEDEA765, 0xBF8C0DE3, 0xC03A1368, 0xC0E8B648, 0xC197F4D4,
    0xC247CD5A, 0xC2F83E2A, 0xC3A94590, 0xC45AE1D7, 0xC50D1149, 0xC5BFD22E,
    0xC67322CE, 0xC727016D, 0xC7DB6C50, 0xC89061BA, 0xC945DFEC, 0xC9FBE527,
    0xCAB26FA9, 0xCB697DB0, 0xCC210D79, 0xCCD91D3D, 0xCD91AB39, 0xCE4AB5A2,
    0xCF043AB3, 0xCFBE389F, 0xD078AD9E, 0xD13397E2, 0xD1EEF59E, 0xD2AAC504,
    0xD3670446, 0xD423B191, 0xD4E0CB15, 0xD59E4EFF, 0xD65C3B7B, 0xD71A8EB5,
    0xD7D946D8, 0xD898620C, 0xD957DE7A, 0xDA17BA4A, 0xDAD7F3A2, 0xDB9888A8,
    0xDC597781, 0xDD1ABE51, 0xDDDC5B3B, 0xDE9E4C60, 0xDF608FE4, 0xE02323E5,
    0xE0E60685, 0xE1A935E2, 0xE26CB01B, 0xE330734D, 0xE3F47D96, 0xE4B8CD11,
    0xE57D5FDA, 0xE642340D, 0xE70747C4, 0xE7CC9917, 0xE8922622, 0xE957ECFB,
    0xEA1DEBBB, 0xEAE4207A, 0xEBAA894F, 0xEC71244F, 0xED37EF91, 0xEDFEE92B,
    0xEEC60F31, 0xEF8D5FB8, 0xF054D8D5, 0xF11C789A, 0xF1E43D1C, 0xF2AC246E,
    0xF3742CA2, 0xF43C53CB, 0xF50497FB, 0xF5CCF743, 0xF6956FB7, 0xF75DFF66,
    0xF826A462, 0xF8EF5CBB, 0xF9B82684, 0xFA80FFCB, 0xFB49E6A3, 0xFC12D91A,
    0xFCDBD541, 0xFDA4D929, 0xFE6DE2E0, 0xFF36F078};

static fract32 square_lut[1024];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _sport0_isr(void) __attribute__((interrupt_handler));

/*----- Extern function implementations ------------------------------*/

void sport0_init(void) {

    int j;
    for (j = 0; j < 1024; j++) {

        if (j < 512) {
            square_lut[j] = 0x80000000;
        } else {
            square_lut[j] = 0x7fffffff;
        }
    }

    /// TODO: Do we need secondary enabled?

    // Configure SPORT0 Rx.
    // Clock Falling Edge, Receive Frame Sync, Data Format Sign Extend.
    *pSPORT0_RCR1 = RCKFE | RFSR | DTYPE_SIGX;
    // Rx Stereo Frame Sync Enable, Rx Word Length 32 bit.
    *pSPORT0_RCR2 = RSFSE | SLEN(0x1f);

    // Configure SPORT0 Tx.
    *pSPORT0_TCR1 = TCKFE | TFSR;
    *pSPORT0_TCR2 = TSFSE | SLEN(0x1f);
    ssync();

    /// TODO: DMA linked descriptor mode.

    // SPORT0 Rx DMA.
    *pDMA3_PERIPHERAL_MAP = PMAP_SPORT0RX;
    /* *pDMA3_CONFIG = 0x108a; // 0x760a; */
    *pDMA3_CONFIG = FLOW_AUTO | DI_EN | WDSIZE_32 | WNR;
    // Start address of data buffer.
    *pDMA3_START_ADDR = &g_codec_rx_buffer;
    // DMA inner loop count.
    // *pDMA3_X_COUNT = 2; // 2 samples.
    *pDMA3_X_COUNT = BUFFER_LENGTH;
    *pDMA3_X_MODIFY = SAMPLE_SIZE;
    ssync();

    // SPORT0 Tx DMA.
    *pDMA4_PERIPHERAL_MAP = PMAP_SPORT0TX;
    /* *pDMA4_CONFIG = 0x1008; // 0x7608; */
    *pDMA4_CONFIG = FLOW_AUTO | WDSIZE_32 | DI_EN;
    // Start address of data buffer
    *pDMA4_START_ADDR = &g_codec_tx_buffer;
    // DMA inner loop count
    // *pDMA4_X_COUNT = 2; // 2 samples.
    *pDMA4_X_COUNT = BUFFER_LENGTH;
    // Inner loop address increment
    // *pDMA4_X_MODIFY = 4; // 32 bit.
    *pDMA4_X_MODIFY = SAMPLE_SIZE;
    ssync();

    // SPORT0 Rx DMA3 interrupt IVG9.
    *pSIC_IAR2 |= P16_IVG(9);
    ssync();

    // Set SPORT0 Rx interrupt vector.
    *pEVT9 = _sport0_isr;
    ssync();

    // Enable SPORT0 Rx interrupt.
    *pSIC_IMASK0 |= IRQ_DMA3;
    *pSIC_IMASK0 |= IRQ_DMA4;
    ssync();

    int i;
    // unmask in the core event processor
    asm volatile("cli %0; bitset(%0, 9); sti %0; csync;" : "+d"(i));
    ssync();

    // Enable SPORT0 Rx DMA.
    *pDMA3_CONFIG |= DMAEN;
    ssync();

    // Enable SPORT0 Tx DMA.
    *pDMA4_CONFIG |= DMAEN;
    ssync();

    // Enable SPORT0 Rx.
    *pSPORT0_RCR1 |= RSPEN;
    ssync();

    // Enable SPORT0 Tx.
    *pSPORT0_TCR1 |= TSPEN;
    ssync();
}

void sport1_init(void) {

    // /// TODO: Do we need secondary enabled?
    //
    // // Configure SPORT1 Rx.
    // *pSPORT1_RCR1 = RCKFE | RFSR | DTYPE_SIGX;
    // *pSPORT1_RCR2 = RSFSE | RXSE | SLEN(0x1f);
    //
    // // Configure SPORT1 Tx.
    // *pSPORT1_TCR1 = TCKFE | TFSR;
    // *pSPORT1_TCR2 = TSFSE | TXSE | SLEN(0x1f);
    // ssync();
    //
    // /// TODO: DMA linked descriptor mode.
    //
    // // SPORT1 Rx DMA.
    // *pDMA5_PERIPHERAL_MAP = PMAP_SPORT1RX;
    // // Configure DMA.
    // *pDMA5_CONFIG = FLOW_AUTO | WDSIZE_32 | WNR;
    // // Start address of data buffer.
    // *pDMA5_START_ADDR = &g_cpu_rx_buffer;
    // // DMA inner loop count.
    // *pDMA5_X_COUNT = 2; // 2 samples.
    // // Inner loop address increment.
    // *pDMA5_X_MODIFY = 4; // 32 bit.
    // ssync();
    //
    // // SPORT1 Tx DMA.
    // *pDMA6_PERIPHERAL_MAP = PMAP_SPORT1TX;
    // // Configure DMA.
    // *pDMA6_CONFIG = FLOW_AUTO | WDSIZE_32;
    // // Start address of data buffer
    // *pDMA6_START_ADDR = &g_cpu_tx_buffer;
    // // DMA inner loop count
    // *pDMA6_X_COUNT = 2; // 2 samples.
    // // Inner loop address increment
    // *pDMA6_X_MODIFY = 4; // 32 bit.
    // ssync();
    //
    // // Enable SPORT1 Rx DMA.
    // *pDMA5_CONFIG |= DMAEN;
    // ssync();
    //
    // // Enable SPORT1 Tx DMA.
    // *pDMA6_CONFIG |= DMAEN;
    // ssync();
    //
    // // Enable SPORT1 Rx.
    // *pSPORT1_RCR1 |= RSPEN;
    // ssync();
    //
    // // Enable SPORT1 Tx.
    // *pSPORT1_TCR1 |= TSPEN;
    // ssync();
}

inline t_audio_buffer *sport0_get_rx_buffer(void) { return &g_codec_in; }

inline t_audio_buffer *sport0_get_tx_buffer(void) { return &g_codec_out; }

/// TODO: DMA ping-pong block buffer.
inline bool sport0_frame_received(void) { return g_sport0_frame_received; }

inline void sport0_frame_processed(void) { g_sport0_frame_received = false; }

/// TODO: Block processing.  For now we process each frame as it arrives.
__attribute__((interrupt_handler)) static void _sport0_isr(void) {

    // static int i = 0;

    // static uint32_t cycles_this;
    // static uint32_t cycles_last;

    if (*pDMA3_IRQ_STATUS) {

        // Clear interrupt status.
        *pDMA3_IRQ_STATUS = DMA_DONE;
        ssync();
        int j;
        for (j = 0; j < BUFFER_LENGTH; j++) {

            // Get input from codec.
            g_temp_rx_buffer[j] = g_codec_rx_buffer[j];
        }
    }

    if (*pDMA4_IRQ_STATUS) {

        // Clear interrupt status.
        *pDMA4_IRQ_STATUS = DMA_DONE;
        ssync();
        int j;
        for (j = 0; j < BUFFER_LENGTH; j++) {

            // Get input from codec.
            g_codec_tx_buffer[j] = g_temp_rx_buffer[j];
        }
    }
    // cycles_this = cycles();
    //
    // g_sport_isr_period[i] = cycles_this - cycles_last;
    //
    // cycles_last = cycles_this;
    //
    // if (i >= CYCLE_LOG_LENGTH) {
    //     i = 0;
    // }

    /// TODO: DMA ping-pong block buffer.

    // int j;
    // for (j = 0; j < BLOCK_SIZE; j++) {
    //
    //     // Get input from codec.
    //     g_codec_in[0][j] = g_codec_rx_buffer[j * 2];
    //     g_codec_in[1][j] = g_codec_rx_buffer[j * 2 + 1];
    //
    //     // Send output to codec.
    //     g_codec_tx_buffer[j * 2] = g_codec_out[0][j];
    //     g_codec_tx_buffer[j * 2 + 1] = g_codec_out[1][j];
    // }
    // int j;
    // for (j = 0; j < BLOCK_SIZE; j++) {
    //
    //     // Get input from codec.
    //     g_codec_tx_buffer[j * 2] = g_codec_rx_buffer[j * 2];
    //     g_codec_tx_buffer[j * 2 + 1] = g_codec_rx_buffer[j * 2 + 1];
    // }
    // int j;
    // static int k = 0;
    // for (j = 0; j < BUFFER_LENGTH; j++) {
    //
    //     // Get input from codec.
    //     g_codec_tx_buffer[j] = sine_lut[k++];
    //
    //     if (k >= 1024) {
    //         k = 0;
    //     }
    // }
    // int j;
    // static int k = 0;
    // for (j = 0; j < BLOCK_SIZE; j++) {
    //
    //     // Get input from codec.
    //     // g_codec_tx_buffer[j * 2] = sine_lut[k];
    //     // g_codec_tx_buffer[j * 2 + 1] = sine_lut[k];
    //
    //     g_codec_tx_buffer[j * 2] = square_lut[k];
    //     g_codec_tx_buffer[j * 2 + 1] = square_lut[k];
    //     // g_codec_tx_buffer[j * 2] = sine_lut[k];
    //     // g_codec_tx_buffer[j * 2 + 1] = sine_lut[k];
    //
    //     if (++k >= 1024) {
    //         k = 0;
    //     }
    // }

    g_sport0_frame_received = true;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
