/**
 * \file
 * \brief This is the implementation of the Radio MAC part.
 * \author (c) 2013 Erich Styger, http://mcuoneclipse.com/
 * \note MIT License (http://opensource.org/licenses/mit-license.html), see 'RNet_License.txt'
 *
 * This module encapsulates the Media Access of network stack.
 */

#include "RNetConf.h"
#if PL_HAS_RADIO
#include "RMAC.h"
#include "RPHY.h"
#include "RNWK.h"

static uint8_t RMAC_SeqNr = 0;
static uint8_t RMAC_ExpectedAckSeqNr;

uint8_t RMAC_PutPayload(uint8_t *buf, size_t bufSize, uint8_t payloadSize) {
  RMAC_BUF_TYPE(buf) = RMAC_MSG_TYPE_DATA|RMAC_MSG_TYPE_REQ_ACK;
  RMAC_ExpectedAckSeqNr = RMAC_SeqNr;
  RMAC_BUF_SEQN(buf) = RMAC_SeqNr++;
  return RPHY_PutPayload(buf, bufSize, payloadSize+RMAC_HEADER_SIZE);
}

uint8_t RMAC_OnPacketRx(RPHY_PacketDesc *packet) {
  return RNWK_OnPacketRx(packet); /* pass data packet up the stack */
}

uint8_t RMAC_SendACK(RPHY_PacketDesc *rxPacket, RPHY_PacketDesc *ackPacket) {
  RMAC_BUF_TYPE(ackPacket->data) = RMAC_MSG_TYPE_ACK; /* set type to ack */
  RMAC_BUF_SEQN(ackPacket->data) = RMAC_BUF_SEQN(rxPacket->data);
  /* use same sequence number as in the received package, so no change */
  return RPHY_PutPayload(ackPacket->data, ackPacket->dataSize, RMAC_HEADER_SIZE+RNWK_HEADER_SIZE);
}

RMAC_MsgType RMAC_GetType(uint8_t *buf, size_t bufSize) {
  (void)bufSize; /* not used */
  return RMAC_BUF_TYPE(buf);
}

bool RMAC_IsExpectedACK(uint8_t *buf, size_t bufSize) {
  (void)bufSize; /* not used */
  return RMAC_BUF_SEQN(buf)==RMAC_ExpectedAckSeqNr;
}

void RMAC_SniffPacket(RPHY_PacketDesc *packet, bool isTx) {
  RNWK_SniffPacket(packet, isTx);
}
#if 0
 * typedef enum RMAC_MsgType {
  RMAC_MSG_TYPE_INIT = 0x0,  /* initialization value */
  RMAC_MSG_TYPE_DATA = 0x1, /* flag: data message */
  RMAC_MSG_TYPE_ACK = 0x2,  /* flag: acknowledge message */
  RMAC_MSG_TYPE_CMD = 0x4,  /* flag: command message */
  RMAC_MSG_TYPE_REQ_ACK = 0x80, /* flag: ack requested */
} RMAC_MsgType;
#endif

void RMAC_DecodeType(uint8_t *buf, size_t bufSize, RPHY_PacketDesc *packet) {
  RMAC_MsgType type;
  bool first = TRUE;
  
  type = RMAC_BUF_TYPE(packet->data);
  buf[0] = '\0';
  UTIL1_chcat(buf, bufSize, '(');
  if (type&RMAC_MSG_TYPE_REQ_ACK) {
    UTIL1_strcat(buf, bufSize, (unsigned char*)"RACK");
    first = FALSE;
  }
  if (type&RMAC_MSG_TYPE_DATA) {
    if (!first) {
      UTIL1_chcat(buf, bufSize, '|');
    }
    UTIL1_strcat(buf, bufSize, (unsigned char*)"DATA");
    first = FALSE;
  }
  if (type&RMAC_MSG_TYPE_ACK) {
    if (!first) {
      UTIL1_chcat(buf, bufSize, '|');
    }
    UTIL1_strcat(buf, bufSize, (unsigned char*)"ACK");
    first = FALSE;
  }
  if (type&RMAC_MSG_TYPE_CMD) {
    UTIL1_strcat(buf, bufSize, (unsigned char*)"CMD");
    first = FALSE;
  }
  UTIL1_chcat(buf, bufSize, ')');
}

void RMAC_Deinit(void) {
  /* nothing needed */
}

void RMAC_Init(void) {
  RMAC_SeqNr = 0;
  RMAC_ExpectedAckSeqNr = 0;
}
#endif /* PL_HAS_RADIO */
