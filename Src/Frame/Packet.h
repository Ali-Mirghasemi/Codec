/**
 * @file Packet.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief this library implement packet frame consist of three layers (HEADER + DATA + FOOTER)
 *          +------------------+---------------+------------------+
 * Packet:  | HEADER (8x Byte) | DATA (N Byte) | FOOTER (4x Byte) |
 *          +------------------+---------------+------------------+
 *          +----------------------+-----------------------+-----------------------+
 * Header:  | First Sign (2x Byte) | Packet Size (4x Byte) | Second Sign (2x Byte) |
 *          +----------------------+-----------------------+-----------------------+
 * 
 * @version 0.1
 * @date 2022-07-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _PACKET_H_
#define _PACKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../Codec.h"

/************************************************************************/
/*                            Configuration                             */
/************************************************************************/
/**
 * @brief size of header, do not change this value
 */
#define PACKET_HEADER_SIZE                      8
/**
 * @brief size of footer, do not change this value
 */
#define PACKET_FOOTER_SIZE                      4
/**
 * @brief packet first sign uint16_t
 */
#define PACKET_FIRST_SIGN                       0x33CC
/**
 * @brief packet second sign uint16_t
 */
#define PACKET_SECOND_SIGN                      0x55AA
/**
 * @brief packet footer sign uint32_t
 */
#define PACKET_FOOTER_SIGN                      0xCC33AA55  
/**
 * @brief set packet byte order
 */
#define PACKET_BYTE_ORDER                       ByteOrder_BigEndian
/**
 * @brief set packet max size
 */
#define PACKET_MAX_SIZE                         0xFFFFFFFF

/************************************************************************/

typedef enum {
    Packet_Error_FirstSign          = 1,
    Packet_Error_SecondSign         = 2,
    Packet_Error_PacketSize         = 3,
    Packet_Error_FooterSign         = 4,
    Packet_Error_Data               = 5,
    Packet_Error_DataPtr            = 6,
} Packet_Error;

typedef struct {
    uint8_t*        Data;
    uint32_t        Len;
    uint32_t        Size;
} Packet;

void Packet_init(Packet* frame, uint8_t* data, uint32_t size);
uint32_t Packet_len(Packet* frame);
Codec_LayerImpl* Packet_baseLayer(void);

Stream_LenType Packet_sync(Codec* codec, IStream* stream);

#ifdef __cplusplus
};
#endif

#endif /* _PACKET_H_ */

