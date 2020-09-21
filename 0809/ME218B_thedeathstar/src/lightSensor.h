/*
 * These defines should eventually be moved to the IR sensor module
 */

// IR sensor names

#define IR_9FL_LR 3
#define IR_9F_SR 4
#define IR_9FR_LR 5
#define IR_9BL_LR 0
#define IR_9B_SR 2
//Note: Swapped Center and right, although this should not work, just testing
#define IR_9BR_LR 1
#define IR_12F 6
#define IR_12B 7

void initLights(void);

// GetIRDutyCycle dummy declaration for compilation
char getIRDutyCycle(char sensor);