#include <stdio.h>

#include <string.h>
#include "Codec.h"
#include "StreamBuffer.h"
#include "InputStream.h"
#include "OutputStream.h"

#include "Frame/BasicFrame.h"
#include "Frame/Packet.h"

#define PUTCHAR                 putchar
#define PUTS                    puts
#define PRINTF                  printf


#define CYCLES_NUM              20

uint8_t  cycles;
uint8_t  index;
uint32_t errorCode;

uint32_t Assert_Num(int a, int b, uint16_t line, uint8_t cycles, uint8_t index);
uint32_t Assert_Status(Codec_Status a, Codec_Status b, uint16_t line, uint8_t cycles, uint8_t index);
uint32_t Assert_Bytes(uint8_t* a, uint8_t* b, int len, uint16_t line, uint8_t cycles, uint8_t index);
uint32_t Assert_BasicFrame(BasicFrame* a, BasicFrame* b, uint16_t line, uint8_t cycles, uint8_t index);
uint32_t Assert_Packet(Packet* a, Packet* b, uint16_t line, uint8_t cycles, uint8_t index);

#define assert(TYPE, ...)               if ((errorCode = Assert_ ##TYPE (__VA_ARGS__, __LINE__, cycles, index))) return errorCode;
#define compress(LINE, CYCLE, INDEX)    (((uint32_t) LINE << 16) | ((uint32_t) CYCLE << 8) | ((uint32_t) INDEX))
#define decompres(RESULT)               RESULT >> 16, (RESULT >> 8) & 0xFF, RESULT & 0xFF

typedef uint32_t (*TestFn)(void);

uint32_t Test_Frame_BasicFrame(void);
uint32_t Test_Buffer_BasicFrame(void);
uint32_t Test_Frame_Packet(void);
uint32_t Test_Buffer_Packet(void);
uint32_t Test_Frame_Noise_Packet(void);
uint32_t Test_Frame_Sync_Packet(void);
uint32_t Test_Async_Noise_Packet(void);
uint32_t Test_Async_Sync_Packet(void);

static const TestFn TESTS[] = {
    Test_Frame_BasicFrame,
    Test_Buffer_BasicFrame,
    Test_Frame_Packet,
    Test_Buffer_Packet,
    Test_Frame_Noise_Packet,
    Test_Frame_Sync_Packet,
    Test_Async_Noise_Packet,
    Test_Async_Sync_Packet,
};
static const uint32_t TESTS_LEN = sizeof(TESTS) / sizeof(TESTS[0]);

int main()
{
    int i;
    uint32_t result;
    uint32_t errorCount = 0;

    PUTS("------- Start Codec Tests -------");
    for (i = 0; i < TESTS_LEN; i++) {
        result = TESTS[i]();
        if (result) {
            PRINTF("Line: %u, Cycle: %u, Index: %u\n",
                   decompres(result));
            errorCount++;
            PRINTF("%u: Test Failed\n", i);
        }
        else {
            PRINTF("%u: Test Pass\n", i);
        }
        PUTS("-------------------------");
    }
    PRINTF("\nTest Ended, %u Error Counts\n", errorCount);
}

