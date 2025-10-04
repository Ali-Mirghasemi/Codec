#include "Codec.h"

#ifndef NULL
    #define NULL          ((void*) 0)
#endif

#if CODEC_SUPPORT_NEXT_LAYER_NULL
    #define __nextLayer(C, F, L, P)             (L)->nextLayer ? (L)->nextLayer((C), (F), (P)) : CODEC_LAYER_NULL
#else
    #define __nextLayer(C, F, L, P)             (L)->nextLayer((C), (F), (P))
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
    codec->DecodeAll = 0;
    codec->FreeStream = 1;
}
/**
 * @brief This function help you te get full frame size before encode
 *
 * @param codec
 * @param frame
 * @return Stream_LenType
 */
Stream_LenType Codec_frameSize(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    Codec_LayerImpl* layer = codec->BaseLayer;
    uint32_t size = 0;
    while (layer) {
        size += layer->getLen(codec, frame, phase);
        layer = __nextLayer(codec, frame, layer, phase);
    }
    return size;
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
    StreamIn stream;
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
Codec_Status Codec_decodeFrame(Codec* codec, Codec_Frame* frame, StreamIn* stream) {
    Codec_LayerImpl* layer = codec->BaseLayer;
    Codec_Error error;
    Codec_Status status = Codec_Status_Error;
    StreamIn lock;
    Stream_LenType layerLen;

    layerLen = layer->getLen(codec, frame, Codec_Phase_Decode);
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
            if ((layerLen = IStream_availableUncheck(&lock)) > 0) {
                // add padding
                IStream_ignore(&lock, layerLen);
            }
        #endif
            // unlock stream
            IStream_unlock(stream, &lock);
            if ((layer = __nextLayer(codec, frame, layer, Codec_Phase_Decode)) == CODEC_LAYER_NULL) {
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
        layerLen = layer->getLen(codec, frame, Codec_Phase_Decode);
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
void Codec_decode(Codec* codec, StreamIn* stream) {
    Codec_Frame* frame = codec->RxFrame;
    StreamIn lock;
    Codec_Error error;
    Stream_LenType layerLen;

    layerLen = codec->RxLayer->getLen(codec, frame, Codec_Phase_Decode);
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
                if (codec->FreeStream) {
                    IStream_ignore(stream, available);
                }
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
        #if CODEC_DECODE_PADDING
            if ((layerLen = IStream_availableUncheck(&lock)) > 0) {
                // add padding
                IStream_ignore(&lock, layerLen);
            }
        #endif
            // unlock stream
            IStream_unlock(stream, &lock);
            if ((codec->RxLayer = __nextLayer(codec, frame, codec->RxLayer, Codec_Phase_Decode)) == CODEC_LAYER_NULL
            ) {
                // frame received
            #if CODEC_DECODE_CALLBACK
                if (codec->onDecode) {
                    codec->onDecode(codec, frame);
                }
            #endif // CODEC_DECODE_CALLBACK
                // back to base layer
                codec->RxLayer = codec->BaseLayer;
                // a single frame detected
                if (!codec->DecodeAll) {
                    break;
                }
            }
        }
        // get layer len
        layerLen = codec->RxLayer->getLen(codec, frame, Codec_Phase_Decode);
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
    StreamOut stream;
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
Codec_Status Codec_encodeFrame(Codec* codec, Codec_Frame* frame, StreamOut* stream, Codec_EncodeMode mode) {
    Codec_LayerImpl* layer = codec->BaseLayer;
    Stream_LenType layerLen;
    Codec_Status status = Codec_Status_Pending;
    StreamOut lock;
    Codec_Error error;

    while (layer != CODEC_LAYER_NULL &&
            (layerLen = layer->getLen(codec, frame, Codec_Phase_Encode)) <= OStream_space(stream)) {
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
            if ((layerLen = OStream_spaceUncheck(&lock)) > 0) {
            #if CODEC_ENCODE_PADDING_MODE == CODEC_ENCODE_PADDING_IGNORE
                OStream_ignore(&lock, layerLen);
            #else
                OStream_writePadding(&lock, (uint8_t) CODEC_ENCODE_PADDING_VALUE, layerLen);
            #endif
            }
        #endif // CODEC_ENCODE_PADDING
            OStream_unlock(stream, &lock);
            if (Codec_EncodeMode_FlushLayer == mode) {
                OStream_flush(stream);
            }
            layer = __nextLayer(codec, frame, layer, Codec_Phase_Encode);
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
void Codec_encode(Codec* codec, StreamOut* stream) {
    Codec_Frame* frame = codec->TxFrame;
    StreamOut lock;
    Codec_Error error;
    Stream_LenType layerLen;

    while (codec->TxLayer != CODEC_LAYER_NULL &&
            (layerLen = codec->TxLayer->getLen(codec, frame, Codec_Phase_Encode)) <= OStream_space(stream)) {
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
            if ((layerLen = OStream_spaceUncheck(&lock)) > 0) {
            #if CODEC_ENCODE_PADDING_MODE == CODEC_ENCODE_PADDING_IGNORE
                OStream_ignore(&lock, layerLen);
            #else
                OStream_writePadding(&lock, (uint8_t) CODEC_ENCODE_PADDING_VALUE, layerLen);
            #endif
            }
        #endif // CODEC_ENCODE_PADDING
            OStream_unlock(stream, &lock);
            if (Codec_EncodeMode_FlushLayer == codec->EncodeMode) {
                OStream_flush(stream);
            }
            codec->TxLayer = __nextLayer(codec, frame, codec->TxLayer, Codec_Phase_Encode);
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
/**
 * @brief Set FreeStream flag in codec, 
 * it's enable/disable free stream after use sync function and when not found pattern
 * 
 * @param codec 
 * @param enabled 0: disabled, !0: enabled
 */
void Codec_setFreeStream(Codec* codec, uint8_t enabled) {
    codec->FreeStream = enabled != 0;
}
/**
 * @brief enable or disable decode all bytes in buffer or just decode a single frame 
 * in decode function for async mode
 * 
 * @param codec 
 * @param enabled 0: disabled, !0: enabled 
 */
void Codec_setDecodeAll(Codec* codec, uint8_t enabled) {
    codec->DecodeAll = enabled != 0;
}

#if !CODEC_SUPPORT_NEXT_LAYER_NULL
/**
 * @brief return next layer of packet, return null if it's last layer
 * 
 * @param codec 
 * @param frame 
 * @param phase 
 * @return Codec_LayerImpl* 
 */
Codec_LayerImpl* Codec_endLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return CODEC_LAYER_NULL;
}
#endif

