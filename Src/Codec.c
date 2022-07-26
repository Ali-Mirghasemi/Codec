#include "Codec.h"

#ifndef NULL
    #define NULL          ((void*) 0)
#endif

/**
 * @brief initialize codec
 *
 * @param codec codec to initialize
 * @param baseLayer base layer of codec
 * @param frame frame to use for codec
 */
void Codec_init(Codec* codec, Codec_LayerImpl* baseLayer) {
    codec->BaseLayer = baseLayer;
#if CODEC_ARGS
    codec->Args = (void*) 0;
#endif
#if CODEC_DECODE
#if CODEC_DECODE_ASYNC
    codec->RxLayer = baseLayer;
    codec->RxFrame = NULL;
#endif
#if CODEC_DECODE_CALLBACK
    codec->onDecode = (Codec_OnFrameFn) 0;
#endif
#if CODEC_DECODE_ERROR
    codec->onDecodeError = (Codec_OnErrorFn) 0;
#endif
#if CODEC_DECODE_SYNC
    codec->sync = (Codec_SyncFn) 0;
#endif
#endif // CODEC_DECODE
#if CODEC_ENCODE
#if CODEC_ENCODE_ASYNC
    codec->TxLayer = baseLayer;
    codec->TxFrame = NULL;
    codec->EncodeMode = Codec_EncodeMode_Normal;
#endif
#if CODEC_ENCODE_CALLBACK
    codec->onEncode = (Codec_OnFrameFn) 0;
#endif
#if CODEC_ENCODE_ERROR
    codec->onEncodeError = (Codec_OnErrorFn) 0;
#endif
#endif // CODEC_ENCODE
}
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
#endif // CODEC_ARGS
#if CODEC_DECODE
#if CODEC_DECODE_CALLBACK
/**
 * @brief set on decode frame callback
 *
 * @param codec
 * @param fn
 */
void Codec_onDecode(Codec* codec, Codec_OnFrameFn fn) {
    codec->onDecode = fn;
}
#endif // CODEC_DECODE_CALLBACK
#if CODEC_DECODE_ERROR
/**
 * @brief set on decode error callback
 *
 * @param codec
 * @param fn
 */
void Codec_onDecodeError(Codec* codec, Codec_OnErrorFn fn) {
    codec->onDecodeError = fn;
}
#endif // CODEC_DECODE_ERROR
#if CODEC_DECODE_SYNC
/**
 * @brief set decode sync function
 *
 * @param codec
 * @param fn
 */
void Codec_setDecodeSync(Codec* codec, Codec_SyncFn fn) {
    codec->sync = fn;
}
#endif // CODEC_DECODE_SYNC
#if CODEC_DECODE_ON_BUFFER
/**
 * @brief decode a frame from a buffer
 *
 * @param codec
 * @param buffer
 * @param size
 * @param frame
 * @return Codec_Status
 */
Codec_Status Codec_decodeBuffer(Codec* codec, Codec_Frame* frame, uint8_t* buffer, Stream_LenType size) {
    IStream stream;
    IStream_init(&stream, NULL, buffer, size);
    Stream_moveWritePos(&stream.Buffer, size);
    return Codec_decodeFrame(codec, frame, &stream);
}
#endif // CODEC_DECODE_ON_BUFFER
/**
 * @brief decode a frame from a stream, all of frame bytes must exists
 *
 * @param codec
 * @param stream
 * @param frame
 * @return Codec_Status
 */
Codec_Status Codec_decodeFrame(Codec* codec, Codec_Frame* frame, IStream* stream) {
    Codec_LayerImpl* layer = codec->BaseLayer;
    Codec_Error error;
    Codec_Status status = Codec_Status_Error;
    IStream lock;
    Stream_LenType layerLen;

    layerLen = layer->getLayerLen(codec, frame);
    while (IStream_available(stream) >= layerLen) {
    #if CODEC_DECODE_SYNC
        if (layer == codec->BaseLayer && codec->sync) {
            Stream_LenType available = IStream_available(stream);
            Stream_LenType len = codec->sync(codec, stream);
            if (len > 0) {
                IStream_ignore(stream, len);
                if (IStream_available(stream) < layerLen) {
                    return Codec_Status_Pending;
                }
            }
            else if (len == -1) {
                IStream_ignore(stream, available);
                return Codec_Status_Pending;
            }
        }
    #endif
        // set limit for read header part
        IStream_lock(stream, &lock, layerLen);
        if ((error = layer->parse(codec, frame, &lock)) != CODEC_OK) {
        #if CODEC_DECODE_ERROR
            if (codec->onDecodeError) {
                codec->onDecodeError(codec, frame, layer, error);
            }
        #endif
            // back to base layer
            layer = codec->BaseLayer;
            // unlock stream
            IStream_unlockIgnore(stream);
            // ignore one byte
            IStream_ignore(stream, 1);
        }
        else {
        #if CODEC_DECODE_PADDING
            // add padding
            IStream_ignore(&lock, IStream_lockLen(stream, &lock));
        #endif
            // unlock stream
            IStream_unlock(stream, &lock);
            if((layer = layer->getUpperLayer(codec, frame)) == CODEC_LAYER_NULL) {
            #if CODEC_DECODE_CALLBACK
                // frame received
                if (codec->onDecode) {
                    codec->onDecode(codec, frame);
                }
            #endif // CODEC_DECODE_CALLBACK
                status = Codec_Status_Done;
                break;
            }
        }
        // get layer len
        layerLen = layer->getLayerLen(codec, frame);
    }

    return status;
}
#if CODEC_DECODE_ASYNC
/**
 * @brief begin of decode frame over stream
 *
 * @param codec
 * @param frame
 */
