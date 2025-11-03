/**
 * @file CodecUserConfigTemplate.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief This is configuration template file for Codec library
 * @version 0.1
 * @date 2025-11-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef _CODEC_USER_CONFIG_H_
#define _CODEC_USER_CONFIG_H_

/**
 * @brief enable user arguments into codec
 */
//#define CODEC_ARGS                              1
/**
 * @brief enable encode feature
 */
//#define CODEC_ENCODE                            1
/**
 * @brief enable decode feature
 */
//#define CODEC_DECODE                            1
/**
 * @brief enable next layer null feature, this feature allow you to set null as last layer function
 */
//#define CODEC_SUPPORT_NEXT_LAYER_NULL           1
/**
 * @brief This feature enable helper macros for codec library and need `Macro` library
 */
//#define CODEC_SUPPORT_MACRO                     (1 || defined(CODEC_LIB_MACRO))

/* Codec Encode Options */
/**
 * @brief enable encode on raw buffer
 */
//#define CODEC_ENCODE_ON_BUFFER              1
/**
 * @brief enable encode async feature,
 * this feature allow you to encode on stream with smaller buffer size
 */
//#define CODEC_ENCODE_ASYNC                  1
/**
 * @brief enable queue for encode async feature, !*this feature is not working yet*!
 */
//#define CODEC_ENCODE_QUEUE                  1
/**
 * @brief enable callback feature for when encode completed
 */
//#define CODEC_ENCODE_CALLBACK               1
/**
 * @brief enable encode error callback
 */
//#define CODEC_ENCODE_ERROR                  1
/**
 * @brief enable encode padding for keep layer size fixed
 */
//#define CODEC_ENCODE_PADDING                1
/* Encode Padding Options */
//#define CODEC_ENCODE_PADDING_IGNORE     -1
//#define CODEC_ENCODE_PADDING_VALUE      0x00
//#define CODEC_ENCODE_PADDING_MODE       CODEC_LAYER_PADDING_IGNORE

/* Codec Decode Options */
/**
 * @brief enable encode on raw buffer
 */
//#define CODEC_DECODE_ON_BUFFER              1
/**
 * @brief enable decode async feature,
 * this feature allow you to encode on stream with smaller buffer size
 */
//#define CODEC_DECODE_ASYNC                  1
/**
 * @brief enable queue for decode async feature, !*this feature is not working yet*!
 */
//#define CODEC_DECODE_QUEUE                  1
/**
 * @brief enable sync options for decode
 */
//#define CODEC_DECODE_SYNC                   1
/**
 * @brief enable callback feature for when decode completed
 */
//#define CODEC_DECODE_CALLBACK               1
/**
 * @brief enable decode error callback
 */
//#define CODEC_DECODE_ERROR                  1
/**
 * @brief enable decode padding for keep layer size fixed
 */
//#define CODEC_DECODE_PADDING                1

/**
 * @brief choose what type use for codec frame, default is void
 */
//#define CODEC_FRAME
//typedef void Codec_Frame;
/**
 * @brief choose what type use for codec error
 */
//#define CODEC_ERROR
//typedef uint32_t Codec_Error;
/**
 * @brief choose what type use for codec layer index
 */
//#define CODEC_LAYER_INDEX
//typedef uint16_t Codec_LayerIndex;

#endif // _CODEC_USER_CONFIG_H_
