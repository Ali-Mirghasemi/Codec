#include "BasicFrame.h"

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
 * @brief header get len function
 *
 * @param codec
 * @param frame
 * @return Stream_LenType
 */
Stream_LenType BasicFrame_Header_getLen(Codec* codec, Codec_Frame* frame) {
    return sizeof(BasicFrame_Header);
}
/**
 * @brief header get upper layer function
 *
 * @param codec
 * @param frame
 * @return Codec_LayerImpl*
 */
Codec_LayerImpl* BasicFrame_Header_getUpperLayer(Codec* codec, Codec_Frame* frame) {
    return &BASIC_FRAME_DATA_IMPL;
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
/**
 * @brief data get len function
 *
 * @param codec
 * @param frame
 * @return Stream_LenType
 */
Stream_LenType BasicFrame_Data_getLen(Codec* codec, Codec_Frame* frame) {
    return ((BasicFrame*) frame)->Header.PacketSize;
}
/**
 * @brief data get upper layer function
 *
 * @param codec
 * @param frame
 * @return Codec_LayerImpl*
 */
Codec_LayerImpl* BasicFrame_Data_getUpperLayer(Codec* codec, Codec_Frame* frame) {
    return CODEC_LAYER_NULL;
}
