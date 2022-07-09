#include <stdio.h>
#include <stdint.h>

#include "Codec.h"
#include "BasicFrame.h"

#define FRAME_NUM           10

#define PRINTF              printf

void BasicFrame_onDecode(Codec* codec, Codec_Frame* frame);

int main()
{
    static BasicFrame frame;
    static uint8_t frameBuff[100];
    int num = FRAME_NUM;
    uint8_t outBuff[40];
    uint8_t inBuff[40];
    uint8_t temp[5] = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E};

    Codec codec;
    IStream in;
    OStream out;

    // init decode frame
    BasicFrame_init(&frame, frameBuff, sizeof(frameBuff));
    // init codec
    Codec_init(&codec, &BASIC_FRAME_IMPL, &frame);
    Codec_onFrame(&codec, BasicFrame_onDecode);
    // init output stream
    OStream_init(&out, NULL, outBuff, sizeof(outBuff));
    // init input stream
    IStream_init(&in, NULL, inBuff, sizeof(inBuff));


    while (num-- > 0) {
        // encode and send
        BasicFrame outFrame;
        BasicFrame_init(&outFrame, temp, sizeof(temp));
        Codec_encode(&codec, &outFrame, &out, Codec_EncodeMode_Flush);
        // write output stream to input stream
        Stream_readStream(&out.Buffer, &in.Buffer, Stream_available(&out.Buffer));
        // decode
        Codec_decode(&codec, &in);
    }


}

void BasicFrame_onDecode(Codec* codec, Codec_Frame* frame) {
    int i;
    BasicFrame* bFrame = (BasicFrame*) frame;

    PRINTF("\nPacket Size: %u\n", bFrame->Header.PacketSize);
    PRINTF("[");
    for (i = 0; i < bFrame->Header.PacketSize; i++) {
        PRINTF("0x%02X, ", bFrame->Data.Data[i]);
    }
    PRINTF("]\n");

}
