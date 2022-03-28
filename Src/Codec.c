#include "Codec.h"


void Codec_init(Codec* codec, Codec_LayerImpl* baseLayer, Codec_Frame* frame) {
    codec->BaseLayer = baseLayer;
    codec->CurrentLayer = baseLayer;
    codec->Frame = frame;
}

void Codec_decode(Codec* codec, IStream* stream) {
    Codec_LayerImpl* layer = codec->CurrentLayer;
    Codec_Frame* frame = codec->Frame;
    Codec_Error error;
    Stream_LenType layerLen;

    layerLen = layer->getLayerLen(codec, frame);
    while (IStream_available(stream) > layerLen) {
        // set limit for read header part
        IStream_setLimit(stream, layerLen);
        if ((error = layer->parse(codec, stream, frame)) != CODEC_OK) {
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
        // get layer len
        layerLen = layer->getLayerLen(codec, frame);
    }
    // update current layer under process
    codec->CurrentLayer = layer;
}

Codec_Error Codec_encode(Codec* codec, OStream* stream, Codec_Frame* frame) {
    Codec_LayerImpl* layer = codec->BaseLayer;
    Codec_Error error;

    while (layer != CODEC_LAYER_NULL) {
        OStream_setLimit(layer->getLayerLen(codec, frame));
        if((error = layer->write(codec, stream, frame)) != CODEC_OK) {
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
        layer = layer->getUpperLayer(codec, frame);
    }
    
    OStream_setLimit(stream, STREAM_NO_LIMIT);

    return error;
}

Codec_Error Codec_encodeFlush(Codec* codec, OStream* stream, Codec_Frame* frame) {
    Codec_LayerImpl* layer = codec->BaseLayer;
    Codec_Error error;

    while (layer != CODEC_LAYER_NULL) {
        OStream_setLimit(layer->getLayerLen(codec, frame));
        if((error = layer->write(codec, stream, frame)) != CODEC_OK) {
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
        OStream_flush(stream);
        layer = layer->getUpperLayer(codec, frame);
    }
    
    OStream_setLimit(stream, STREAM_NO_LIMIT);

    return error;
}