void Codec_beginDecode(Codec* codec, Codec_Frame* frame) {
    codec->RxLayer = codec->BaseLayer;
    codec->RxFrame = frame;
}
/**
 * @brief decode frame over input stream
 *
 * @param codec codec to decode
 * @param stream input stream to decode
 */
void Codec_decode(Codec* codec, IStream* stream) {
    Codec_Frame* frame = codec->RxFrame;
    IStream lock;
    Codec_Error error;
    Stream_LenType layerLen;

    layerLen = codec->RxLayer->getLayerLen(codec, frame);
    while (IStream_available(stream) >= layerLen) {
    #if CODEC_DECODE_SYNC
        if (codec->RxLayer == codec->BaseLayer && codec->sync) {
            Stream_LenType available = IStream_available(stream);
            Stream_LenType len = codec->sync(codec, stream);
            if (len > 0) {
                IStream_ignore(stream, len);
                if (IStream_available(stream) < layerLen) {
                    break;
                }
            }
            else if (len == -1) {
                IStream_ignore(stream, available);
                break;
            }
        }
    #endif
        // set limit for read header part
        IStream_lock(stream, &lock, layerLen);
        if ((error = codec->RxLayer->parse(codec, frame, &lock)) != CODEC_OK) {
        #if CODEC_DECODE_ERROR
            if (codec->onDecodeError) {
                codec->onDecodeError(codec, frame, codec->RxLayer, error);
            }
        #endif
            // back to base layer
            codec->RxLayer = codec->BaseLayer;
            // unlock stream
            IStream_unlockIgnore(stream);
            // ignore one byte
            IStream_ignore(stream, 1);
        }
        else {
        #if CODEC_LAYER_PADDING
            // add padding
            IStream_ignore(stream, IStream_lockLen(stream, &lock));
        #endif
            // unlock stream
            IStream_unlock(stream, &lock);
            if((codec->RxLayer = codec->RxLayer->getUpperLayer(codec, frame)) == CODEC_LAYER_NULL) {
                // frame received
            #if CODEC_DECODE_CALLBACK
                if (codec->onDecode) {
                    codec->onDecode(codec, frame);
                }
            #endif // CODEC_DECODE_CALLBACK
                // back to base layer
                codec->RxLayer = codec->BaseLayer;
            #if !CODEC_DECODE_CONTINUOUS
                // a single frame detected
                break;
            #endif
            }
        }
        // get layer len
        layerLen = codec->RxLayer->getLayerLen(codec, frame);
    }
}
#endif // CODEC_DECODE_ASYNC
#endif // CODEC_DECODE
#if CODEC_ENCODE
#if CODEC_ENCODE_CALLBACK
/**
 * @brief set encode callback function
 *
 * @param codec
 * @param fn
 */
void Codec_onEncode(Codec* codec, Codec_OnFrameFn fn) {
    codec->onEncode = fn;
}
#endif // CODEC_ENCODE_CALLBACK
#if CODEC_ENCODE_ERROR
/**
 * @brief set encode error callback function
 *
 * @param codec
 * @param fn
 */
void Codec_onEncodeError(Codec* codec, Codec_OnErrorFn fn) {
    codec->onEncodeError = fn;
}
#endif // CODEC_ENCODE_ERROR
#if CODEC_ENCODE_ON_BUFFER
/**
 * @brief encode a frame to a buffer
 *
 * @param codec
 * @param frame
 * @param buffer
 * @param size
 * @return Codec_Status
 */
Codec_Status Codec_encodeBuffer(Codec* codec, Codec_Frame* frame, uint8_t* buffer, Stream_LenType size) {
    OStream stream;
    OStream_init(&stream, NULL, buffer, size);
    return Codec_encodeFrame(codec, frame, &stream, Codec_EncodeMode_Normal);
}
#endif // CODEC_ENCODE_ON_BUFFER
/**
 * @brief encode a frame to a stream
 *
 * @param codec
 * @param stream
 * @param frame
 * @return Codec_Status
 */
