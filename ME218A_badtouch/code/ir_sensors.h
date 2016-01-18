/* ir_sensors.h
 * ============
 * Module to return sensor value. Coordinates IR LED and PT.
 */
 
#ifndef __IR_SENSORS__
#define __IR_SENSORS__

//Sensor/pin Constants  
#define RR_SENSOR 0	 
#define LL_SENSOR 1
#define LB_SENSOR 2	 
#define LR_SENSOR 3
#define LF_SENSOR 4	 
#define RL_SENSOR 5
#define RF_SENSOR 6
#define RB_SENSOR 7

//Sensor result constants
#define UNCOVERED 0
#define COVERED   1

//Interface

/* IrSensorsInit
 * =============
 * Initializes the ports for use with the IR
 * emitting/detecting.
 */
void IrSensorsInit(void);

/* GetSensorValue
 * ==============
 * Gives whether the sensor is covered or uncovered.
 *
 * Args:
 *   sensor: The sensor to read. Easiest to use
 *           constants in header file.
 *
 * Returns:
 *   Constant indicating sensor status.
 */
char GetSensorValue(char sensor);

#endif