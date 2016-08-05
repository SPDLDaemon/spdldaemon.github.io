/* 
 * File:   AccelerometerModule.h
 * Author: jordanm1
 *
 * Created on May 15, 2016, 4:34 PM
 */

#ifndef ACCELEROMETERMODULE_H
#define	ACCELEROMETERMODULE_H

void InitAccelerometer( void );
bool Accel_IOC_ISR(void);
void CheckForPair(void);
void IgnorePair(void);

#endif	/* ACCELEROMETERMODULE_H */

