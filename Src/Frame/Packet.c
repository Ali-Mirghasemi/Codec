#include "Packet.h"

#if STREAM_BYTE_ORDER
    #define __setByteOrder(STREAM)      IStream_setByteOrder(STREAM, PACKET_BYTE_ORDER)
#else
    #define __setByteOrder(STREAM)   
#endif

static const uint16_t   __PACKET_FIRST_SIGN     = PACKET_FIRST_SIGN;
static const uint16_t   __PACKET_SECOND_SIGN    = PACKET_SECOND_SIGN;
static const uint32_t   __PACKET_FOOTER_SIGN    = PACKET_FOOTER_SIGN;

#if CODEC_DECODE
static Codec_Error      Packet_Header_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
static Codec_Error      Packet_Data_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
static Codec_Error      Packet_Footer_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
#endif // CODEC_DECODE

#if CODEC_ENCODE
static Codec_Error      Packet_Header_write(Codec* codec, Codec_Frame* frame, OStream* stream);
static Codec_Error      Packet_Data_write(Codec* codec, Codec_Frame* frame, OStream* stream);
static Codec_Error      Packet_Footer_write(Codec* codec, Codec_Frame* frame, OStream* stream);
#endif // CODEC_ENCODE

static Stream_LenType   Packet_Header_getLen(Codec* codec, Codec_Frame* frame);
static Codec_LayerImpl* Packet_Header_getUpperLayer(Codec* codec, Codec_Frame* frame);

static Stream_LenType   Packet_Data_getLen(Codec* codec, Codec_Frame* frame);
static Codec_LayerImpl* Packet_Data_getUpperLayer(Codec* codec, Codec_Frame* frame);

static Stream_LenType   Packet_Footer_getLen(Codec* codec, Codec_Frame* frame);
static Codec_LayerImpl* Packet_Footer_getUpperLayer(Codec* codec, Codec_Frame* frame);

static const Codec_LayerImpl PACKET_HEADER_IMPL = {
#if CODEC_DECODE
    Packet_Header_parse,
#endif 
#if CODEC_ENCODE
    Packet_Header_write,
#endif
    Packet_Header_getLen,
    Packet_Header_getUpperLayer,
};

static const Codec_LayerImpl PACKET_DATA_IMPL = {
#if CODEC_DECODE
    Packet_Data_parse,
#endif
#if CODEC_ENCODE
    Packet_Data_write,
#endif
    Packet_Data_getLen,
    Packet_Data_getUpperLayer,
};

static const Codec_LayerImpl PACKET_FOOTER_IMPL = {
#if CODEC_DECODE
    Packet_Footer_parse,
#endif
#if CODEC_ENCODE
    Packet_Footer_write,
#endif
    Packet_Footer_getLen,
    Packet_Footer_getUpperLayer,
};

void Packet_init(Packet* frame, uint8_t* data, uint32_t size) {
    frame->Data = data;
    frame->Size = size;
}
uint32_t Packet_len(Packet* frame) {
    return frame->Size + PACKET_HEADER_SIZE + PACKET_FOOTER_SIZE;
}
Codec_LayerImpl* Packet_baseLayer(void) {
    return (Codec_LayerImpl*) &PACKET_HEADER_IMPL;
}

Stream_LenType Packet_sync(Codec* codec, IStream* stream) {
    __setByteOrder(stream);
    return IStream_findUInt16(stream, __PACKET_FIRST_SIGN);
}

#if CODEC_DECODE

static Codec_Error Packet_Header_parse(Codec* codec, Codec_Frame* frame, IStream* stream) {
    Packet* p = (Packet*) frame;
    __setByteOrder(stream);
    if (IStream_readUInt16(stream) != __PACKET_FIRST_SIGN) {
        return (Codec_Error) Packet_Error_FirstSign;
    }
    p->Size = IStream_readUInt32(stream);
    if (p->Size >= PACKET_MAX_SIZE) {
        return (Codec_Error) Packet_Error_PacketSize;
    }
    if (IStream_readUInt16(stream) != __PACKET_SECOND_SIGN) {
        return (Codec_Error) Packet_Error_SecondSign;
    }
    return CODEC_OK;
}
static Codec_Error Packet_Data_parse(Codec* codec, Codec_Frame* frame, IStream* stream) {
    Packet* p = (Packet*) frame;
    if (IStream_available(stream) < p->Size) {
        return (Codec_Error) Packet_Error_Data;
    }
    if (p->Data == NULL) {
        return (Codec_Error) Packet_Error_DataPtr;
    }
    IStream_readBytes(stream, p->Data, p->Size);
    return CODEC_OK;
}
static Codec_Error Packet_Footer_parse(Codec* codec, Codec_Frame* frame, IStream* stream) {
    Packet* p = (Packet*) frame;
    __setByteOrder(stream);
    if (IStream_readUInt32(stream) != __PACKET_FOOTER_SIGN) {
        return (Codec_Error) Packet_Error_FooterSign;
    }
    return CODEC_OK;
}
#endif // CODEC_DECODE

#if CODEC_ENCODE

static Codec_Error Packet_Header_write(Codec* codec, Codec_Frame* frame, OStream* stream) {
    Packet* p = (Packet*) frame;
    __setByteOrder(stream);
    OStream_writeUInt16(stream, __PACKET_FIRST_SIGN);
    OStream_writeUInt32(stream, p->Size);
    OStream_writeUInt16(stream, __PACKET_SECOND_SIGN);
    return CODEC_OK;
}
static Codec_Error Packet_Data_write(Codec* codec, Codec_Frame* frame, OStream* stream) {
    Packet* p = (Packet*) frame;
    OStream_writeBytes(stream, p->Data, p->Size);
    return CODEC_OK;
}
static Codec_Error Packet_Footer_write(Codec* codec, Codec_Frame* frame, OStream* stream) {
    __setByteOrder(stream);
    OStream_writeUInt32(stream, __PACKET_FOOTER_SIGN);
    return CODEC_OK;
}

#endif // CODEC_ENCODE


static Stream_LenType Packet_Header_getLen(Codec* codec, Codec_Frame* frame) {
    return PACKET_HEADER_SIZE;
}
static Codec_LayerImpl* Packet_Header_getUpperLayer(Codec* codec, Codec_Frame* frame) {
    return (Codec_LayerImpl*) &PACKET_DATA_IMPL;
}

static Stream_LenType Packet_Data_getLen(Codec* codec, Codec_Frame* frame) {
    Packet* p = (Packet*) frame;
    return p->Size;
}
static Codec_LayerImpl* Packet_Data_getUpperLayer(Codec* codec, Codec_Frame* frame) {
    return (Codec_LayerImpl*) &PACKET_FOOTER_IMPL;
}

static Stream_LenType Packet_Footer_getLen(Codec* codec, Codec_Frame* frame) {
    return PACKET_FOOTER_SIZE;
}
static Codec_LayerImpl* Packet_Footer_getUpperLayer(Codec* codec, Codec_Frame* frame) {
    return NULL;
}

