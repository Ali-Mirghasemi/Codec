/**
 * @file Codec.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief this library help you for encode/decode frames, it's based on stream library
 * @version 0.2
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
#define CODEC_VER_MINOR    3
#define CODEC_VER_FIX      0

#include <stdint.h>

/************************************************************************/
/*                            Configuration                             */
/************************************************************************/
/**
 * @brief enable user arguments into codec
 */
#define CODEC_ARGS                              1
/**
 * @brief enable encode feature
 */
#define CODEC_ENCODE                            1
/**
 * @brief enable decode feature
 */
#define CODEC_DECODE                            1

/* Codec Encode Options */
#if CODEC_ENCODE
    /**
     * @brief enable encode on raw buffer
     */
    #define CODEC_ENCODE_ON_BUFFER              1
    /**
     * @brief enable encode async feature,
     * this feature allow you to encode on stream with smaller buffer size
     */
    #define CODEC_ENCODE_ASYNC                  1
    /**
     * @brief enable queue for encode async feature, !*this feature is not working yet*!
     */
    #define CODEC_ENCODE_QUEUE                  1
    /**
     * @brief enable callback feature for when encode completed
     */
    #define CODEC_ENCODE_CALLBACK               1
    /**
     * @brief enable encode error callback
     */
    #define CODEC_ENCODE_ERROR                  1
    /**
     * @brief enable encode padding for keep layer size fixed
     */
    #define CODEC_ENCODE_PADDING                1
    /* Encode Padding Options */
    #if CODEC_ENCODE_PADDING
        #define CODEC_ENCODE_PADDING_IGNORE     -1
        #define CODEC_ENCODE_PADDING_VALUE      0x00

        #define CODEC_ENCODE_PADDING_MODE       CODEC_LAYER_PADDING_IGNORE
    #endif // CODEC_ENCODE_PADDING
#endif // CODEC_ENCODE

/* Codec Decode Options */
#if CODEC_DECODE
    /**
     * @brief enable encode on raw buffer
     */
    #define CODEC_DECODE_ON_BUFFER              1
    /**
     * @brief enable decode async feature,
     * this feature allow you to encode on stream with smaller buffer size
     */
    #define CODEC_DECODE_ASYNC                  1
    /**
     * @brief enable queue for decode async feature, !*this feature is not working yet*!
     */
    #define CODEC_DECODE_QUEUE                  1
    /**
     * @brief enable sync options for decode
     */
    #define CODEC_DECODE_SYNC                   1
    /**
     * @brief enable callback feature for when decode completed
     */
    #define CODEC_DECODE_CALLBACK               1
    /**
     * @brief enable decode error callback
     */
    #define CODEC_DECODE_ERROR                  1
    /**
     * @brief enable decode padding for keep layer size fixed
     */
    #define CODEC_DECODE_PADDING                1
    /**
     * @brief if stream has bytes keep decoding until end of stream
     */
    #define CODEC_DECODE_CONTINUOUS             1
#endif // CODEC_DECODE
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
#if CODEC_ENCODE
    #include "OutputStream.h"

#if !OSTREAM_LOCK
    #error "Codec Library use OSTREAM_LOCK, you must enable it"
#endif
#endif // CODEC_ENCODE

#if CODEC_DECODE
    #include "InputStream.h"

#if !ISTREAM_LOCK
    #error "Codec Library use ISTREAM_LOCK, you must enable it"
#endif
#endif // CODEC_DECODE


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
struct __Codec_EncodeQueue;
typedef struct __Codec_EncodeQueue Codec_EncodeQueue;
struct __Codec_DecodeQueue;
typedef struct __Codec_DecodeQueue Codec_DecodeQueue;
#if CODEC_DECODE
/**
 * @brief this function parse layer from input stream
 */
typedef Codec_Error (*Codec_ParseFn)(Codec* codec, Codec_Frame* frame, IStream* stream);
#endif // CODEC_DECODE
#if CODEC_ENCODE
/**
 * @brief this function write layer into output stream
 */
typedef Codec_Error (*Codec_WriteFn)(Codec* codec, Codec_Frame* frame, OStream* stream);
#endif // CODEC_ENCODE
/**
 * @brief this function return size of layer in bytes
 */
typedef Stream_LenType (*Codec_GetLenFn)(Codec* codec, Codec_Frame* frame);
/**
 * @brief this function return next or upper layer of packet, return null if it's last layer
 */
typedef Codec_LayerImpl* (*Codec_GetUpperLayerFn)(Codec* codec, Codec_Frame* frame);
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
typedef Stream_LenType (*Codec_SyncFn)(Codec* codec, IStream* stream);
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
};

void Codec_init(Codec* codec, Codec_LayerImpl* baseLayer);
Stream_LenType Codec_frameSize(Codec* codec, Codec_Frame* frame);

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

    Codec_Status Codec_decodeFrame(Codec* codec, Codec_Frame* frame, IStream* stream);

#if CODEC_DECODE_ASYNC
    void Codec_beginDecode(Codec* codec, Codec_Frame* frame);
    void Codec_decode(Codec* codec, IStream* stream);
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

    Codec_Status Codec_encodeFrame(Codec* codec, Codec_Frame* frame, OStream* stream, Codec_EncodeMode mode);

#if CODEC_ENCODE_ASYNC
    void Codec_encodeMode(Codec* codec, Codec_EncodeMode mode);
    void Codec_beginEncode(Codec* codec, Codec_Frame* frame, Codec_EncodeMode mode);
    void Codec_encode(Codec* codec, OStream* stream);
#endif

#endif


#ifdef __cplusplus
};
#endif

#endif /* _CODEC_H_ */
