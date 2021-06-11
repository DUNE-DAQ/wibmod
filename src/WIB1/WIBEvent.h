#ifndef WIBEVENT_H_
#define WIBEVENT_H_

#include <stdint.h>

static const int RCE_COLblocks = 2;
static const uint8_t RCE_SOFbyte = 0xBC;   // K28.5
static const int FELIX_COLblocks = 4;
static const uint8_t FELIX_SOFbyte = 0x3C; // K28.1

typedef struct {
  uint8_t S1_ErrC : 4;
  uint8_t S2_ErrC : 4;
  uint8_t Reserved_0;
  uint16_t Checksum_A;
  uint16_t Checksum_B;
  uint16_t Time_Stamp;
  uint16_t Errors;
  uint16_t Reserved_1;
  struct /* Stream[8] */ {
    uint8_t Header : 4;
    uint16_t Channel[8];
  } Stream[8];
} WIBEvent_COLDATA_t;

typedef struct {
  uint8_t StartOfFrame;
  uint8_t Version : 5;
  uint8_t FiberNo : 3;
  uint8_t CrateNo : 5;
  uint8_t SlotNo : 3;
  uint8_t Reserved_0;
  bool    Mismatch;
  bool    OutOfSync;
  uint16_t Reserved_1 : 14;
  uint16_t WIB_Errors;
  uint8_t Z_mode : 1;
  uint64_t Timestamp;
  uint32_t WIB_counter;
  // FELIX: CRC-20   RCE: CRC-32
  uint32_t CRC;
  uint8_t  CRClength; // bits
  int COLDATA_count;
  WIBEvent_COLDATA_t COLDATA[];
} WIBEvent_t;

#endif
