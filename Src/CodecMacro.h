/**
 * @file CodecMacro.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief This file contains macros and definitions for codec operations.
 * @version 0.1
 * @date 2025-06-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef _CODEC_MACRO_H_
#define _CODEC_MACRO_H_

#include "Macro.h"

#define CODEC_BEGIN(...)                        Codec_Error err = CODEC_OK; \
                                                MACRO_FOR(MACRO_DUMMY, __VA_ARGS__)

#define CODEC_END()                             (void) err; \
                                                return err

// -------------------------------------- Write APIs -------------------------------------
#define CODEC_WRITE(...)                        MACRO_FN_MAP((CODEC_WRITE_RAW, CODEC_WRITE_TYPE, CODEC_WRITE_VAL, CODEC_WRITE_NONE), __VA_ARGS__)

#define CODEC_WRITE_NONE()

#define CODEC_WRITE_VAL(VAL)                    err = OStream_write(stream, (uint8_t*) &(VAL), sizeof(VAL)); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }

#define CODEC_WRITE_TYPE(TYPE, VAL)             err = OStream_write ##TYPE(stream, (VAL)); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }

#define CODEC_WRITE_RAW(TYPE, VAL, LEN)         err = OStream_write(stream, (uint8_t*) (VAL), (LEN)); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }

// -------------------------------------- Read APIs -------------------------------------
#define CODEC_READ(...)                         MACRO_FN_MAP((CODEC_READ_RAW, CODEC_READ_TYPE, CODEC_READ_VAL, CODEC_READ_NONE), __VA_ARGS__)

#define CODEC_READ_NONE()

#define CODEC_READ_VAL(VAL)                     err = IStream_read(stream, (uint8_t*) &(VAL), sizeof(VAL)); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }                                        

#define CODEC_READ_TYPE(TYPE, VAL)              err = IStream_read ##TYPE ##Safe(stream, (VAL)); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }

#define CODEC_READ_RAW(TYPE, VAL, LEN)          err = IStream_read(stream, (uint8_t*) (VAL), (LEN)); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }                                                                                                                      

// -------------------------------------- Length APIs -------------------------------------
#define CODEC_VALUE_LEN(...)                    MACRO_FN_MAP((CODEC_VALUE_LEN_ARR, CODEC_VALUE_LEN_TYPE, CODEC_VALUE_LEN_VAL, CODEC_VALUE_LEN_NONE), __VA_ARGS__)

#define CODEC_VALUE_LEN_NONE()
#define CODEC_VALUE_LEN_VAL(VAL)                sizeof(VAL)
#define CODEC_VALUE_LEN_TYPE(TYPE, VAL)         sizeof(STREAM_VALUE_TYPE(TYPE))
#define CODEC_VALUE_LEN_ARR(TYPE, VAL, LEN)     sizeof(STREAM_VALUE_TYPE(TYPE)) * (LEN)
    
// ------------------------------------ Implementation Macros ----------------------------------

#define CODEC_IMPL_LEN_0(TYPE)                  
#define CODEC_IMPL_LEN_1(VAL)                   len += sizeof(VAL);
#define CODEC_IMPL_LEN_2(TYPE, VAL)             len += sizeof(STREAM_VALUE_TYPE(TYPE));
#define CODEC_IMPL_LEN_3(TYPE, VAL, LEN)        len += sizeof(STREAM_VALUE_TYPE(TYPE)) * LEN;

#define CODEC_IMPL_ENCODE(NAME, FN_PREFIX, PACKET_TYPE, ...) \
    FN_PREFIX Codec_Error NAME(Codec* codec, Codec_Frame* __frame, StreamOut* stream) { \
        CODEC_BEGIN(PACKET_TYPE* frame = (PACKET_TYPE*) __frame); \
        MACRO_FOR_MAP((CODEC_WRITE_RAW, CODEC_WRITE_TYPE, CODEC_WRITE_VAL, CODEC_WRITE_NONE), __VA_ARGS__); \
        CODEC_END(); \
    }

#define CODEC_IMPL_DECODE(NAME, FN_PREFIX, PACKET_TYPE, ...) \
    FN_PREFIX Codec_Error NAME(Codec* codec, Codec_Frame* __frame, StreamIn* stream) { \
        CODEC_BEGIN(PACKET_TYPE* frame = (PACKET_TYPE*) __frame); \
        MACRO_FOR_MAP((CODEC_READ_RAW, CODEC_READ_TYPE, CODEC_READ_VAL, CODEC_READ_NONE), __VA_ARGS__); \
        CODEC_END(); \
    }

#define CODEC_IMPL_GET_LEN(NAME, FN_PREFIX, PACKET_TYPE, ...) \
    FN_PREFIX Stream_LenType NAME(Codec* codec, Codec_Frame* __frame, Codec_Phase phase) { \
        CODEC_BEGIN(PACKET_TYPE* frame = (PACKET_TYPE*) __frame; , Stream_LenType len = 0); \
        MACRO_FOR_MAP((CODEC_IMPL_LEN_3, CODEC_IMPL_LEN_2, CODEC_IMPL_LEN_1, CODEC_IMPL_LEN_0), __VA_ARGS__) \
        CODEC_END(); \
    }

#define CODEC_IMPL_LAYER_OBJ(NAME, OBJ_PREFIX, NEXT_LAYER, ...) \
    OBJ_PREFIX Codec_LayerImpl NAME = { \
        .parse = NAME ## _parse, \
        .write = NAME ## _write, \
        .getLen = NAME ## _getLen, \
        .nextLayer = NEXT_LAYER, \
    };

#define CODEC_IMPL_LAYER(NAME, PACKET_TYPE, FN_PREFIX, OBJ_PREFIX, NEXT_LAYER, ...) \
    CODEC_IMPL_ENCODE(NAME ## _write, FN_PREFIX, PACKET_TYPE, __VA_ARGS__) \
    CODEC_IMPL_DECODE(NAME ## _parse, FN_PREFIX,PACKET_TYPE, __VA_ARGS__) \
    CODEC_IMPL_GET_LEN(NAME ## _getLen, FN_PREFIX, PACKET_TYPE, __VA_ARGS__) \
    CODEC_IMPL_LAYER_OBJ(NAME, OBJ_PREFIX, NEXT_LAYER, __VA_ARGS__)

#endif // _CODEC_MACRO_H_