Codec_Status Codec_encodeFrame(Codec* codec, Codec_Frame* frame, OStream* stream, Codec_EncodeMode mode) {
    Codec_LayerImpl* layer = codec->BaseLayer;
    Stream_LenType layerLen;
    Codec_Status status = Codec_Status_Pending;
    OStream lock;
    Codec_Error error;

    while (layer != CODEC_LAYER_NULL &&
            (layerLen = layer->getLayerLen(codec, frame)) <= OStream_space(stream)) {
        OStream_lock(stream, &lock, layerLen);
        if((error = layer->write(codec, frame, &lock)) != CODEC_OK) {
        #if CODEC_ENCODE_ERROR
            if (codec->onEncodeError) {
                codec->onEncodeError(codec, frame, layer, error);
            }
        #endif
            //OStream_unlockIgnore(stream);
            return Codec_Status_Error;
        }
        else {
        #if CODEC_ENCODE_PADDING
            #if CODEC_ENCODE_PADDING_MODE == CODEC_ENCODE_PADDING_IGNORE
                OStream_ignore(&lock, OStream_lockLen(stream, &lock));
            #else
                OStream_writePadding(&lock, (uint8_t) CODEC_ENCODE_PADDING_VALUE, OStream_lockLen(stream, &lock));
            #endif
        #endif // CODEC_ENCODE_PADDING
            OStream_unlock(stream, &lock);
            if (Codec_EncodeMode_FlushLayer == mode) {
                OStream_flush(stream);
            }
            layer = layer->getUpperLayer(codec, frame);
        }
    }

    if (layer == CODEC_LAYER_NULL) {
        // done
    #if CODEC_ENCODE_CALLBACK
        if (codec->onEncode) {
            codec->onEncode(codec, frame);
        }
    #endif // CODEC_ENCODE_CALLBACK

        if ((mode & Codec_EncodeMode_Flush) != 0) {
            OStream_flush(stream);
        }
        status = Codec_Status_Done;
    }

    return status;
}
#if CODEC_ENCODE_ASYNC
/**
 * @brief set encode mode
 *
 * @param codec
 * @param mode
 */
void Codec_encodeMode(Codec* codec, Codec_EncodeMode mode) {
    codec->EncodeMode = mode;
}
/**
 * @brief begin of encode frame over stream
 *
 * @param codec
 * @param frame
 */
void Codec_beginEncode(Codec* codec, Codec_Frame* frame, Codec_EncodeMode mode) {
    codec->TxLayer = codec->BaseLayer;
    codec->TxFrame = frame;
    codec->EncodeMode = mode;
}
/**
 * @brief encode a frame to output stream
 *
 * @param codec
 * @param frame
 * @param stream
 * @param mode encode mode
 * @return Codec_Error
 */
void Codec_encode(Codec* codec, OStream* stream) {
    Codec_Frame* frame = codec->TxFrame;
    OStream lock;
    Codec_Error error;
    Stream_LenType layerLen;

    while (codec->TxLayer != CODEC_LAYER_NULL &&
            (layerLen = codec->TxLayer->getLayerLen(codec, frame)) <= OStream_space(stream)) {
        OStream_lock(stream, &lock, layerLen);
        if((error = codec->TxLayer->write(codec, frame, &lock)) != CODEC_OK) {
        #if CODEC_ENCODE_ERROR
            if (codec->onEncodeError) {
                codec->onEncodeError(codec, frame, codec->TxLayer, error);
            }
        #endif
            // unlock stream
            OStream_unlockIgnore(stream);
            // back to base layer
            codec->TxLayer = codec->BaseLayer;
            return;
        }
        else {
        #if CODEC_ENCODE_PADDING
            #if CODEC_ENCODE_PADDING_MODE == CODEC_ENCODE_PADDING_IGNORE
                OStream_ignore(&lock, OStream_lockLen(stream, &lock));
            #else
                OStream_writePadding(&lock, (uint8_t) CODEC_ENCODE_PADDING_VALUE, OStream_lockLen(stream, &lock));
            #endif
        #endif // CODEC_ENCODE_PADDING
            OStream_unlock(stream, &lock);
            if (Codec_EncodeMode_FlushLayer == codec->EncodeMode) {
                OStream_flush(stream);
            }
            codec->TxLayer = codec->TxLayer->getUpperLayer(codec, frame);
        }
    }

    if (codec->TxLayer == NULL) {
        // done
    #if CODEC_ENCODE_CALLBACK
        if (codec->onEncode) {
            codec->onEncode(codec, frame);
        }
    #endif // CODEC_ENCODE_CALLBACK

        if ((codec->EncodeMode & Codec_EncodeMode_Flush) != 0) {
            OStream_flush(stream);
        }
    }
}
#endif // CODEC_ENCODE_ASYNC
#endif // CODEC_ENCODE


