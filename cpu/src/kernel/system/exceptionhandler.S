@-----------------------------------------------------------------------
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
@-----------------------------------------------------------------------
@
@ Original work by Texas Instruments, modified by bangcorrupt 2023.
@ Adapted from AM335x Starterware.

@
@ Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
@
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
@***********************************************************************

@***********************************************************************
@
@ exceptionhandler.S - Definitions of exception handlers
@
@***********************************************************************

@
@ Share header file with assembly source code
@

#include "hw_intc.h"
#include "hw_aintc.h"
#include "soc_AM1808.h"

@************************** Global symbols *****************************
        .global IRQHandler
        .global FIQHandler
        .global AbortHandler
        .global SWIHandler
        .global UndefInstHandler
        .global CPUAbortHandler
        .global fnRAMVectors  
        .global p_IRQPriority
        
        .equ ADDR_HIPVR1, SOC_AINTC_0_REGS + AINTC_HIPVR(0)
        .equ ADDR_HIPVR2, SOC_AINTC_0_REGS + AINTC_HIPVR(1)
        .equ ADDR_HIPIR1, SOC_AINTC_0_REGS + AINTC_HIPIR(0)
        .equ ADDR_HIPIR2, SOC_AINTC_0_REGS + AINTC_HIPIR(1)
        .equ ADDR_HINLR1, SOC_AINTC_0_REGS + AINTC_HINLR(0)
        .equ ADDR_HINLR2, SOC_AINTC_0_REGS + AINTC_HINLR(1)
        .equ ADDR_SECR, SOC_AINTC_0_REGS + AINTC_SECR(0)
        .equ ADDR_SICR, SOC_AINTC_0_REGS + AINTC_SICR
        .equ MASK_SWI_NUM, 0xFF000000
 
        .equ PRI_INDEX_MASK, AINTC_HIPIR_PRI_INDX
        .equ OVERRIDE_MASK, AINTC_HINLR_OVERRIDE

        .equ INTR_BYTE_SHIFT, 0x03
        .equ INTR_BIT_MASK, 0x07

        .equ ADDR_SIR_IRQ, SOC_AINTC_0_REGS + INTC_SIR_IRQ
        .equ ADDR_SIR_FIQ, SOC_AINTC_0_REGS + INTC_SIR_FIQ
        .equ ADDR_CONTROL, SOC_AINTC_0_REGS + INTC_CONTROL
        .equ ADDR_THRESHOLD, SOC_AINTC_0_REGS + INTC_THRESHOLD
        .equ ADDR_IRQ_PRIORITY, SOC_AINTC_0_REGS + INTC_IRQ_PRIORITY


        .equ MASK_ACTIVE_IRQ, INTC_SIR_IRQ_ACTIVEIRQ
        .equ MASK_ACTIVE_FIQ, INTC_SIR_FIQ_ACTIVEFIQ
        .equ NEWIRQAGR, INTC_CONTROL_NEWIRQAGR
        .equ NEWFIQAGR, INTC_CONTROL_NEWFIQAGR

        .equ MODE_SYS, 0x1F
        .equ MODE_IRQ, 0x12 
        .equ I_BIT, 0x80

@**************************** Text Section *****************************
        .text

@ This source file is assembled for ARM instructions
        .code 32
