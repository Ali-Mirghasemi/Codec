/**
 * @file Codec.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief this library help you for encode/decode frames, it's based on stream library
 * @version 0.4
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

#define CODEC_VER_MAJOR    0
#define CODEC_VER_MINOR    6
#define CODEC_VER_FIX      0

#include "CodecConfig.h"

#if CODEC_ENCODE
    #include "OutputStream.h"

#if !STREAM_WRITE_LOCK
    #error "Codec Library use STREAM_WRITE_LOCK, you must enable it"
#endif
#endif // CODEC_ENCODE

#if CODEC_DECODE
    #include "InputStream.h"

#if !STREAM_READ_LOCK
    #error "Codec Library use STREAM_READ_LOCK, you must enable it"
#endif
#endif // CODEC_DECODE

#if CODEC_SUPPORT_MACRO
    #include "CodecMacro.h"
#endif // CODEC_SUPPORT_MACRO

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
 * @brief return base stream errors
 */
#define CODEC_ERROR_STREAM      ((Codec_Error) 0x1000)
/**
 * @brief return null when it's last layer
 */
#define CODEC_LAYER_NULL        ((void*) 0)

/* Pre-define data types */
struct __Codec;
typedef struct __Codec Codec;
struct __Codec_LayerImpl;
typedef struct __Codec_LayerImpl Codec_LayerImpl;
struct __Codec_EncodeQueue;
typedef struct __Codec_EncodeQueue Codec_EncodeQueue;
struct __Codec_DecodeQueue;
typedef struct __Codec_DecodeQueue Codec_DecodeQueue;

/**
 * @brief codec phase
 */
typedef enum {
    Codec_Phase_Encode,
    Codec_Phase_Decode,
} Codec_Phase;

#if CODEC_DECODE
/**
 * @brief this function parse layer from input stream
 */
typedef Codec_Error (*Codec_ParseFn)(Codec* codec, Codec_Frame* frame, StreamIn* stream);
#endif // CODEC_DECODE
#if CODEC_ENCODE
/**
 * @brief this function write layer into output stream
 */
typedef Codec_Error (*Codec_WriteFn)(Codec* codec, Codec_Frame* frame, StreamOut* stream);
#endif // CODEC_ENCODE
/**
 * @brief this function return size of layer in bytes
 */
