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

#define CODEC_WRITE_VAL(VAL)                    err = OStream_write(stream, (uint8_t*) &(VAL), sizeof(VAL)); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }

#define CODEC_WRITE(TYPE, ...)                  err = OStream_write ##TYPE(stream, __VA_ARGS__); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }

#define CODEC_READ_VAL(VAL)                     err = OStream_read(stream, (uint8_t*) &(VAL), sizeof(VAL)); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }                                        


#define CODEC_READ(TYPE, ...)                   err = OStream_read ##TYPE ##Safe(stream, __VA_ARGS__); \
                                                if (err != Stream_Ok) { \
                                                    return err | CODEC_ERROR_STREAM; \
                                                }                                        

#define CODEC_VALUE_LEN(TYPE)                   len += sizeof(STREAM_VALUE_TYPE(TYPE))
#define CODEC_VALUE_ARR_LEN(TYPE, LEN)          len += sizeof(STREAM_VALUE_TYPE(TYPE)) * (LEN)
    
// ------------------------------------ Implementation Macros ----------------------------------

#define CODEC_IMPL_WRITE_0()             
#define CODEC_IMPL_WRITE_0(VAL)                 CODEC_WRITE_VAL(VAL);
#define CODEC_IMPL_WRITE_1(TYPE, VAL)           CODEC_WRITE(TYPE, VAL);
#define CODEC_IMPL_WRITE_2(TYPE, VAL, LEN)      CODEC_WRITE(TYPE, VAL, LEN);

#define CODEC_IMPL_READ_0()                     
#define CODEC_IMPL_READ_1(VAL)                  CODEC_READ_VAL(TYPE);
#define CODEC_IMPL_READ_2(TYPE, VAL)            CODEC_READ(TYPE, VAL);
#define CODEC_IMPL_READ_3(TYPE, VAL, LEN)       CODEC_READ(TYPE, VAL, LEN);

#define CODEC_IMPL_LEN_0(TYPE)                  
#define CODEC_IMPL_LEN_1(VAL)                   len += sizeof(VAL);
#define CODEC_IMPL_LEN_2(TYPE, VAL)             len += sizeof(STREAM_VALUE_TYPE(TYPE));
#define CODEC_IMPL_LEN_3(TYPE, VAL, LEN)        len += sizeof(STREAM_VALUE_TYPE(TYPE)) * LEN;

#define CODEC_IMPL_ENCODE(NAME, FN_PREFIX, PACKET_TYPE, ...) \
    FN_PREFIX Codec_Error NAME(Codec* codec, Codec_Frame* __frame, OStream* stream, Codec_EncodeMode mode) { \
        CODEC_BEGIN(PACKET_TYPE* frame = (PACKET_TYPE*) __frame); \
        MACRO_FOR_MAP((CODEC_IMPL_WRITE_2, CODEC_IMPL_WRITE_1, CODEC_IMPL_WRITE_0, CODEC_IMPL_WRITE_0), __VA_ARGS__) \
        CODEC_END(); \
    }

#define CODEC_IMPL_DECODE(NAME, FN_PREFIX, PACKET_TYPE, ...) \
    FN_PREFIX Codec_Error NAME(Codec* codec, Codec_Frame* __frame, IStream* stream) { \
        CODEC_BEGIN(PACKET_TYPE* frame = (PACKET_TYPE*) __frame); \
        MACRO_FOR_MAP((CODEC_IMPL_READ_3, CODEC_IMPL_READ_2, CODEC_IMPL_READ_1, CODEC_IMPL_READ_0), __VA_ARGS__) \
        CODEC_END(); \
    }

#define CODEC_IMPL_GET_LEN(NAME, FN_PREFIX, PACKET_TYPE, ...) \
    FN_PREFIX Stream_LenType NAME(Codec* codec, Codec_Frame* __frame, Codec_Phase phase) { \
        CODEC_BEGIN(PACKET_TYPE* frame = (PACKET_TYPE*) __frame; , Stream_LenType len); \
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
