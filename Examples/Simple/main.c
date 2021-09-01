#include <stdio.h>
#include <stdlib.h>

#include "InputStream.h"
#include "OutputStream.h"
#include "Codec.h"


typedef struct {
    uint32_t    PacketSize;
} Frame_Header;

typedef struct {
    Frame_Header    Header;
    uint8_t         Data[20];
} Frame;

Stream_LenType Frame_getHeaderLen(Codec* codec, Codec_Frame* frame);
Stream_LenType Frame_getPacketSize(Codec* codec, Codec_Frame* frame);
Stream_LenType Frame_getFooterLen(Codec* codec, Codec_Frame* frame);
Codec_Result Frame_parseHeader(Codec* codec, Codec_Frame* frame, IStream* stream);
Codec_Result Frame_parseData(Codec* codec, Codec_Frame* frame, IStream* stream);
Codec_Result Frame_parseFooter(Codec* codec, Codec_Frame* frame, IStream* stream);
Codec_Result Frame_writeHeader(Codec* codec, Codec_Frame* frame, OStream* stream);
Codec_Result Frame_writeData(Codec* codec, Codec_Frame* frame, OStream* stream);
Codec_Result Frame_writeFooter(Codec* codec, Codec_Frame* frame, OStream* stream);
void Frame_onDetect(Codec* codec, Codec_Frame* frame);

const Codec_HeaderImpl HeaderImpl = {
    Frame_getHeaderLen,
    Frame_getPacketSize,
    NULL,
    Frame_parseHeader,
    Frame_writeHeader,
};
const Codec_DataImpl DataImpl = {
    Frame_parseData,
    Frame_writeData,
};
const Codec_FooterImpl FooterImpl = {
    Frame_getFooterLen,
    Frame_parseFooter,
    Frame_writeFooter,
};

Frame tempFrame = {0};

void printArray(uint8_t* buff, int len);

int main() {
    Frame mFrame;
    uint8_t inBuff[40] = {0};
    uint8_t outBuff[40] = {0};
    OStream out;
    IStream in;
    Codec codec;
    Codec_init(&codec, &HeaderImpl, &DataImpl, &FooterImpl, &tempFrame);
    codec.onFrame = Frame_onDetect;

    IStream_init(&in, NULL, inBuff, sizeof(inBuff));
    OStream_init(&out, NULL, outBuff, sizeof(outBuff));

    mFrame.Header.PacketSize = 5;
    mFrame.Data[0] = 0xAA;
    mFrame.Data[1] = 0xBB;
    mFrame.Data[2] = 0xCC;
    mFrame.Data[3] = 0xDD;
    mFrame.Data[4] = 0xEE;

    Codec_encode(&codec, &out, &mFrame);
    printArray(outBuff, sizeof(outBuff));

    Stream_readStream(&out.Buffer, &in.Buffer, Stream_available(&out.Buffer));
    printArray(inBuff, sizeof(inBuff));
    Codec_decode(&codec, &in);
}

void Frame_onDetect(Codec* codec, Codec_Frame* frame) {
    Frame* myFrame = (Frame*) frame;

    printf("PacketSize: %d\n", myFrame->Header.PacketSize);
    printArray(myFrame->Data, myFrame->Header.PacketSize);
}

Stream_LenType Frame_getHeaderLen(Codec* codec, Codec_Frame* frame) {
    return 8;
}
Stream_LenType Frame_getPacketSize(Codec* codec, Codec_Frame* frame) {
    return ((Frame*) frame)->Header.PacketSize;
}
Stream_LenType Frame_getFooterLen(Codec* codec, Codec_Frame* frame) {
    return 4;
}
Codec_Result Frame_parseHeader(Codec* codec, Codec_Frame* frame, IStream* stream) {
    Frame* myFrame = (Frame*) frame;

    if (IStream_readUInt16(stream) != 0x1234) {
        return Codec_Error;
    }
    myFrame->Header.PacketSize = IStream_readUInt32(stream);
    if (IStream_readUInt16(stream) != 0x4321) {
        return Codec_Error;
    }

    return Codec_Ok;
}
Codec_Result Frame_parseData(Codec* codec, Codec_Frame* frame, IStream* stream) {
    Frame* myFrame = (Frame*) frame;

    IStream_readBytes(stream, myFrame->Data, myFrame->Header.PacketSize);

    return Codec_Ok;
}
Codec_Result Frame_parseFooter(Codec* codec, Codec_Frame* frame, IStream* stream) {
    return IStream_readUInt32(stream) != 0xAA55CC33;
}
Codec_Result Frame_writeHeader(Codec* codec, Codec_Frame* frame, OStream* stream) {
    Frame* myFrame = (Frame*) frame;
    OStream_writeUInt16(stream, 0x1234);
    OStream_writeUInt32(stream, myFrame->Header.PacketSize);
    OStream_writeUInt16(stream, 0x4321);

    return Codec_Ok;
}
Codec_Result Frame_writeData(Codec* codec, Codec_Frame* frame, OStream* stream) {
    Frame* myFrame = (Frame*) frame;

    OStream_writeBytes(stream, myFrame->Data, myFrame->Header.PacketSize);

    return Codec_Ok;
}
Codec_Result Frame_writeFooter(Codec* codec, Codec_Frame* frame, OStream* stream) {
    OStream_writeUInt32(stream, 0xAA55CC33);

    return Codec_Ok;
}

void printArray(uint8_t* buff, int len) {
    putchar('[');
    while (len-- > 0) {
        printf("0x%02X, ", *buff++);
    }
    puts("]");
}

