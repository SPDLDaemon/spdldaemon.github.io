/* 
 * File:   ProcessDataModule.h
 * Author: wfindlay
 *
 * Created on May 7, 2016, 12:35 PM
 */

#ifndef PROCESSDATAMODULE_H
#define	PROCESSDATAMODULE_H

#include "PACService.h"

#define XBEE_RESPONSE 0x89
#define INCOMING_PACKET 0x81

// Class protocol packet types
#define REQ_PAIR 0x00
#define ENCR_KEY 0x01
#define CTRL 0x02
#define STATUS 0x03

void CopyFrameData(uint8_t *PacketSource, uint8_t PacketSize);
void ProcessData(void);
bool DataReceived(void);
void ConstructPacket(void);
void ConstructEncryptionPacket(void);
void ConstructPairRequestPacket(PACColor_t Color, uint8_t LobbyistNumber);
void ConstructControlPacket(int8_t AccelInput, int8_t SteerInput, bool SpecAction, bool Unpair, bool EBrake);
uint8_t GetStatus(void);
uint8_t *GetPacket2Send(void);
uint8_t GetPacket2SendSize(void);

#endif	/* PROCESSDATAMODULE_H */

