/*
 * PWM Module for the E128 Microcontroller
 *
 * This PWM Module allows PWM signals to be sent from pins U0-U5
 * at frequencies ranging from approximately 150 Hz to 20000 Hz.
 * These frequencies come in quantized ranges; at any given time,
 * only two of the ranges are achievable, one through channels 0, 1,
 * 4, and 5 and the other through channels 2 and 3.
 *
 * @author David Hoffert
 */

#ifndef PWM_E128_H
#define PWM_E128_H

/*
 * Module constants
 */
 
// Frequency ranges
#define FROM_150_TO_300 1
#define FROM_400_TO_900 2
#define FROM_1000_TO_2500	3 
#define FROM_3000_TO_7500 4
#define FROM_8000_TO_20000 5

// Frequency values for 150-300 range
#define HZ_150 255
#define HZ_200 203
#define HZ_250 162
#define HZ_300 101

// Frequency values for 400-900 range
#define HZ_400 227
#define HZ_500 182
#define HZ_600 151
#define HZ_700 130
#define HZ_750 121
#define HZ_800 114
#define HZ_900 101

// Frequency values for 1000-2500 range
#define HZ_1000 250
#define HZ_1500 167
#define HZ_2000 125
#define HZ_2500 100

// Frequency values for 3000-7500 range
#define HZ_3000 250
#define HZ_4000 188
#define HZ_5000 150
#define HZ_6000 125
#define HZ_7000 107
#define HZ_7500 100

// Frequency values for 8000-20000 range
#define HZ_8000 250
#define HZ_9000 222
#define HZ_10000 200
#define HZ_11000 182
#define HZ_12000 167
#define HZ_12500 160
#define HZ_13000 154
#define HZ_14000 143
#define HZ_15000 133
#define HZ_16000 125
#define HZ_17000 118
#define HZ_17500 114
#define HZ_18000 111
#define HZ_19000 105
#define HZ_20000 100

/*
 * Initializes the overall PWM subsystem.  Must be run before any
 * individual channel is initialized.
 *
 * @param chan0145range The frequency range available to channels 0,1,4,5
 *                      Must use a predefined module constant for this
 * @param chan23range   The frequency range available to channels 2,3
 *                      Must use a predefined module constant for this
 *
 * @author David Hoffert
 */
void pwm_sys_init(unsigned char chan0145range, unsigned char chan23range);

/*
 * Initializes an individual PWM channel.  Must be run after the
 * system initialization is performed.  Default duty cycle is 0.
 *
 * @param chan  Which channel to initialize, 0-5 (others ignored)
 * @param freq  The initial frequency this channel should run at
 *              Must use a predefined module constant for this
 *
 * @author David Hoffert
 */
void pwm_chan_init(unsigned char chan, unsigned char freq);

/*
 * Disables an individual PWM channel.
 *
 * @param chan Which channel to disable, 0-5 (others ignored)
 *
 * @author David Hoffert
 */
void pwm_chan_disable(unsigned char chan);

/*
 * Changes the duty cycle of an individual PWM channel.
 *
 * @param chan Which channel to modify, 0-5 (others ignored)
 * @param dc The new duty cycle, a number between 0 and 100
 *
 * @author David Hoffert
 */
void pwm_change_dc(unsigned char chan, unsigned char dc);

/*
 * Changes the frequency of an individual PWM channel.
 *
 * @param chan Which channel to modify, 0-5 (others ignored)
 * @param freq The new frequency this channel should run at
 *             Must use a predefined module constant for this
 *
 * @author David Hoffert
 */
void pwm_change_freq(unsigned char chan, unsigned char freq);

#endif