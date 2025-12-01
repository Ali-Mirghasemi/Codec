/**
 * @file CodecConfig.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief This is configuration file for Codec library
 * @version 0.1
 * @date 2025-11-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef _CODEC_CONFIG_H_
#define _CODEC_CONFIG_H_

#include <stdint.h>

/* Check User Config file exists to include it or not */
#ifndef __has_include
    #define __has_include(X)    0
#endif

#if defined(CODEC_USER_CONFIG) || __has_include("CodecUserConfig.h")
    #include "CodecUserConfig.h"
#endif

#ifndef CODEC_LIB_MACRO
    #define CODEC_LIB_MACRO     0
#endif

/************************************************************************/
/*                            Configuration                             */
/************************************************************************/
/**
 * @brief enable user arguments into codec
 */
#ifndef CODEC_ARGS
    #define CODEC_ARGS                              1
#endif
/**
 * @brief enable encode feature
 */
#ifndef CODEC_ENCODE
    #define CODEC_ENCODE                            1
#endif
/**
 * @brief enable decode feature
 */
#ifndef CODEC_DECODE
    #define CODEC_DECODE                            1
#endif
/**
 * @brief enable next layer null feature, this feature allow you to set null as last layer function
 */
#ifndef CODEC_SUPPORT_NEXT_LAYER_NULL
    #define CODEC_SUPPORT_NEXT_LAYER_NULL           1
#endif
/**
 * @brief This feature enable helper macros for codec library and need `Macro` library
 */
#ifndef CODEC_SUPPORT_MACRO
    #define CODEC_SUPPORT_MACRO                     (1 || CODEC_LIB_MACRO)
#endif

/* Codec Encode Options */
#if CODEC_ENCODE
    /**
     * @brief enable encode on raw buffer
     */
    #ifndef CODEC_ENCODE_ON_BUFFER
        #define CODEC_ENCODE_ON_BUFFER              1
    #endif
    /**
     * @brief enable encode async feature,
     * this feature allow you to encode on stream with smaller buffer size
     */
    #ifndef CODEC_ENCODE_ASYNC
        #define CODEC_ENCODE_ASYNC                  1
    #endif
    /**
     * @brief enable queue for encode async feature, !*this feature is not working yet*!
     */
    #ifndef CODEC_ENCODE_QUEUE
        #define CODEC_ENCODE_QUEUE                  1
    #endif
    /**
     * @brief enable callback feature for when encode completed
     */
    #ifndef CODEC_ENCODE_CALLBACK
        #define CODEC_ENCODE_CALLBACK               1
    #endif
    /**
     * @brief enable encode error callback
     */
    #ifndef CODEC_ENCODE_ERROR
        #define CODEC_ENCODE_ERROR                  1
    #endif
    /**
     * @brief enable encode padding for keep layer size fixed
     */
    #ifndef CODEC_ENCODE_PADDING
        #define CODEC_ENCODE_PADDING                1
    #endif
    /* Encode Padding Options */
    #if CODEC_ENCODE_PADDING
        #ifndef CODEC_ENCODE_PADDING_IGNORE    
            #define CODEC_ENCODE_PADDING_IGNORE     -1
        #endif
        
        #ifndef CODEC_ENCODE_PADDING_VALUE
            #define CODEC_ENCODE_PADDING_VALUE      0x00
        #endif

        #ifndef CODEC_ENCODE_PADDING_MODE
            #define CODEC_ENCODE_PADDING_MODE       CODEC_LAYER_PADDING_IGNORE
        #endif
    #endif // CODEC_ENCODE_PADDING
#endif // CODEC_ENCODE

/* Codec Decode Options */
#if CODEC_DECODE
    /**
     * @brief enable encode on raw buffer
     */
    #ifndef CODEC_DECODE_ON_BUFFER
        #define CODEC_DECODE_ON_BUFFER              1
    #endif
    /**
     * @brief enable decode async feature,
     * this feature allow you to encode on stream with smaller buffer size
     */
    #ifndef CODEC_DECODE_ASYNC
        #define CODEC_DECODE_ASYNC                  1
    #endif
    /**
     * @brief enable queue for decode async feature, !*this feature is not working yet*!
     */
    #ifndef CODEC_DECODE_QUEUE
        #define CODEC_DECODE_QUEUE                  1
    #endif
    /**
     * @brief enable sync options for decode
     */
    #ifndef CODEC_DECODE_SYNC
        #define CODEC_DECODE_SYNC                   1
    #endif
    /**
     * @brief enable callback feature for when decode completed
     */
    #ifndef CODEC_DECODE_CALLBACK
        #define CODEC_DECODE_CALLBACK               1
    #endif
    /**
     * @brief enable decode error callback
     */
    #ifndef CODEC_DECODE_ERROR
        #define CODEC_DECODE_ERROR                  1
    #endif
    /**
     * @brief enable decode padding for keep layer size fixed
     */
    #ifndef CODEC_DECODE_PADDING
        #define CODEC_DECODE_PADDING                1
    #endif
#endif // CODEC_DECODE
/**
 * @brief choose what type use for codec frame, default is void
 */
#ifndef CODEC_FRAME
    typedef void Codec_Frame;
#endif
/**
 * @brief choose what type use for codec error
 */
#ifndef CODEC_ERROR
    typedef uint32_t Codec_Error;
#endif
/**
 * @brief choose what type use for codec layer index
 */
#ifndef CODEC_LAYER_INDEX
    typedef uint16_t Codec_LayerIndex;
#endif

/************************************************************************/

#endif // _CODEC_CONFIG_H_