uint32_t Test_Frame_BasicFrame(void) {

    #define testBasicFrame(PAT, N)          PRINTF(#PAT " %dx\n", N);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    BasicFrame_init(&frame, PAT, sizeof(PAT));\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * BasicFrame_len(&frame));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    status = Codec_decodeFrame(&codec, &tempFrame, &istream);\
                                                    assert(Status, status, Codec_Status_Done);\
                                                    assert(BasicFrame, &tempFrame, &frame);\
                                                }\
                                            }


    static const uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static const uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static const uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static const uint8_t PAT4[2] = {0x3A, 0x3B};
    static const uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    BasicFrame frame;
    BasicFrame tempFrame;

    uint8_t txBuff[37];
    uint8_t rxBuff[37];
    uint8_t tempBuff[30];

    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, BasicFrame_baseLayer());
    BasicFrame_init(&tempFrame, tempBuff, sizeof(tempBuff));

    testBasicFrame(PAT1, 1);
    testBasicFrame(PAT1, 2);
    testBasicFrame(PAT1, 3);

    testBasicFrame(PAT2, 1);
    testBasicFrame(PAT2, 2);
    testBasicFrame(PAT2, 3);

    testBasicFrame(PAT3, 1);
    testBasicFrame(PAT3, 2);
    testBasicFrame(PAT3, 3);

    testBasicFrame(PAT4, 1);
    testBasicFrame(PAT4, 2);
    testBasicFrame(PAT4, 3);

    testBasicFrame(PAT5, 1);
    testBasicFrame(PAT5, 2);
    testBasicFrame(PAT5, 3);

    return 0;
}
uint32_t Test_Buffer_BasicFrame(void) {
    #define testBasicFrame(PAT)             PRINTF(#PAT "\n");\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                BasicFrame_init(&frame, PAT, sizeof(PAT));\
                                                status = Codec_encodeBuffer(&codec, &frame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                status = Codec_decodeBuffer(&codec, &tempFrame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                assert(BasicFrame, &tempFrame, &frame);\
                                            }


    static const uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static const uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static const uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static const uint8_t PAT4[2] = {0x3A, 0x3B};
    static const uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;

    Codec codec;
    BasicFrame frame;
    BasicFrame tempFrame;

    uint8_t buffer[37];
    uint8_t tempBuff[30];


    Codec_init(&codec, BasicFrame_baseLayer());
    BasicFrame_init(&tempFrame, tempBuff, sizeof(tempBuff));

    index = 0;

    testBasicFrame(PAT1);

    testBasicFrame(PAT2);

    testBasicFrame(PAT3);

    testBasicFrame(PAT4);

    testBasicFrame(PAT5);

    return 0;
}
uint32_t Test_Frame_Packet(void) {
    #define testPacket(PAT, N)              PRINTF(#PAT " %dx\n", N);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * Packet_len(&frame));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    status = Codec_decodeFrame(&codec, &tempFrame, &istream);\
                                                    assert(Status, status, Codec_Status_Done);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }


    static const uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static const uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static const uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static const uint8_t PAT4[2] = {0x3A, 0x3B};
    static const uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[60];
    uint8_t rxBuff[60];
    uint8_t tempBuff[30];


    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, Packet_baseLayer());
    Packet_init(&tempFrame, tempBuff, sizeof(tempBuff));


    testPacket(PAT1, 1);
    testPacket(PAT1, 2);
    testPacket(PAT1, 3);

    testPacket(PAT2, 1);
    testPacket(PAT2, 2);
    testPacket(PAT2, 3);

    testPacket(PAT3, 1);
    testPacket(PAT3, 2);
    testPacket(PAT3, 3);

    testPacket(PAT4, 1);
    testPacket(PAT4, 2);
    testPacket(PAT4, 3);

    testPacket(PAT5, 1);
    testPacket(PAT5, 2);
    testPacket(PAT5, 3);

    return 0;
}
uint32_t Test_Buffer_Packet(void) {
    #define testPacket(PAT)                 PRINTF(#PAT "\n");\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                Packet_init(&frame, PAT, sizeof(PAT));\
                                                status = Codec_encodeBuffer(&codec, &frame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                status = Codec_decodeBuffer(&codec, &tempFrame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                assert(Packet, &tempFrame, &frame);\
                                            }

    static const uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static const uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static const uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static const uint8_t PAT4[2] = {0x3A, 0x3B};
    static const uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t buffer[37];
    uint8_t tempBuff[30];

    Codec_init(&codec, Packet_baseLayer());
    Packet_init(&tempFrame, tempBuff, sizeof(tempBuff));

    index = 0;

    testPacket(PAT1);

    testPacket(PAT2);

    testPacket(PAT3);

    testPacket(PAT4);

    testPacket(PAT5);

    return 0;
}
uint32_t Test_Frame_Noise_Packet(void) {
    #define addNoise(STREAM, N)             OStream_writePadding(STREAM, 0xFF, N)
    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx - Noise: %d\n", N, S);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    status = Codec_decodeFrame(&codec, &tempFrame, &istream);\
                                                    assert(Status, status, Codec_Status_Done);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }

    static const uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static const uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static const uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static const uint8_t PAT4[2] = {0x3A, 0x3B};
    static const uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[80];
    uint8_t rxBuff[80];
    uint8_t tempBuff[30];


    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, Packet_baseLayer());
    Packet_init(&tempFrame, tempBuff, sizeof(tempBuff));

    testPacket(PAT1, 1, 1);
    testPacket(PAT1, 1, 3);
    testPacket(PAT1, 1, 7);
    testPacket(PAT1, 2, 1);
    testPacket(PAT1, 2, 3);
    testPacket(PAT1, 2, 7);
    testPacket(PAT1, 3, 1);
    testPacket(PAT1, 3, 3);
    testPacket(PAT1, 3, 7);

    testPacket(PAT2, 1, 1);
    testPacket(PAT2, 1, 3);
    testPacket(PAT2, 1, 7);
    testPacket(PAT2, 2, 1);
    testPacket(PAT2, 2, 3);
    testPacket(PAT2, 2, 7);
    testPacket(PAT2, 3, 1);
    testPacket(PAT2, 3, 3);
    testPacket(PAT2, 3, 7);

    testPacket(PAT3, 1, 1);
    testPacket(PAT3, 1, 3);
    testPacket(PAT3, 1, 7);
    testPacket(PAT3, 2, 1);
    testPacket(PAT3, 2, 3);
    testPacket(PAT3, 2, 7);
    testPacket(PAT3, 3, 1);
    testPacket(PAT3, 3, 3);
    testPacket(PAT3, 3, 7);

    testPacket(PAT4, 1, 1);
    testPacket(PAT4, 1, 3);
    testPacket(PAT4, 1, 7);
    testPacket(PAT4, 2, 1);
    testPacket(PAT4, 2, 3);
    testPacket(PAT4, 2, 7);
    testPacket(PAT4, 3, 1);
    testPacket(PAT4, 3, 3);
    testPacket(PAT4, 3, 7);

    testPacket(PAT5, 1, 1);
    testPacket(PAT5, 1, 3);
    testPacket(PAT5, 1, 7);
    testPacket(PAT5, 2, 1);
    testPacket(PAT5, 2, 3);
    testPacket(PAT5, 2, 7);
    testPacket(PAT5, 3, 1);
    testPacket(PAT5, 3, 3);
    testPacket(PAT5, 3, 7);

    return 0;
}
uint32_t Test_Frame_Sync_Packet(void) {
    #define addNoise(STREAM, N)             OStream_writePadding(STREAM, 0xFF, N)
    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx - Noise: %d\n", N, S);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    status = Codec_decodeFrame(&codec, &tempFrame, &istream);\
                                                    assert(Status, status, Codec_Status_Done);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }

    static const uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static const uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static const uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static const uint8_t PAT4[2] = {0x3A, 0x3B};
    static const uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[80];
    uint8_t rxBuff[80];
    uint8_t tempBuff[30];


    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, Packet_baseLayer());
    Codec_setDecodeSync(&codec, Packet_sync);
    Packet_init(&tempFrame, tempBuff, sizeof(tempBuff));


    testPacket(PAT1, 1, 1);
    testPacket(PAT1, 1, 3);
    testPacket(PAT1, 1, 7);
    testPacket(PAT1, 2, 1);
    testPacket(PAT1, 2, 3);
    testPacket(PAT1, 2, 7);
    testPacket(PAT1, 3, 1);
    testPacket(PAT1, 3, 3);
    testPacket(PAT1, 3, 7);

    testPacket(PAT2, 1, 1);
    testPacket(PAT2, 1, 3);
    testPacket(PAT2, 1, 7);
    testPacket(PAT2, 2, 1);
    testPacket(PAT2, 2, 3);
    testPacket(PAT2, 2, 7);
    testPacket(PAT2, 3, 1);
    testPacket(PAT2, 3, 3);
    testPacket(PAT2, 3, 7);

    testPacket(PAT3, 1, 1);
    testPacket(PAT3, 1, 3);
    testPacket(PAT3, 1, 7);
    testPacket(PAT3, 2, 1);
    testPacket(PAT3, 2, 3);
    testPacket(PAT3, 2, 7);
    testPacket(PAT3, 3, 1);
    testPacket(PAT3, 3, 3);
    testPacket(PAT3, 3, 7);

    testPacket(PAT4, 1, 1);
    testPacket(PAT4, 1, 3);
    testPacket(PAT4, 1, 7);
    testPacket(PAT4, 2, 1);
    testPacket(PAT4, 2, 3);
    testPacket(PAT4, 2, 7);
    testPacket(PAT4, 3, 1);
    testPacket(PAT4, 3, 3);
    testPacket(PAT4, 3, 7);

    testPacket(PAT5, 1, 1);
    testPacket(PAT5, 1, 3);
    testPacket(PAT5, 1, 7);
    testPacket(PAT5, 2, 1);
    testPacket(PAT5, 2, 3);
    testPacket(PAT5, 2, 7);
    testPacket(PAT5, 3, 1);
    testPacket(PAT5, 3, 3);
    testPacket(PAT5, 3, 7);

    return 0;
}
uint32_t Test_Async_Noise_Packet(void) {
    #define addNoise(STREAM, N)             OStream_writePadding(STREAM, 0xFF, N)
#if !CODEC_DECODE_CONTINUOUS
    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx - Noise: %d\n", N, S);\
                                            Codec_beginDecode(&codec, &tempFrame);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    Codec_decode(&codec, &istream);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }
#else
    #define testPacket(PAT, N)              PRINTF(#PAT " %dx\n", N);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * Packet_len(&frame));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    Codec_decode(&codec, &tempFrame, &istream);\
                                                    assert(Status, status, Codec_Status_Done);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }
