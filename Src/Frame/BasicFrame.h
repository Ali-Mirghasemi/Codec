/**
 * @file BasicFrame.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief this library implement basic frame consist of two layer (HEADER + DATA)
 * +------------------+---------------+
 * | HEADER (4x Byte) | DATA (N Byte) |
 * +------------------+---------------+
 * @version 0.1
 * @date 2021-09-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _BASIC_FRAME_H_
#define _BASIC_FRAME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../Codec.h"

typedef struct {
    uint32_t            PacketSize;
} BasicFrame_Header;

typedef struct {
    uint8_t*            Data;
} BasicFame_Data;

typedef struct {
    BasicFrame_Header  Header;
    BasicFame_Data     Data;
} BasicFrame;

void BasicFrame_init(BasicFrame* frame, uint8_t* data, uint32_t size);
uint32_t BasicFrame_len(BasicFrame* frame);
Codec_LayerImpl* BasicFrame_baseLayer(void);

#ifdef __cplusplus
};
#endif

#endif /* _BASIC_FRAME_H_ */