typedef Stream_LenType (*Codec_GetLenFn)(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
/**
 * @brief this function return next or upper layer of packet, return null if it's last layer
 */
typedef Codec_LayerImpl* (*Codec_NextLayerFn)(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
/**
 * @brief this function is called when codec decode/encode a frame completed
 */
typedef void (*Codec_OnFrameFn)(Codec* codec, Codec_Frame* frame);
/**
 * @brief this function is called when encode/decode error occurred
 */
typedef void (*Codec_OnErrorFn)(Codec* codec, Codec_Frame* frame, Codec_LayerImpl* layer, Codec_Error error);
/**
 * @brief this function used to sync frame with stream in decoding
 */
typedef Stream_LenType (*Codec_SyncFn)(Codec* codec, StreamIn* stream);
/**
 * @brief decode/encode states
 */
typedef enum {
    Codec_Status_Done           = 0,
    Codec_Status_InProgress     = 1,
    Codec_Status_Error          = 2,
    Codec_Status_Pending        = 3,
} Codec_Status;
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
#if CODEC_DECODE
    Codec_ParseFn           parse;
#endif
#if CODEC_ENCODE
    Codec_WriteFn           write;
#endif
    Codec_GetLenFn          getLen;
    Codec_NextLayerFn       nextLayer;
};
/**
 * @brief hold codec parameters
 */
struct __Codec {
#if CODEC_ARGS
    void*                   Args;
#endif
    Codec_LayerImpl*        BaseLayer;
#if CODEC_DECODE
#if CODEC_DECODE_ASYNC
    Codec_LayerImpl*        RxLayer;
    Codec_Frame*            RxFrame;
#endif
#if CODEC_DECODE_CALLBACK
    Codec_OnFrameFn         onDecode;
#endif
#if CODEC_DECODE_ERROR
    Codec_OnErrorFn         onDecodeError;
#endif
#if CODEC_DECODE_SYNC
    Codec_SyncFn            sync;
#endif
#endif // CODEC_DECODE
#if CODEC_ENCODE
#if CODEC_ENCODE_ASYNC
    Codec_LayerImpl*        TxLayer;
    Codec_Frame*            TxFrame;
    Codec_EncodeMode        EncodeMode;
#endif
#if CODEC_ENCODE_CALLBACK
    Codec_OnFrameFn         onEncode;
#endif
#if CODEC_ENCODE_ERROR
    Codec_OnErrorFn         onEncodeError;
#endif
#endif // CODEC_ENCODE
    uint8_t                 FreeStream      : 1;
    uint8_t                 DecodeAll       : 1;
    uint8_t                 Reserved        : 6;
};

void Codec_init(Codec* codec, Codec_LayerImpl* baseLayer);
Stream_LenType Codec_frameSize(Codec* codec, Codec_Frame* frame, Codec_Phase phase);

#define   Codec_deinit(CODEC)                                           memset((CODEC), 0, sizeof(Codec))

#if CODEC_ARGS
    void  Codec_setArgs(Codec* codec, void* args);
    void* Codec_getArgs(Codec* codec);
#endif

/* Decode Functions */
#if CODEC_DECODE

#if CODEC_DECODE_CALLBACK
    void Codec_onDecode(Codec* codec, Codec_OnFrameFn fn);
#endif

#if CODEC_DECODE_ERROR
    void Codec_onDecodeError(Codec* codec, Codec_OnErrorFn fn);
#endif

#if CODEC_DECODE_SYNC
    void Codec_setDecodeSync(Codec* codec, Codec_SyncFn fn);
#endif

#if CODEC_DECODE_ON_BUFFER
    Codec_Status Codec_decodeBuffer(Codec* codec, Codec_Frame* frame, uint8_t* buffer, Stream_LenType size);
#endif

    Codec_Status Codec_decodeFrame(Codec* codec, Codec_Frame* frame, StreamIn* stream);

#if CODEC_DECODE_ASYNC
    void Codec_beginDecode(Codec* codec, Codec_Frame* frame);
    void Codec_decode(Codec* codec, StreamIn* stream);
#endif

#endif

#if CODEC_ENCODE

#if CODEC_ENCODE_CALLBACK
    void Codec_onEncode(Codec* codec, Codec_OnFrameFn fn);
#endif

#if CODEC_ENCODE_ERROR
    void Codec_onEncodeError(Codec* codec, Codec_OnErrorFn fn);
#endif

#if CODEC_ENCODE_ON_BUFFER
    Codec_Status Codec_encodeBuffer(Codec* codec, Codec_Frame* frame, uint8_t* buffer, Stream_LenType size);
#endif

    Codec_Status Codec_encodeFrame(Codec* codec, Codec_Frame* frame, StreamOut* stream, Codec_EncodeMode mode);

#if CODEC_ENCODE_ASYNC
    void Codec_encodeMode(Codec* codec, Codec_EncodeMode mode);
    void Codec_beginEncode(Codec* codec, Codec_Frame* frame, Codec_EncodeMode mode);
    void Codec_encode(Codec* codec, StreamOut* stream);
#endif

#endif

void Codec_setFreeStream(Codec* codec, uint8_t enabled);
void Codec_setDecodeAll(Codec* codec, uint8_t enabled);

// ----------------------------------- Macros -----------------------------------
#if CODEC_DECODE && CODEC_ENCODE
    #define CODEC_LAYER_IMPL(parse, write, getLen, nextLayer) { .parse = parse, .write = write, .getLen = getLen, .nextLayer = nextLayer }
#elif CODEC_DECODE
    #define CODEC_LAYER_IMPL(parse, write, getLen, nextLayer) { .parse = parse, .getLen = getLen, .nextLayer = nextLayer }
#elif CODEC_ENCODE
    #define CODEC_LAYER_IMPL(parse, write, getLen, nextLayer) { .write = write, .getLen = getLen, .nextLayer = nextLayer }
#endif

#if CODEC_SUPPORT_NEXT_LAYER_NULL
    #define Codec_endLayer             CODEC_LAYER_NULL
#else
    Codec_LayerImpl* Codec_endLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
#endif

#ifdef __cplusplus
};
#endif

#endif /* _CODEC_H_ */