#endif


    static const uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static const uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static const uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static const uint8_t PAT4[2] = {0x3A, 0x3B};
    static const uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[80];
    uint8_t rxBuff[80];
    uint8_t tempBuff[30];


    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, Packet_baseLayer());
    Packet_init(&tempFrame, tempBuff, sizeof(tempBuff));


    testPacket(PAT1, 1, 1);
    testPacket(PAT1, 1, 3);
    testPacket(PAT1, 1, 7);
    testPacket(PAT1, 2, 1);
    testPacket(PAT1, 2, 3);
    testPacket(PAT1, 2, 7);
    testPacket(PAT1, 3, 1);
    testPacket(PAT1, 3, 3);
    testPacket(PAT1, 3, 7);

    testPacket(PAT2, 1, 1);
    testPacket(PAT2, 1, 3);
    testPacket(PAT2, 1, 7);
    testPacket(PAT2, 2, 1);
    testPacket(PAT2, 2, 3);
    testPacket(PAT2, 2, 7);
    testPacket(PAT2, 3, 1);
    testPacket(PAT2, 3, 3);
    testPacket(PAT2, 3, 7);

    testPacket(PAT3, 1, 1);
    testPacket(PAT3, 1, 3);
    testPacket(PAT3, 1, 7);
    testPacket(PAT3, 2, 1);
    testPacket(PAT3, 2, 3);
    testPacket(PAT3, 2, 7);
    testPacket(PAT3, 3, 1);
    testPacket(PAT3, 3, 3);
    testPacket(PAT3, 3, 7);

    testPacket(PAT4, 1, 1);
    testPacket(PAT4, 1, 3);
    testPacket(PAT4, 1, 7);
    testPacket(PAT4, 2, 1);
    testPacket(PAT4, 2, 3);
    testPacket(PAT4, 2, 7);
    testPacket(PAT4, 3, 1);
    testPacket(PAT4, 3, 3);
    testPacket(PAT4, 3, 7);

    testPacket(PAT5, 1, 1);
    testPacket(PAT5, 1, 3);
    testPacket(PAT5, 1, 7);
    testPacket(PAT5, 2, 1);
    testPacket(PAT5, 2, 3);
    testPacket(PAT5, 2, 7);
    testPacket(PAT5, 3, 1);
    testPacket(PAT5, 3, 3);
    testPacket(PAT5, 3, 7);

    return 0;
}
uint32_t Test_Async_Sync_Packet(void) {
    #define addNoise(STREAM, N)             OStream_writePadding(STREAM, 0xFF, N)
#if !CODEC_DECODE_CONTINUOUS
    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx - Noise: %d\n", N, S);\
                                            Codec_beginDecode(&codec, &tempFrame);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    Codec_decode(&codec, &istream);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }
#else
    #define testPacket(PAT, N)              PRINTF(#PAT " %dx\n", N);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * Packet_len(&frame));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    Codec_decode(&codec, &tempFrame, &istream);\
                                                    assert(Status, status, Codec_Status_Done);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }
