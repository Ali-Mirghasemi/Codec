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

#define CODEC_VER_MAJOR    0
#define CODEC_VER_MINOR    1
#define CODEC_VER_FIX      0

/************************************************************************/
/*                            Configuration                             */
/************************************************************************/
/**
 * @brief enable user arguments into codec
 */
#define CODEC_ARGS                          1
/**
 * @brief enable auto padding for fix layer sizes
 */
#define CODEC_LAYER_PADDING                 1
/**
 * @brief select padding mode
 */
#if CODEC_LAYER_PADDING
    #define CODEC_LAYER_PADDING_IGNORE      0
    #define CODEC_LAYER_PADDING_VALUE       0xFF

    #define CODEC_LAYER_PADDING_MODE        CODEC_LAYER_PADDING_IGNORE

#endif
/**
 * @brief enable encode error callback
 */
#define CODEC_ENCODE_ERROR                  1
/**
 * @brief enable decode error callback
 */
#define CODEC_DECODE_ERROR                  1
/**
 * @brief choose what type use for codec frame, default is void
 */
typedef void Codec_Frame;
/**
 * @brief choose what type use for codec error
 */
typedef uint32_t Codec_Error;
/**
 * @brief choose what type use for codec layer index
 */
typedef uint16_t Codec_LayerIndex;

/************************************************************************/
#if !STREAM_WRITE_LIMIT
    #error "Codec Library use STREAM_LIMIT_WRITE, you must enable it"
#endif

#if !STREAM_READ_LIMIT
    #error "Codec Library use STREAM_LIMIT_READ, you must enable it"
#endif

#define __CODEC_VER_STR(major, minor, fix)     #major "." #minor "." #fix
#define _CODEC_VER_STR(major, minor, fix)      __CODEC_VER_STR(major, minor, fix)
/**
 * @brief show codec version in string format
 */
#define CODEC_VER_STR                          _CODEC_VER_STR(CODEC_VER_MAJOR, CODEC_VER_MINOR, CODEC_VER_FIX)
/**
 * @brief show codec version in integer format, ex: 0.2.0 -> 200
 */
#define CODEC_VER                              ((CODEC_VER_MAJOR * 10000UL) + (CODEC_VER_MINOR * 100UL) + (CODEC_VER_FIX))
/**
 * @brief return ok of parse was successful
 */
#define CODEC_OK                ((Codec_Error) 0)
/**
 * @brief return null when it's last layer
 */
#define CODEC_LAYER_NULL        ((Codec_LayerImpl*) 0)

/* Pre-define data types */
struct __Codec;
typedef struct __Codec Codec;
struct __Codec_LayerImpl;
typedef struct __Codec_LayerImpl Codec_LayerImpl;
/**
 * @brief this function parse layer from input stream
 */
typedef Codec_Error (*Codec_ParseFn)(Codec* codec, Codec_Frame* frame, IStream* stream);
/**
 * @brief this function write layer into output stream
 */
typedef Codec_Error (*Codec_WriteFn)(Codec* codec, Codec_Frame* frame, OStream* stream);
/**
 * @brief this function return size of layer in bytes
 */
typedef Stream_LenType (*Codec_GetLenFn)(Codec* codec, Codec_Frame* frame);
/**
 * @brief this function return next or upper layer of packet, return null if it's last layer
 */
typedef Codec_LayerImpl* (*Codec_GetUpperLayerFn)(Codec* codec, Codec_Frame* frame);
/**
 * @brief this function is called when codec decode a frame from stream
 */
typedef void (*Codec_OnFrameFn)(Codec* codec, Codec_Frame* frame);
/**
 * @brief this function is called when encode/decode error occurred
 */
typedef void (*Codec_OnErrorFn)(Codec* codec, Codec_Frame* frame, Codec_LayerImpl* layer, Codec_Error error);
/**
 * @brief codec encode modes
 */
typedef enum {
  Codec_EncodeMode_Normal       = 0x00,         /**< normal encode mode, just write to stream */
  Codec_EncodeMode_Flush        = 0x01,         /**< flush stream after encode done */
  Codec_EncodeMode_FlushLayer   = 0x03,         /**< flush each layer */
} Codec_EncodeMode;
/**
 * @brief hold layer implementation
 */
struct __Codec_LayerImpl {
    Codec_ParseFn           parse;
    Codec_WriteFn           write;
    Codec_GetLenFn          getLayerLen;
    Codec_GetUpperLayerFn   getUpperLayer;
};
/**
 * @brief hold codec parameters
 */
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

#if CODEC_ENCODE_ERROR || CODEC_DECODE_ERROR
    void Codec_onError(Codec* codec, Codec_OnErrorFn fn);
#endif 

#if CODEC_ARGS
    void  Codec_setArgs(Codec* codec, void* args);
    void* Codec_getArgs(Codec* codec);
#endif

void Codec_decode(Codec* codec, IStream* stream);
Codec_Error Codec_encode(Codec* codec, Codec_Frame* frame, OStream* stream, Codec_EncodeMode mode);


#ifdef __cplusplus
};
#endif

#endif /* _CODEC_H_ */
