#include "Codec.h"

/**
 * @brief initialize codec
 *
 * @param codec codec to initialize
 * @param baseLayer base layer of codec
 * @param frame frame to use for codec
 */
void Codec_init(Codec* codec, Codec_LayerImpl* baseLayer, Codec_Frame* frame) {
    codec->BaseLayer = baseLayer;
    codec->CurrentLayer = baseLayer;
    codec->Frame = frame;
    codec->onFrame = (Codec_OnFrameFn) 0;
#if CODEC_DECODE_ERROR || CODEC_ENCODE_ERROR
    codec->onError = (Codec_OnErrorFn) 0;
#endif
#if CODEC_ARGS
    codec->Args = (void*) 0;
#endif
}
/**
 * @brief decode frame over input stream
 *
 * @param codec codec to decode
 * @param stream input stream to decode
 */
void Codec_decode(Codec* codec, IStream* stream) {
    Codec_LayerImpl* layer = codec->CurrentLayer;
    Codec_Frame* frame = codec->Frame;
    Codec_Error error;
    Stream_LenType layerLen;

    layerLen = layer->getLayerLen(codec, frame);
    while (IStream_available(stream) >= layerLen) {
        // set limit for read header part
        IStream_setLimit(stream, layerLen);
        if ((error = layer->parse(codec, frame, stream)) != CODEC_OK) {
        #if CODEC_DECODE_ERROR
            if (codec->onError) {
                codec->onError(codec, frame, layer, error);
            }
        #endif
            // back to base layer
            layer = codec->BaseLayer;
        }
    #if CODEC_LAYER_PADDING
        // add padding
        IStream_ignore(stream, IStream_getLimit(stream));
    #endif
        if((layer = layer->getUpperLayer(codec, frame)) == CODEC_LAYER_NULL) {
            // frame received
            if (codec->onFrame) {
                codec->onFrame(codec, frame);
                // back to base layer
                layer = codec->BaseLayer;
            }
        }
        // clear limit for read
        IStream_setLimit(stream, STREAM_NO_LIMIT);
        // get layer len
        layerLen = layer->getLayerLen(codec, frame);
    }
    // update current layer under process
    codec->CurrentLayer = layer;
}
/**
 * @brief encode frame over output stream
 *
 * @param codec
 * @param frame
 * @param stream
 * @param mode encode mode
 * @return Codec_Error
 */
Codec_Error Codec_encode(Codec* codec, Codec_Frame* frame, OStream* stream, Codec_EncodeMode mode) {
    Codec_LayerImpl* layer = codec->BaseLayer;
    Codec_Error error;

    while (layer != CODEC_LAYER_NULL) {
        OStream_setLimit(stream, layer->getLayerLen(codec, frame));
        if((error = layer->write(codec, frame, stream)) != CODEC_OK) {
        #if CODEC_ENCODE_ERROR
            if (codec->onError) {
                codec->onError(codec, frame, layer, error);
            }
        #endif
            break;
        }
    #if CODEC_LAYER_PADDING
        #if CODEC_LAYER_PADDING_MODE == CODEC_LAYER_PADDING_IGNORE
            OStream_ignore(stream, OStream_getLimit(stream));
        #else
            OStream_writePadding(stream, CODEC_LAYER_PADDING_VALUE, OStream_getLimit(stream));
        #endif
    #endif
        if (Codec_EncodeMode_FlushLayer == mode) {
            OStream_flush(stream);
        }
        layer = layer->getUpperLayer(codec, frame);
    }

    OStream_setLimit(stream, STREAM_NO_LIMIT);

    if (error == CODEC_OK && Codec_EncodeMode_Flush == mode) {
        OStream_flush(stream);
    }

    return error;
}
/**
 * @brief set on decode frame callback
 *
 * @param codec
 * @param fn
 */
void Codec_onFrame(Codec* codec, Codec_OnFrameFn fn) {
    codec->onFrame = fn;
}
#if CODEC_ENCODE_ERROR
/**
 * @brief set on encode/decode error callback
 *
 * @param codec
 * @param fn
 */
void Codec_onError(Codec* codec, Codec_OnErrorFn fn) {
    codec->onError = fn;
}
#endif

#if CODEC_ARGS
/**
 * @brief set user args
 *
 * @param codec
 * @param args
 */
void Codec_setArgs(Codec* codec, void* args) {
    codec->Args = args;
}
/**
 * @brief get user args
 *
 * @param codec
 * @return void*
 */
void* Codec_getArgs(Codec* codec) {
    return codec->Args;
}
#endif