#endif


    static const uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static const uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static const uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static const uint8_t PAT4[2] = {0x3A, 0x3B};
    static const uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[80];
    uint8_t rxBuff[80];
    uint8_t tempBuff[30];


    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, Packet_baseLayer());
    Codec_setDecodeSync(&codec, Packet_sync);
    Packet_init(&tempFrame, tempBuff, sizeof(tempBuff));


    testPacket(PAT1, 1, 1);
    testPacket(PAT1, 1, 3);
    testPacket(PAT1, 1, 7);
    testPacket(PAT1, 2, 1);
    testPacket(PAT1, 2, 3);
    testPacket(PAT1, 2, 7);
    testPacket(PAT1, 3, 1);
    testPacket(PAT1, 3, 3);
    testPacket(PAT1, 3, 7);

    testPacket(PAT2, 1, 1);
    testPacket(PAT2, 1, 3);
    testPacket(PAT2, 1, 7);
    testPacket(PAT2, 2, 1);
    testPacket(PAT2, 2, 3);
    testPacket(PAT2, 2, 7);
    testPacket(PAT2, 3, 1);
    testPacket(PAT2, 3, 3);
    testPacket(PAT2, 3, 7);

    testPacket(PAT3, 1, 1);
    testPacket(PAT3, 1, 3);
    testPacket(PAT3, 1, 7);
    testPacket(PAT3, 2, 1);
    testPacket(PAT3, 2, 3);
    testPacket(PAT3, 2, 7);
    testPacket(PAT3, 3, 1);
    testPacket(PAT3, 3, 3);
    testPacket(PAT3, 3, 7);

    testPacket(PAT4, 1, 1);
    testPacket(PAT4, 1, 3);
    testPacket(PAT4, 1, 7);
    testPacket(PAT4, 2, 1);
    testPacket(PAT4, 2, 3);
    testPacket(PAT4, 2, 7);
    testPacket(PAT4, 3, 1);
    testPacket(PAT4, 3, 3);
    testPacket(PAT4, 3, 7);

    testPacket(PAT5, 1, 1);
    testPacket(PAT5, 1, 3);
    testPacket(PAT5, 1, 7);
    testPacket(PAT5, 2, 1);
    testPacket(PAT5, 2, 3);
    testPacket(PAT5, 2, 7);
    testPacket(PAT5, 3, 1);
    testPacket(PAT5, 3, 3);
    testPacket(PAT5, 3, 7);

    return 0;
}


