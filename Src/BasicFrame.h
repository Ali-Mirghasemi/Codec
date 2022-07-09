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

#include "Codec.h"

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


Codec_Error      BasicFrame_Header_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
Codec_Error      BasicFrame_Header_write(Codec* codec, Codec_Frame* frame, OStream* stream);
Stream_LenType   BasicFrame_Header_getLen(Codec* codec, Codec_Frame* frame);
Codec_LayerImpl* BasicFrame_Header_getUpperLayer(Codec* codec, Codec_Frame* frame);

Codec_Error      BasicFrame_Data_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
Codec_Error      BasicFrame_Data_write(Codec* codec, Codec_Frame* frame, OStream* stream);
Stream_LenType   BasicFrame_Data_getLen(Codec* codec, Codec_Frame* frame);
Codec_LayerImpl* BasicFrame_Data_getUpperLayer(Codec* codec, Codec_Frame* frame);

static const Codec_LayerImpl BASIC_FRAME_HEADER_IMPL = {
    BasicFrame_Header_parse,
    BasicFrame_Header_write,
    BasicFrame_Header_getLen,
    BasicFrame_Header_getUpperLayer,
};

static const Codec_LayerImpl BASIC_FRAME_DATA_IMPL = {
    BasicFrame_Data_parse,
    BasicFrame_Data_write,
    BasicFrame_Data_getLen,
    BasicFrame_Data_getUpperLayer,
};

#define BASIC_FRAME_IMPL        BASIC_FRAME_HEADER_IMPL

void BasicFrame_init(BasicFrame* frame, uint8_t* data, uint32_t size);

#ifdef __cplusplus
};
#endif

#endif /* _BASIC_FRAME_H_ */

