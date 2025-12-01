#include <stdio.h>

#include "InputStream.h"
#include "OutputStream.h"
#include "Codec.h"

// Print functions
#define PRINTF          printf
#define PUTS            puts
#define PUTCHAR         putchar


typedef struct {
    uint32_t    PacketSize;
} Frame_Header;

typedef struct {
    Frame_Header    Header;
    uint8_t         Data[20];
} Frame;

// Header Impl functions
Codec_Error Frame_parseHeader(Codec* codec, Codec_Frame* frame, StreamIn* stream);
Codec_Error Frame_writeHeader(Codec* codec, Codec_Frame* frame, StreamOut* stream);
Stream_LenType Frame_getHeaderLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
Codec_LayerImpl* Frame_getHeaderNextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
// Data Impl functions
Codec_Error Frame_parseData(Codec* codec, Codec_Frame* frame, StreamIn* stream);
Codec_Error Frame_writeData(Codec* codec, Codec_Frame* frame, StreamOut* stream);
Stream_LenType Frame_getDataSize(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
Codec_LayerImpl* Frame_getDataNextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
// Footer Impl functions
Codec_Error Frame_parseFooter(Codec* codec, Codec_Frame* frame, StreamIn* stream);
Codec_Error Frame_writeFooter(Codec* codec, Codec_Frame* frame, StreamOut* stream);
Stream_LenType Frame_getFooterLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
Codec_LayerImpl* Frame_getFooterNextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase);


const Codec_LayerImpl FRAME_LAYER_HEADER = {
    Frame_parseHeader,
    Frame_writeHeader,
    Frame_getHeaderLen,
    Frame_getHeaderNextLayer,
};

const Codec_LayerImpl FRAME_LAYER_DATA = {
    Frame_parseData,
    Frame_writeData,
    Frame_getDataSize,
    Frame_getDataNextLayer,
};
const Codec_LayerImpl FRAME_LAYER_FOOTER = {
    Frame_parseFooter,
    Frame_writeFooter,
    Frame_getFooterLen,
    Frame_getFooterNextLayer,
};

// Frame on decode callback
void Frame_onDetect(Codec* codec, Codec_Frame* frame);
// Frame on error callback
void Frame_onError(Codec* codec, Codec_Frame* frame, Codec_LayerImpl* layer, Codec_Error error);

Frame tempFrame = {0};

void printArray(uint8_t* buff, int len);

int main() {
    Frame mFrame;
    uint8_t inBuff[40] = {0};
    uint8_t outBuff[40] = {0};
    StreamOut out;
    StreamIn in;
    Codec codec;
    Codec_init(&codec, (Codec_LayerImpl*) &FRAME_LAYER_HEADER);
    Codec_onDecode(&codec, Frame_onDetect);
    Codec_onDecodeError(&codec, Frame_onError);

    IStream_init(&in, NULL, inBuff, sizeof(inBuff));
    OStream_init(&out, NULL, outBuff, sizeof(outBuff));

    mFrame.Header.PacketSize = 5;
    mFrame.Data[0] = 0xAA;
    mFrame.Data[1] = 0xBB;
    mFrame.Data[2] = 0xCC;
    mFrame.Data[3] = 0xDD;
    mFrame.Data[4] = 0xEE;

    Codec_encodeFrame(&codec, &mFrame, &out, Codec_EncodeMode_Flush);
    printArray(outBuff, sizeof(outBuff));

    Stream_readStream(&out.Buffer, &in.Buffer, Stream_available(&out.Buffer));
    printArray(inBuff, sizeof(inBuff));
    Codec_decodeFrame(&codec, &tempFrame, &in);
}

void Frame_onDetect(Codec* codec, Codec_Frame* frame) {
    Frame* myFrame = (Frame*) frame;

    PRINTF("PacketSize: %d\n", myFrame->Header.PacketSize);
    printArray(myFrame->Data, myFrame->Header.PacketSize);
}
void Frame_onError(Codec* codec, Codec_Frame* frame, Codec_LayerImpl* layer, Codec_Error error) {
    Frame* myFrame = (Frame*) frame;

    PRINTF("Frame Error: %u\n", error);
}

/************************************** Header Impl *************************************************/
Codec_Error Frame_parseHeader(Codec* codec, Codec_Frame* frame, StreamIn* stream) {
    Frame* myFrame = (Frame*) frame;
    IStream_setByteOrder(stream, ByteOrder_BigEndian);

    if (IStream_readUInt16(stream) != 0x1234) {
        return 1;
    }
    myFrame->Header.PacketSize = IStream_readUInt32(stream);
    if (IStream_readUInt16(stream) != 0x4321) {
        return 2;
    }

    return CODEC_OK;
}
Codec_Error Frame_writeHeader(Codec* codec, Codec_Frame* frame, StreamOut* stream) {
    Frame* myFrame = (Frame*) frame;
    OStream_setByteOrder(stream, ByteOrder_BigEndian);

    OStream_writeUInt16(stream, 0x1234);
    OStream_writeUInt32(stream, myFrame->Header.PacketSize);
    OStream_writeUInt16(stream, 0x4321);

    return CODEC_OK;
}
Stream_LenType Frame_getHeaderLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return 8;
}
Codec_LayerImpl* Frame_getHeaderNextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return (Codec_LayerImpl*) &FRAME_LAYER_DATA;
}
/************************************** Data Impl *************************************************/
Codec_Error Frame_parseData(Codec* codec, Codec_Frame* frame, StreamIn* stream) {
    Frame* myFrame = (Frame*) frame;
    IStream_setByteOrder(stream, ByteOrder_BigEndian);

    IStream_readBytes(stream, myFrame->Data, myFrame->Header.PacketSize);

    return CODEC_OK;
}
Codec_Error Frame_writeData(Codec* codec, Codec_Frame* frame, StreamOut* stream) {
    Frame* myFrame = (Frame*) frame;
    OStream_setByteOrder(stream, ByteOrder_BigEndian);

    OStream_writeBytes(stream, myFrame->Data, myFrame->Header.PacketSize);

    return CODEC_OK;
}
Stream_LenType Frame_getDataSize(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return ((Frame*) frame)->Header.PacketSize;
}
Codec_LayerImpl* Frame_getDataNextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return (Codec_LayerImpl*) &FRAME_LAYER_FOOTER;
}
/************************************** Footer Impl *************************************************/
Codec_Error Frame_parseFooter(Codec* codec, Codec_Frame* frame, StreamIn* stream) {
    IStream_setByteOrder(stream, ByteOrder_BigEndian);
    return IStream_readUInt32(stream) != 0xAA55CC33;
}
Codec_Error Frame_writeFooter(Codec* codec, Codec_Frame* frame, StreamOut* stream) {
    OStream_setByteOrder(stream, ByteOrder_BigEndian);
    OStream_writeUInt32(stream, 0xAA55CC33);

    return CODEC_OK;
}
Stream_LenType Frame_getFooterLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return 4;
}
Codec_LayerImpl* Frame_getFooterNextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return CODEC_LAYER_NULL;
}

/************************************** Utils *************************************************/

void printArray(uint8_t* buff, int len) {
    PUTCHAR('[');
    while (len-- > 0) {
        PRINTF("0x%02X, ", *buff++);
    }
    PUTS("]");
}

