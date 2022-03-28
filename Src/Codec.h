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

#ifdef __cplusplus
extern "C" {
#endif

#include "InputStream.h"
#include "OutputStream.h"

/************************************************************************/
/*                            Configuration                             */
/************************************************************************/

#define CODEC_ARGS                          1

#define CODEC_LAYER_PADDING                 1

#if CODEC_LAYER_PADDING
    #define CODEC_LAYER_PADDING_IGNORE      0
    #define CODEC_LAYER_PADDING_VALUE       0xFF

    #define CODEC_LAYER_PADDING_MODE        CODEC_LAYER_PADDING_IGNORE

#endif

#define CODEC_ENCODE_ERROR                  1

#define CODEC_DECODE_ERROR                  1

typedef void Codec_Frame;

typedef uint32_t Codec_Error;

typedef uint16_t Codec_LayerIndex;

/************************************************************************/

#define CODEC_OK                ((Codec_Error) 0)

#define CODEC_LAYER_NULL        ((Codec_LayerImpl*) 0)

struct __Codec;
typedef struct __Codec Codec;

struct __Codec_LayerImpl;
typedef struct __Codec_LayerImpl Codec_LayerImpl;

typedef Codec_Error (*Codec_ParseFn)(Codec* codec, IStream* stream, Codec_Frame* frame);
typedef Codec_Error (*Codec_WriteFn)(Codec* codec, OStream* stream, Codec_Frame* frame);
typedef Stream_LenType (*Codec_GetLenFn)(Codec* codec, Codec_Frame* frame);
typedef Codec_LayerImpl* (*Codec_GetUpperLayerFn)(Codec* codec, Codec_Frame* frame);
typedef void (*Codec_OnFrameFn)(Codec* codec, Codec_Frame* frame);
typedef void (*Codec_OnErrorFn)(Codec* codec, Codec_Frame* frame, Codec_LayerImpl* layer, Codec_Error error);

struct __Codec_LayerImpl {
    Codec_ParseFn           parse;
    Codec_WriteFn           write;
    Codec_GetLenFn          getLayerLen;
    Codec_GetUpperLayerFn   getUpperLayer;
};

struct __Codec {
#if CODEC_ARGS
    void*                   Args;
#endif
    Codec_LayerImpl*        BaseLayer;
    Codec_LayerImpl*        CurrentLayer;
    Codec_Frame*            Frame;
    Codec_OnFrameFn         onFrame;
    Codec_OnErrorFn         onError;
};

void Codec_init(Codec* codec, Codec_LayerImpl* baseLayer, Codec_Frame* frame);
void Codec_onFrame(Codec* codec, Codec_OnFrameFn fn);
void Codec_onError(Codec* codec, Codec_OnErrorFn fn);

#if CODEC_ARGS
    void  Codec_setArgs(Codec* codec, void* args);
    void* Codec_getArgs(Codec* codec);
#endif

void Codec_decode(Codec* codec, IStream* stream);
Codec_Error Codec_encode(Codec* codec, OStream* stream, Codec_Frame* frame);
Codec_Error Codec_encodeFlush(Codec* codec, OStream* stream, Codec_Frame* frame);


#ifdef __cplusplus
};
#endif

#endif /* _CODEC_H_ */
