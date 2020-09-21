// Definitions for the number of the interrupt vector to be
// used when defining an interrupt response routine

#ifndef S12eVEC_H
#define S12eVEC_H

#define _Vec_Reserved63             63
#define _Vec_Reserved62             62
#define _Vec_Reserved61             61
#define _Vec_Reserved60             60
#define _Vec_pwmesdn                59
#define _Vec_vreglvi                58
#define _Vec_pmffault3              57
#define _Vec_pmffault2              56
#define _Vec_pmffault1              55
#define _Vec_pmffault0              54
#define _Vec_pmfgcr                 53
#define _Vec_pmfgbr                 52
#define _Vec_pmfgar                 51
// Timer 2 Pulse accumulator input edge
#define _Vec_tim2paie               50
// Timer 2 Pulse accumulator overflow
#define _Vec_tim2paovf              49
// Timer 2 overflow
#define _Vec_tim2ovf                48
// Standard Timer 2 channel 7
#define _Vec_tim2ch7                47
// Standard Timer 2 channel 6
#define _Vec_tim2ch6                46
// Standard Timer 2 channel 5
#define _Vec_tim2ch5                45
// Standard Timer 2 channel 4
#define _Vec_tim2ch4                44
#define _Vec_Reserved43             43
// Timer 1 Pulse accumulator input edge
#define _Vec_tim1paie               42
// Timer 1 Pulse accumulator overflow
#define _Vec_tim1paovf              41
// Timer 1 overflow
#define _Vec_tim1ovf                40
// Standard Timer 1 channel 7
#define _Vec_tim1ch7                39
// Standard Timer 1 channel 6
#define _Vec_tim1ch6                38
// Standard Timer 1 channel 5
#define _Vec_tim1ch5                37
// Standard Timer 1 channel 4
#define _Vec_tim1ch4                36
#define _Vec_flash                  35
#define _Vec_Reserved34             34
#define _Vec_Reserved33             33
#define _Vec_Reserved32             32
#define _Vec_iic                    31
#define _Vec_Reserved30             30
#define _Vec_crgscm                 29
#define _Vec_crgplllck              28
#define _Vec_Reserved27             27
#define _Vec_Reserved26             26
#define _Vec_Reserved25             25
#define _Vec_portAD                 24
#define _Vec_atd0                   23
// SCI 2
#define _Vec_sci2                   22
// SCI 1
#define _Vec_sci1                   21
// SCI 0
#define _Vec_sci0                   20
// SPI
#define _Vec_spi                    19
// Timer 0 Pulse accumulator input edge
#define _Vec_tim0paie               18
// Timer 0 Pulse accumulator overflow
#define _Vec_tim0paovf              17
// Timer 0 overflow
#define _Vec_tim0ovf                16
// Standard Timer 0 channel 7
#define _Vec_tim0ch7                15
// Standard Timer 0 channel 6
#define _Vec_tim0ch6                14
// Standard Timer 0 channel 5
#define _Vec_tim0ch5                13
// Standard Timer 0 channel 4
#define _Vec_tim0ch4                12
#define _Vec_Reserved11             11
#define _Vec_Reserved10             10
#define _Vec_Reserved9              9
#define _Vec_Reserved8              8
// Real Time Interrupt
#define _Vec_rti                    7
// IRQ
#define _Vec_irq                    6
// XIRQ
#define _Vec_xirq                   5
// Software Interrupt
#define _Vec_swi                    4
#define _Vec_trap                   3
#define _Vec_cop                    2
#define _Vec_clkmon                 1
#define _Vec_reset                  0

#endif
