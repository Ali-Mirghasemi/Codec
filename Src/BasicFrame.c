#include "BasicFrame.h"


void BasicFrame_init(BasicFrame* frame, uint8_t* data, uint16_t size) {
    frame->Header.PacketSize = size;
    frame->Data.Data = data;
    frame->Data.Size = size;
}

Codec_Error BasicFrame_Header_parse(Codec* codec, IStream* stream, Codec_Frame* frame) {
    ((BasicFrame*) frame)->Header.PacketSize = IStream_readUInt32(stream);
    return CODEC_OK;
}
Codec_Error BasicFrame_Header_write(Codec* codec, OStream* stream, Codec_Frame* frame) {
    OStream_writeUInt32(stream, ((BasicFrame*) frame)->Header.PacketSize);
    return CODEC_OK;
}
Stream_LenType BasicFrame_Header_getLen(Codec* codec, Codec_Frame* frame) {
    return sizeof(BasicFrame_Header);
}
Codec_LayerImpl* BasicFrame_Header_getUpperLayer(Codec* codec, Codec_Frame* frame) {
    return &BASIC_FRAME_DATA_IMPL;
}

Codec_Error BasicFrame_Data_parse(Codec* codec, IStream* stream, Codec_Frame* frame) {
    BasicFrame* bFrame = (BasicFrame*) frame;
    uint32_t len = bFrame->Header.PacketSize > bFrame->Data.Size ? bFrame->Data.Size : bFrame->Header.PacketSize;
    IStream_readBytes(stream, bFrame->Data.Data, len);
    return CODEC_OK;
}
Codec_Error BasicFrame_Data_write(Codec* codec, OStream* stream, Codec_Frame* frame) {
    BasicFrame* bFrame = (BasicFrame*) frame;
    OStream_writeBytes(stream, bFrame->Data.Data, bFrame->Header.PacketSize);
    return CODEC_OK;
}
Stream_LenType BasicFrame_Data_getLen(Codec* codec, Codec_Frame* frame) {
    return ((BasicFrame*) frame)->Header.PacketSize;
}
Codec_LayerImpl* BasicFrame_Data_getUpperLayer(Codec* codec, Codec_Frame* frame) {
    return CODEC_LAYER_NULL;
}