@***********************************************************************
@*                  Function Definition of SWI Handler
@***********************************************************************
@
@ The SWI Handler switches to system mode if the SWI number is 458752. If the
@ SWI number is different, no mode switching will be done. No other SWI are 
@ handled here
@
SWIHandler:
        STMFD    r13!, {r0-r1, r14}       @ Save context in SVC stack
        LDR      r0, [r14, #-4]           @ R0 points to SWI instruction
        BIC      r0, r0, #MASK_SWI_NUM    @ Get the SWI number
        CMP      r0, #458752
        MRSEQ    r1, spsr                 @ Copy SPSR  
        ORREQ    r1, r1, #0x1F            @ Change the mode to System
        MSREQ    spsr_cf, r1              @ Restore SPSR
        LDMFD    r13!, {r0-r1, pc}^       @ Restore registers from IRQ stack


@***********************************************************************
@*                  Function Definition of IRQ Handler
@***********************************************************************   
@
@ The IRQ handler jumps to the ISR of highest priority pending IRQ. The address
@ is taken from the HIPVR2 register, which contains the ISR address of highest
@ pending IRQ. This handler does not support nesting.
@
@ TODO: This does not work.

#if NESTED_INTERRUPTS

IRQHandler:
    SUB     lr, lr, #4             @ construct the return address
    PUSH    {lr}                   @ and push the adjusted lr_IRQ
    MRS     lr, SPSR               @ copy spsr_IRQ to lr
    PUSH    {R0-R4,R12,lr}         @ save AAPCS regs and spsr_IRQ
    
   
    @ Identify source.
    LDR     r1, =ADDR_HIPIR2
    LDR     r2, [r1, #0] 
    LDR     r3, =#PRI_INDEX_MASK    @ TODO: Maybe LDRB then not mask?
    AND     r2, r3                  @ r2 == System interrupt number. 

    
    @ Get nesting level.
    LDR     r1, =ADDR_HINLR2
    LDR     r3, [r1, #0]            @ Current value of HINLR2. 

    @ Push current nesting level.
    LDR     r4, p_IRQPriority       @ IRQ priority stack pointer.
    STMFA   r4!, {r3}
    STR     r4, p_IRQPriority
    
    @ Update nesting level.
    BIC     r3, #OVERRIDE_MASK      @ Clear OVERRIDE bit.
    STR     r3, [r1, #0]            @ Write HINLR2.
    LDR     r3, [r1, #0]            @ Read current nesting level

    @ Set nesting level.
    ORR     r3, r3, #OVERRIDE_MASK  @ Set override bit,
    STR     r3, [r1, #0]            @ writing current priority as nesting level.

    
    @ Get C ISR address.
    LDR     r1, =ADDR_HIPVR2        @ R0 points to address of HIPVR2
    LDR     r0, [r1]                @ R0 contains address of ISR

    @ Clear source.
    LDR     r1, =ADDR_SICR         @ r1 == SICR.
    STR     r2, [r1, #0]

    @ Switch mode.
    MSR     CPSR_c, #0x9F          @ switch to SYS mode, IRQ is
                                   @ still disabled. USR mode
                                   @ registers are now current.
    @ Align stack.
    AND     R1, sp, #4             @ test alignment of the stack
    SUB     sp, sp, R1             @ remove any misalignment (0 or 4)
    PUSH    {R1,lr}                @ store the adjustment and lr_USR

    @ Enable IRQ,
    MSR     CPSR_c, #0x1F          @ enable IRQ
    
    @ Call C ISR.
    MOV     lr, pc
    LDR     pc, [r0]
    
    @ Disable IRQ.
    MSR     CPSR_c, #0x9F          @ disable IRQ, remain in SYS mode

    @ Restore stack.
    POP     {R1,lr}                @ restore stack adjustment and lr_USR
    ADD     sp, sp, R1             @ add the stack adjustment (0 or 4)

    @ Switch mode.
    MSR     CPSR_c, #0x92          @ switch to IRQ mode and keep IRQ
                                   @ disabled. FIQ is still enabled.
    @ Pop previous nesting level.
    LDR     r4, p_IRQPriority      @ IRQ priority stack pointer.
    LDMFA   r4!, {r3}
    STR     r4, p_IRQPriority

    @ Restore previous nesting level.
    LDR     r1, =ADDR_HINLR2
    ORR     r3, #OVERRIDE_MASK     @ Set override bit,
    STR     r3, [r1, #0]           @ Write previous priority as nesting level.



    POP     {R0-R4,R12,lr}         @ restore registers and
    MSR     SPSR_cxsf, lr          @ spsr_IRQ
    LDM     sp!, {pc}^             @ return from IRQ.


IRQPriorityStack:
    .fill   100, 4, 0x100

p_IRQPriority:
    .word   IRQPriorityStack

#else

IRQHandler:
        STMFD    r13!, {r0-r3, r12, r14}  @ Save context in IRQ stack
        LDR      r0, =ADDR_HIPVR2         @ R0 points to address of HIPVR2
        LDR      r1, [r0]                 @ R1 contains address of ISR
        ADD      r14, pc, #0              @ Save return address in LR 
        LDR      pc, [r1]                 @ Go to ISR (still in IRQ mode)
        LDMFD    r13!, {r0-r3, r12, r14}  @ Restore registers from IRQ stack
        SUBS     pc, r14, #0x4            @ Return to program before IRQ

#endif  // NESTED_INTERRUPTS


@***********************************************************************
@*                  Function Definition of FIQ Handler
@*********************************************************************** 
@
@ The FIQ Handler jumps to the ISR of the highest priority pending FIQ. The
@ address is taken from HIPVR1, which contains the ISR address of the highest
@ pending FIQ. This handler does not support nesting
@
FIQHandler:
@
@ Save the required context in FIQ stack. 
@
        STMFD    r13!, {r0-r3, r12, r14}  @ Save context in FIQ stack
        LDR      r0, =ADDR_HIPVR1         @ R0 points to address of HIPVR1
        LDR      r1, [r0]                 @ R1 contains address of ISR
        ADD      r14, pc, #0              @ Save return address in LR 
        LDR      pc, [r1]                 @ Go to ISR (still in FIQ mode)
        LDMFD    r13!, {r0-r3, r12, r14}  @ Restore registers from FIQ stack
        SUBS     pc, r14, #0x4            @ Return to program state before FIQ 
        
@***********************************************************************
@*             Function Definition of Abort/Undef Handler
@***********************************************************************
@
@ The Abort handler goes to the C handler of abort mode. Note that the undefined
@ instruction is not handled separately.
@ if nothing is done in the abort mode, the execution enters infinite loop.
@
AbortHandler:
UndefInstHandler:
@
@ Disable all the interrupts
@
        MRS     r0, cpsr                  @ Read from CPSR
        ORR     r0, r0, #0xC0             @ Clear the IRQ and FIQ bits    
        MSR     cpsr, r0                  @ Write to CPSR
        ADD     r14, pc, #0               @ Store the return address
        LDR     pc, =CPUAbortHandler      @ Go to C handler
@
@ Go to infinite loop if returned from C handler
@
loop0:
        B       loop0                      

@***********************************************************************
@
@ End of the file
@
    .end
    
    
    
