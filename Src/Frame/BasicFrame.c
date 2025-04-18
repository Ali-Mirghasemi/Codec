#include "BasicFrame.h"

#if CODEC_DECODE
static Codec_Error      BasicFrame_Header_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
static Codec_Error      BasicFrame_Data_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
#endif // CODEC_DECODE

#if CODEC_ENCODE
static Codec_Error      BasicFrame_Header_write(Codec* codec, Codec_Frame* frame, OStream* stream);
static Codec_Error      BasicFrame_Data_write(Codec* codec, Codec_Frame* frame, OStream* stream);
#endif // CODEC_ENCODE

static Stream_LenType   BasicFrame_Header_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
static Codec_LayerImpl* BasicFrame_Header_nextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase);

static Stream_LenType   BasicFrame_Data_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase);

static const Codec_LayerImpl BASIC_FRAME_HEADER_IMPL = {
#if CODEC_DECODE
    .parse = BasicFrame_Header_parse,
#endif
#if CODEC_ENCODE
    .write = BasicFrame_Header_write,
#endif
    .getLen = BasicFrame_Header_getLen,
    .nextLayer = BasicFrame_Header_nextLayer,
};

static const Codec_LayerImpl BASIC_FRAME_DATA_IMPL = {
#if CODEC_DECODE
    .parse = BasicFrame_Data_parse,
#endif
#if CODEC_ENCODE
    .write = BasicFrame_Data_write,
#endif
    .getLen = BasicFrame_Data_getLen,
    .nextLayer = Codec_endLayer,
};

/**
 * @brief fill basic frame with data
 *
 * @param frame
 * @param data
 * @param size
 */
void BasicFrame_init(BasicFrame* frame, uint8_t* data, uint32_t size) {
    frame->Header.PacketSize = size;
    frame->Data.Data = data;
}
/**
 * @brief return basic frame base layer
 * 
 * @return * Codec_LayerImpl* 
 */
Codec_LayerImpl* BasicFrame_baseLayer(void) {
    return (Codec_LayerImpl*) &BASIC_FRAME_HEADER_IMPL;
}
/**
 * @brief return packet size + header size
 * 
 * @param frame 
 * @return uint32_t 
 */
uint32_t BasicFrame_len(BasicFrame* frame) {
    return frame->Header.PacketSize + sizeof(BasicFrame_Header);
}
#if CODEC_DECODE
/**
 * @brief header parse function
 *
 * @param codec
 * @param frame
 * @param stream
 * @return Codec_Error
 */
Codec_Error BasicFrame_Header_parse(Codec* codec, Codec_Frame* frame, IStream* stream) {
    ((BasicFrame*) frame)->Header.PacketSize = IStream_readUInt32(stream);
    return CODEC_OK;
}
/**
 * @brief data parse function
 *
 * @param codec
 * @param frame
 * @param stream
 * @return Codec_Error
 */
Codec_Error BasicFrame_Data_parse(Codec* codec, Codec_Frame* frame, IStream* stream) {
    BasicFrame* bFrame = (BasicFrame*) frame;
    IStream_readBytes(stream, bFrame->Data.Data, bFrame->Header.PacketSize);
    return CODEC_OK;
}
#endif // CODEC_DECODE
#if CODEC_ENCODE
/**
 * @brief header write function
 *
 * @param codec
 * @param frame
 * @param stream
 * @return Codec_Error
 */
Codec_Error BasicFrame_Header_write(Codec* codec, Codec_Frame* frame, OStream* stream) {
    OStream_writeUInt32(stream, ((BasicFrame*) frame)->Header.PacketSize);
    return CODEC_OK;
}
/**
 * @brief data write function
 *
 * @param codec
 * @param frame
 * @param stream
 * @return Codec_Error
 */
Codec_Error BasicFrame_Data_write(Codec* codec, Codec_Frame* frame, OStream* stream) {
    BasicFrame* bFrame = (BasicFrame*) frame;
    OStream_writeBytes(stream, bFrame->Data.Data, bFrame->Header.PacketSize);
    return CODEC_OK;
}
#endif // CODEC_ENCODE
/**
 * @brief header get len function
 *
 * @param codec
 * @param frame
 * @return Stream_LenType
 */
Stream_LenType BasicFrame_Header_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return sizeof(BasicFrame_Header);
}
/**
 * @brief header get upper layer function
 *
 * @param codec
 * @param frame
 * @return Codec_LayerImpl*
 */
Codec_LayerImpl* BasicFrame_Header_nextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return (Codec_LayerImpl*) &BASIC_FRAME_DATA_IMPL;
}
/**
 * @brief data get len function
 *
 * @param codec
 * @param frame
 * @return Stream_LenType
 */
Stream_LenType BasicFrame_Data_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return ((BasicFrame*) frame)->Header.PacketSize;
}

