/**
 * @file Codec.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief this library help you for encode/decode frames, it's based on stream library
 * @version 0.1
 * @date 2021-09-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _CODEC_H_
#define _CODEC_H_

#if __cplusplus
extern "C" {
#endif

#include "InputStream.h"
#include "OutputStream.h"

/************************************************************************/
/*                            Configuration                             */
/************************************************************************/

typedef void Codec_Frame;

/************************************************************************/

struct __Codec;
typedef struct __Codec Codec;

typedef enum {
    Codec_Ok                = 0x00,
    Codec_Error             = 0x01,
    Codec_NoHeaderImpl      = 0x02,
    Codec_NoDataImpl        = 0x03,
    Codec_NoFooterImpl      = 0x04,
    Codec_NoFrame           = 0x05,
    Codec_InProcess         = 0x10,
    Codec_ProcessDone       = 0x11,
} Codec_Result;

typedef Stream_LenType (*Codec_GetLenFn)(Codec* codec, Codec_Frame* frame);
typedef Codec_Result (*Codec_ParseFn)(Codec* codec, Codec_Frame* frame, IStream* stream);
typedef Codec_Result (*Codec_WriteFn)(Codec* codec, Codec_Frame* frame, OStream* stream);
typedef Stream_LenType (*Codec_FindFn)(Codec* codec, IStream* stream);
typedef void (*Codec_OnFrameFn)(Codec* codec, Codec_Frame* frame);

typedef enum {
    Codec_State_Header      = 0,
    Codec_State_Data        = 1,
    Codec_State_Footer      = 2,
} Codec_State;

typedef struct {
    Codec_GetLenFn          getHeaderLen;
    Codec_GetLenFn          getPacketSize;
    Codec_FindFn            find;
    Codec_ParseFn           parse;
    Codec_WriteFn           write;
} Codec_HeaderImpl;

typedef struct {
    Codec_ParseFn           parse;
    Codec_WriteFn           write;
} Codec_DataImpl;

typedef struct {
    Codec_GetLenFn          getFooterLen;
    Codec_ParseFn           parse;
    Codec_WriteFn           write;
} Codec_FooterImpl;

struct __Codec {
    Codec_HeaderImpl*       HeaderImpl;
    Codec_DataImpl*         DataImpl;
    Codec_FooterImpl*       FooterImpl;
    Codec_Frame*            Frame;
    Codec_OnFrameFn         onFrame;
    Codec_State             State;
};

void Codec_init(Codec* codec, Codec_HeaderImpl* header, Codec_DataImpl* data, Codec_FooterImpl* footer, Codec_Frame* frame);

void Codec_decode(Codec* codec, IStream* stream);
Codec_Result Codec_encode(Codec* codec, OStream* stream, Codec_Frame* frame);
Codec_Result Codec_encodeFlush(Codec* codec, OStream* stream, Codec_Frame* frame, uint8_t flushPart);


#if __cplusplus
};
#endif

#endif /* _CODEC_H_ */
