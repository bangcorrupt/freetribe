@----------------------------------------------------------------------
@
@                     This file is part of Freetribe
@
@                https://github.com/bangcorrupt/freetribe
@
@                                License
@
@                   GNU AFFERO GENERAL PUBLIC LICENSE
@                      Version 3, 19 November 2007
@
@                           AGPL-3.0-or-later
@
@  Freetribe is free software: you can redistribute it and/or modify it
@ under the terms of the GNU Affero General Public License as published by
@    the Free Software Foundation, either version 3 of the License, or
@                   (at your option) any later version.
@
@     Freetribe is distributed in the hope that it will be useful,
@      but WITHOUT ANY WARRANTY; without even the implied warranty
@        of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
@          See the GNU General Public License for more details.
@
@   You should have received a copy of the GNU General Public License
@ along with this program. If not, see <https://www.gnu.org/licenses/>.
@
@                       Copyright bangcorrupt 2023
@
@----------------------------------------------------------------------
@
@ Original work by Texas Instruments, modified by bangcorrupt 2023.

@
@ Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
@
@  Redistribution and use in source and binary forms, with or without
@  modification, are permitted provided that the following conditions
@  are met:
@
@    Redistributions of source code must retain the above copyright
@    notice, this list of conditions and the following disclaimer.
@
@    Redistributions in binary form must reproduce the above copyright
@    notice, this list of conditions and the following disclaimer in the
@    documentation and/or other materials provided with the
@    distribution.
@
@    Neither the name of Texas Instruments Incorporated nor the names of
@    its contributors may be used to endorse or promote products derived
@    from this software without specific prior written permission.
@
@  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
@  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
@  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
@  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
@  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
@  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
@  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
@  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
@  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
@  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
@  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@


@****************************** Global Symbols*********************************
        .global start
        .global _stack                  
        .global _bss_start
        .global _bss_end
        .global _heap_start
        .global _heap_end
        .global start_boot

@************************ Internal Definitions ********************************
@
@ Define the stack sizes for different modes. The user/system mode will use
@ the rest of the total stack size
@
        .set  UND_STACK_SIZE, 0x8
        .set  ABT_STACK_SIZE, 0x8
        .set  FIQ_STACK_SIZE, 0x8
        .set  IRQ_STACK_SIZE, 0x600
        .set  SVC_STACK_SIZE, 0x200

@
@ to set the mode bits in CPSR for different modes
@        
        .set  MODE_USR, 0x10            
        .set  MODE_FIQ, 0x11
        .set  MODE_IRQ, 0x12
        .set  MODE_SVC, 0x13
        .set  MODE_ABT, 0x17
        .set  MODE_UND, 0x1B
        .set  MODE_SYS, 0x1F            

        .equ  I_F_BIT, 0xC0               

@**************************** Code Seection ***********************************
        .text

@
@ This code is assembled for ARM instructions
@
        .code 32

@******************************************************************************
@
@******************************************************************************
@
@ The reset handler sets up the stack pointers for all the modes. The FIQ and
@ IRQ shall be disabled during this. Then, clearthe BSS sections, switch to the
@ main() function. 
@
start:
@
@ Set up the Stack for Undefined mode
@
         LDR   r0, =_stack                     @ Read the stack address
         MSR   cpsr_c, #MODE_UND|I_F_BIT       @ switch to undef  mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0, r0, #UND_STACK_SIZE         @ give stack space
@
@ Set up the Stack for abort mode
@        
         MSR   cpsr_c, #MODE_ABT|I_F_BIT       @ Change to abort mode
         MOV   sp, r0                          @ write the stack pointer
         SUB   r0,r0, #ABT_STACK_SIZE          @ give stack space
@
@ Set up the Stack for FIQ mode
@       
         MSR   cpsr_c, #MODE_FIQ|I_F_BIT       @ change to FIQ mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0,r0, #FIQ_STACK_SIZE          @ give stack space
@
@ Set up the Stack for IRQ mode
@       
         MSR   cpsr_c, #MODE_IRQ|I_F_BIT       @ change to IRQ mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0,r0, #IRQ_STACK_SIZE          @ give stack space
@
@ Set up the Stack for SVC mode
@        
         MSR   cpsr_c, #MODE_SVC|I_F_BIT       @ change to SVC mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0,r0, #SVC_STACK_SIZE          @ give stack space
@
@ Set up the Stack for User/System mode
@      
         MSR   cpsr_c, #MODE_SYS|I_F_BIT       @ change to system mode
         MOV   sp,r0                           @ write the stack pointer


@
@ Clear the BSS section here
@
Clear_Bss_Section:

         LDR   r0, =_bss_start                 @ Start address of BSS
         LDR   r1, =(_bss_end - 0x04)          @ End address of BSS
         MOV   r2, #0
BSS_Loop: 
         STR   r2, [r0], #4                    @ Clear one word in BSS
         CMP   r0, r1
         BLE   BSS_Loop                        @ Clear till BSS end


@
@ Clear the Heap section here
@
Clear_Heap_Section:

         LDR   r0, =_heap_start                @ Start address of Heap
         LDR   r1, =(_heap_end - 0x04)         @ End address of Heap
         MOV   r2, #0
Heap_Loop: 
         STR   r2, [r0], #4                    @ Clear one word in Heap
         CMP   r0, r1
         BLE   Heap_Loop                       @ Clear till Heap end


@
@ Paint the Stack section here
@
Paint_Stack_Section:

         LDR   r0, =_stack                     @ Start address of Stack
         LDR   r1, =(_bss_end)                 @ End address of BSS
         LDR   r2, paint
Stack_Loop: 
         STR   r2, [r0], #-4                   @ Paint one word in Stack
         CMP   r0, r1
         BGT   Stack_Loop                      @ Paint till BSS end

@
@ Enter the start_boot function. The execution still happens in system mode
@
Enter_main:
         LDR   r10,=start_boot                 @ Get the address of start_boot
         MOV   lr,pc                           @ Dummy return 
         BX    r10                             @ Branch to start_boot
         SUB   pc, pc, #0x08                   @ looping   


paint:
    .word 0xdeadc0de

@
@ End of the file
@
         .end