// -------------------------- Assert Functions ------------------------
void printArray(uint8_t* arr, int len) {
    PUTCHAR('{');
    while (--len > 0) {
        PRINTF("0x%02X, ", *arr++);
    }
    PRINTF("0x%02X", *arr);
    PUTS("}");
}
uint32_t Assert_Num(int a, int b, uint16_t line, uint8_t cycles, uint8_t index) {
    if (a != b) {
        PRINTF("[Num] Expected: %d, Found: %d\n", b, a);
        return compress(line, cycles, index);
    }
    return 0;
}
uint32_t Assert_Status(Codec_Status a, Codec_Status b, uint16_t line, uint8_t cycles, uint8_t index) {
    if (a != b) {
        PRINTF("[Status] Expected: %d, Found: %d\n", b, a);
        return compress(line, cycles, index);
    }
    return 0;
}
uint32_t Assert_Bytes(uint8_t* a, uint8_t* b, int len, uint16_t line, uint8_t cycles, uint8_t index) {
    if (memcmp(a, b, len) == 0) {
        PRINTF("[Bytes] Expected: ");
        printArray(b, len);
        PRINTF("\n        Found: ");
        printArray(a, len);
        PUTS("");

        return compress(line, cycles, index);
    }
    return 0;
}
uint32_t Assert_BasicFrame(BasicFrame* a, BasicFrame* b, uint16_t line, uint8_t cycles, uint8_t index) {
    if (!(a->Header.PacketSize == b->Header.PacketSize &&
        memcmp(a->Data.Data, b->Data.Data, a->Header.PacketSize) == 0)) {
        PRINTF("[BasicFrame] Expected Len: %d, Found Len: %d\n",
               b->Header.PacketSize, a->Header.PacketSize);
        PRINTF("    Expected: ");
        printArray(b->Data.Data, b->Header.PacketSize);
        PRINTF("\n    Found: ");
        printArray(a->Data.Data, a->Header.PacketSize);
        PUTS("");

        return compress(line, cycles, index);
    }
    return 0;
}
uint32_t Assert_Packet(Packet* a, Packet* b, uint16_t line, uint8_t cycles, uint8_t index) {
    if (!(a->Size == b->Size &&
        memcmp(a->Data, b->Data, a->Size) == 0)) {
        PRINTF("[Packet] Expected Len: %d, Found Len: %d\n",
               b->Size, a->Size);
        PRINTF("    Expected: ");
        printArray(b->Data, b->Size);
        PRINTF("\n    Found: ");
        printArray(a->Data, a->Size);
        PUTS("");

        return compress(line, cycles, index);
    }
    return 0;
}
