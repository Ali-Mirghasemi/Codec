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

// ------------------------ Custom Frame -------------------------
typedef struct {
    uint32_t            PacketSize;
} CFrame_Header;

typedef struct {
    uint8_t*            Data;
} CFame_Data;

typedef struct {
    CFrame_Header       Header;
    CFame_Data          Data;
} CFrame;

#define CFRAME_WRITE_DIV        1
#define CFRAME_READ_DIV         2
#define CFRAME_DIV              ((CFRAME_READ_DIV) > (CFRAME_WRITE_DIV) ? (CFRAME_READ_DIV) : (CFRAME_WRITE_DIV))
// ---------------------------------------------------------------

uint8_t  cycles;
uint8_t  index;
uint32_t errorCode;

uint32_t Assert_Num(int a, int b, uint16_t line, uint8_t cycles, uint8_t index);
uint32_t Assert_Status(Codec_Status a, Codec_Status b, uint16_t line, uint8_t cycles, uint8_t index);
uint32_t Assert_Bytes(uint8_t* a, uint8_t* b, int len, uint16_t line, uint8_t cycles, uint8_t index);
uint32_t Assert_BasicFrame(BasicFrame* a, BasicFrame* b, uint16_t line, uint8_t cycles, uint8_t index);
uint32_t Assert_CFrame(CFrame* a, CFrame* b, uint16_t line, uint8_t cycles, uint8_t index);
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
uint32_t Test_Frame_DuplicateHeader_Packet(void);
uint32_t Test_Frame_DuplicateHeader_Sync_Packet(void);
uint32_t Test_Async_Noise_Packet(void);
uint32_t Test_Async_Sync_Packet(void);
uint32_t Test_Async_DuplicateHeader_Packet(void);
uint32_t Test_Async_DuplicateHeader_Sync_Packet(void);
uint32_t Test_Async_Part(void);
uint32_t Test_Frame_Size(void);
uint32_t Test_Frame_CFrame(void);
uint32_t Test_Buffer_CFrame(void);

static const TestFn TESTS[] = {
    Test_Frame_BasicFrame,
    Test_Buffer_BasicFrame,
    Test_Frame_Packet,
    Test_Buffer_Packet,
    Test_Frame_Noise_Packet,
    Test_Frame_Sync_Packet,
    Test_Frame_DuplicateHeader_Packet,
    Test_Frame_DuplicateHeader_Sync_Packet,
    Test_Async_Noise_Packet,
    Test_Async_Sync_Packet,
    Test_Async_DuplicateHeader_Packet,
    Test_Async_DuplicateHeader_Sync_Packet,
    Test_Async_Part,
    Test_Frame_Size,
    Test_Frame_CFrame,
    Test_Buffer_CFrame,
};
static const uint32_t TESTS_LEN = sizeof(TESTS) / sizeof(TESTS[0]);

// Parameters for Async
int frameCount = 0;
int errorCount = 0;
uint8_t frameCplt = 0;
uint16_t line;

Codec_Frame* pFrame;
void Codec_onDecodeBasicFrame(Codec* codec, Codec_Frame* frame);
void Codec_onDecodePacket(Codec* codec, Codec_Frame* frame);
void Codec_onDecodeErrorPacket(Codec* codec, Codec_Frame* frame, Codec_LayerImpl* layer, Codec_Error error);
void Codec_onEncodePacket(Codec* codec, Codec_Frame* frame);
void Codec_onEncodeErrorPacket(Codec* codec, Codec_Frame* frame, Codec_LayerImpl* layer, Codec_Error error);

int main()
{
    uint32_t i;
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


    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

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
    #undef testBasicFrame
    #define testBasicFrame(PAT)             PRINTF(#PAT "\n");\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                BasicFrame_init(&frame, PAT, sizeof(PAT));\
                                                status = Codec_encodeBuffer(&codec, &frame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                status = Codec_decodeBuffer(&codec, &tempFrame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                assert(BasicFrame, &tempFrame, &frame);\
                                            }


    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

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


    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

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
    #undef testPacket
    #define testPacket(PAT)                 PRINTF(#PAT "\n");\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                Packet_init(&frame, PAT, sizeof(PAT));\
                                                status = Codec_encodeBuffer(&codec, &frame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                status = Codec_decodeBuffer(&codec, &tempFrame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                assert(Packet, &tempFrame, &frame);\
                                            }

    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

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
    #undef testPacket
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

    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

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

    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

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
uint32_t Test_Frame_DuplicateHeader_Packet(void) {
    #undef addNoise
    #undef testPacket
    #define addNoise(STREAM, N)             for (int noiseIndex = 0; noiseIndex < N; noiseIndex++) { \
                                                OStream_writeUInt16(STREAM, PACKET_FIRST_SIGN); \
                                            }

    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx - Noise: %d\n", N, S);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S * 2));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    status = Codec_decodeFrame(&codec, &tempFrame, &istream);\
                                                    assert(Status, status, Codec_Status_Done);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }

    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[120];
    uint8_t rxBuff[120];
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
uint32_t Test_Frame_DuplicateHeader_Sync_Packet(void) {
    #undef addNoise
    #undef testPacket
    #define addNoise(STREAM, N)             for (int noiseIndex = 0; noiseIndex < N; noiseIndex++) { \
                                                OStream_writeUInt16(STREAM, PACKET_FIRST_SIGN); \
                                            }

    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx - Noise: %d\n", N, S);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S * 2));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    status = Codec_decodeFrame(&codec, &tempFrame, &istream);\
                                                    assert(Status, status, Codec_Status_Done);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }

    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[120];
    uint8_t rxBuff[120];
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
#undef testPacket
#undef addNoise
#define addNoise(STREAM, N)             OStream_writePadding(STREAM, 0xFF, N)
#define testPacket(PAT, N, S)           PRINTF(#PAT " %dx\n", N);\
                                        Codec_beginDecode(&codec, &tempFrame);\
                                        pFrame = &frame;\
                                        frameCount = 0;\
                                        for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                            for (index = 0; index < N; index++) {\
                                                Packet_init(&frame, PAT, sizeof(PAT));\
                                                Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                assert(Num, OStream_pendingBytes(&ostream), (index + 1) * Packet_len(&frame));\
                                            }\
                                            Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                            line = __LINE__;\
                                            frameCount = 0;\
                                            Codec_decode(&codec, &istream);\
                                            assert(Num, frameCount, N);\
                                        }


    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

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
    Codec_onDecode(&codec, Codec_onDecodePacket);
    Codec_setDecodeAll(&codec, 1);

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
#undef testPacket
#define addNoise(STREAM, N)             OStream_writePadding(STREAM, 0xFF, N)
#define testPacket(PAT, N, S)           PRINTF(#PAT " %dx\n", N);\
                                        Codec_beginDecode(&codec, &tempFrame);\
                                        pFrame = &frame;\
                                        for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                            for (index = 0; index < N; index++) {\
                                                Packet_init(&frame, PAT, sizeof(PAT));\
                                                Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                assert(Num, OStream_pendingBytes(&ostream), (index + 1) * Packet_len(&frame));\
                                            }\
                                            Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                            line = __LINE__;\
                                            frameCount = 0;\
                                            Codec_decode(&codec, &istream);\
                                            assert(Num, frameCount, N);\
                                        }


    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

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
    Codec_onDecode(&codec, Codec_onDecodePacket);
    Codec_setDecodeSync(&codec, Packet_sync);
    Codec_setDecodeAll(&codec, 1);
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
uint32_t Test_Async_DuplicateHeader_Packet(void) {
    #undef testPacket
    #undef addNoise

    #define addNoise(STREAM, N)             for (int noiseIndex = 0; noiseIndex < N; noiseIndex++) { \
                                                OStream_writeUInt16(STREAM, PACKET_FIRST_SIGN); \
                                            }

#if !CODEC_DECODE_CONTINUOUS
    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx - Noise: %d\n", N, S);\
                                            Codec_beginDecode(&codec, &tempFrame);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S * 2));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    Codec_decode(&codec, &istream);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }
#else
    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx\n", N);\
                                            Codec_beginDecode(&codec, &tempFrame);\
                                            pFrame = &frame;\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S * 2));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                line = __LINE__;\
                                                frameCount = 0;\
                                                errorCount = 0;\
                                                Codec_decode(&codec, &istream);\
                                                assert(Num, frameCount, N);\
                                                assert(Num, errorCount, N * S * 2);\
                                            }
#endif


    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[120];
    uint8_t rxBuff[120];
    uint8_t tempBuff[30];


    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, Packet_baseLayer());
#if CODEC_DECODE_CONTINUOUS
    Codec_onDecode(&codec, Codec_onDecodePacket);
    Codec_onDecodeError(&codec, Codec_onDecodeErrorPacket);
    Codec_onEncodeError(&codec, Codec_onEncodeErrorPacket);
#endif // CODEC_DECODE_CONTINUOUS
#if STREAM_BYTE_ORDER
    OStream_setByteOrder(&ostream, PACKET_BYTE_ORDER);
    IStream_setByteOrder(&istream, PACKET_BYTE_ORDER);
#endif
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
uint32_t Test_Async_DuplicateHeader_Sync_Packet(void) {
    #undef testPacket
    #undef addNoise

    #define addNoise(STREAM, N)             for (int noiseIndex = 0; noiseIndex < N; noiseIndex++) { \
                                                OStream_writeUInt16(STREAM, PACKET_FIRST_SIGN); \
                                            }

#if !CODEC_DECODE_CONTINUOUS
    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx - Noise: %d\n", N, S);\
                                            Codec_beginDecode(&codec, &tempFrame);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S * 2));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    Codec_decode(&codec, &istream);\
                                                    assert(Packet, &tempFrame, &frame);\
                                                }\
                                            }
#else
    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx\n", N);\
                                            Codec_beginDecode(&codec, &tempFrame);\
                                            pFrame = &frame;\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    Packet_init(&frame, PAT, sizeof(PAT));\
                                                    addNoise(&ostream, S);\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * (Packet_len(&frame) + S * 2));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                line = __LINE__;\
                                                frameCount = 0;\
                                                errorCount = 0;\
                                                Codec_decode(&codec, &istream);\
                                                assert(Num, frameCount, N);\
                                                assert(Num, errorCount, N * S);\
                                            }
#endif


    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[120];
    uint8_t rxBuff[120];
    uint8_t tempBuff[30];


    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, Packet_baseLayer());
#if CODEC_DECODE_CONTINUOUS
    Codec_onDecode(&codec, Codec_onDecodePacket);
    Codec_onDecodeError(&codec, Codec_onDecodeErrorPacket);
    Codec_onEncodeError(&codec, Codec_onEncodeErrorPacket);
#endif // CODEC_DECODE_CONTINUOUS
    Codec_setDecodeSync(&codec, Packet_sync);
#if STREAM_BYTE_ORDER
    OStream_setByteOrder(&ostream, PACKET_BYTE_ORDER);
    IStream_setByteOrder(&istream, PACKET_BYTE_ORDER);
#endif

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
uint32_t Test_Async_Part(void) {
    #undef testPacket
    #define testPacket(PAT, N, S)           PRINTF(#PAT " %dx\n", N);\
                                            Codec_beginDecode(&codec, &tempFrame);\
                                            pFrame = &frame;\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                Packet_init(&frame, PAT, sizeof(PAT));\
                                                frameCplt = 0;\
                                                frameCount = 0;\
                                                Codec_beginEncode(&codec, &frame, Codec_EncodeMode_Normal);\
                                                while (frameCplt == 0) {\
                                                    Codec_encode(&codec, &ostream);\
                                                    Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                    Codec_decode(&codec, &istream);\
                                                }\
                                                line = __LINE__;\
                                                assert(Num, frameCount, 1);\
                                            }

    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    Packet frame;
    Packet tempFrame;

    uint8_t txBuff[10];
    uint8_t rxBuff[10];
    uint8_t tempBuff[30];


    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, Packet_baseLayer());
    Codec_onDecode(&codec, Codec_onDecodePacket);
    Codec_onEncode(&codec, Codec_onEncodePacket);
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

uint32_t Test_Frame_Size(void) {
    #define testBasicFrameSize(LEN)         PRINTF("BasicFrame Data Size: %d\n", LEN);\
                                            basicFrame.Header.PacketSize = LEN;\
                                            assert(Num, Codec_frameSize(&basicCodec, &basicFrame, Codec_Phase_Encode), BasicFrame_len(&basicFrame))

    #define testPacketFrameSize(LEN)        PRINTF("PacketFrame Data Size: %d\n", LEN);\
                                            frame.Len = LEN;\
                                            assert(Num, Codec_frameSize(&packetCodec, &frame, Codec_Phase_Encode), Packet_len(&frame))

    Codec basicCodec;
    Codec packetCodec;
    BasicFrame basicFrame;
    Packet frame;

    Codec_init(&basicCodec, BasicFrame_baseLayer());
    BasicFrame_init(&basicFrame, NULL, 0);
    Codec_init(&packetCodec, Packet_baseLayer());
    Packet_init(&frame, NULL, 0);

    cycles = 0;
    index = 0;

    int LEN = 10;

    testBasicFrameSize(0);
    testBasicFrameSize(4);
    testBasicFrameSize(5);
    testBasicFrameSize(10);
    testBasicFrameSize(12);
    testBasicFrameSize(20);
    testBasicFrameSize(100);

    testPacketFrameSize(0);
    testPacketFrameSize(4);
    testPacketFrameSize(5);
    testPacketFrameSize(10);
    testPacketFrameSize(12);
    testPacketFrameSize(20);
    testPacketFrameSize(100);

    return 0;
}

// ------------------------- Custom Frame --------------------------

#if CODEC_DECODE
static Codec_Error      CFrame_Header_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
static Codec_Error      CFrame_Data_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
static Codec_Error      CFrame_Footer_parse(Codec* codec, Codec_Frame* frame, IStream* stream);
#endif // CODEC_DECODE

#if CODEC_ENCODE
static Codec_Error      CFrame_Header_write(Codec* codec, Codec_Frame* frame, OStream* stream);
static Codec_Error      CFrame_Data_write(Codec* codec, Codec_Frame* frame, OStream* stream);
static Codec_Error      CFrame_Footer_write(Codec* codec, Codec_Frame* frame, OStream* stream);
#endif // CODEC_ENCODE

static Stream_LenType   CFrame_Header_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
static Codec_LayerImpl* CFrame_Header_nextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase);

static Stream_LenType   CFrame_Data_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
static Codec_LayerImpl* CFrame_Data_nextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase);

static Stream_LenType   CFrame_Footer_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase);
static Codec_LayerImpl* CFrame_Footer_nextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase);

static const Codec_LayerImpl CFRAME_HEADER_IMPL = {
#if CODEC_DECODE
    CFrame_Header_parse,
#endif
#if CODEC_ENCODE
    CFrame_Header_write,
#endif
    CFrame_Header_getLen,
    CFrame_Header_nextLayer,
};

static const Codec_LayerImpl CFRAME_DATA_IMPL = {
#if CODEC_DECODE
    CFrame_Data_parse,
#endif
#if CODEC_ENCODE
    CFrame_Data_write,
#endif
    CFrame_Data_getLen,
    CFrame_Data_nextLayer,
};

static const Codec_LayerImpl CFRAME_FOOTER_IMPL = {
#if CODEC_DECODE
    CFrame_Footer_parse,
#endif
#if CODEC_ENCODE
    CFrame_Footer_write,
#endif
    CFrame_Footer_getLen,
    CFrame_Footer_nextLayer,
};

#if CODEC_DECODE
static Codec_Error      CFrame_Header_parse(Codec* codec, Codec_Frame* frame, IStream* stream) {
    ((CFrame*) frame)->Header.PacketSize = IStream_readUInt32(stream);
    return CODEC_OK;
}
static Codec_Error      CFrame_Data_parse(Codec* codec, Codec_Frame* frame, IStream* stream) {
    CFrame* cFrame = (CFrame*) frame;
    IStream_readBytes(stream, cFrame->Data.Data, cFrame->Header.PacketSize / CFRAME_READ_DIV);
    return CODEC_OK;
}
static Codec_Error      CFrame_Footer_parse(Codec* codec, Codec_Frame* frame, IStream* stream) {
    IStream_readUInt32(stream) == 0xABCD1234;
    return CODEC_OK;
}
#endif // CODEC_DECODE

#if CODEC_ENCODE
static Codec_Error      CFrame_Header_write(Codec* codec, Codec_Frame* frame, OStream* stream) {
    OStream_writeUInt32(stream, ((CFrame*) frame)->Header.PacketSize);
    return CODEC_OK;
}
static Codec_Error      CFrame_Data_write(Codec* codec, Codec_Frame* frame, OStream* stream) {
    CFrame* cFrame = (CFrame*) frame;
    OStream_writeBytes(stream, cFrame->Data.Data, cFrame->Header.PacketSize / CFRAME_WRITE_DIV);
    return CODEC_OK;
}
static Codec_Error      CFrame_Footer_write(Codec* codec, Codec_Frame* frame, OStream* stream) {
    OStream_writeUInt32(stream, 0xABCD1234);
}
#endif // CODEC_ENCODE

static Stream_LenType   CFrame_Header_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return sizeof(CFrame_Header);
}
static Codec_LayerImpl* CFrame_Header_nextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return (Codec_LayerImpl*) &CFRAME_DATA_IMPL;
}

static Stream_LenType   CFrame_Data_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return ((CFrame*) frame)->Header.PacketSize;
}
static Codec_LayerImpl* CFrame_Data_nextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return (Codec_LayerImpl*) &CFRAME_FOOTER_IMPL;
}

static Stream_LenType   CFrame_Footer_getLen(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return sizeof(uint32_t);
}
static Codec_LayerImpl* CFrame_Footer_nextLayer(Codec* codec, Codec_Frame* frame, Codec_Phase phase) {
    return CODEC_LAYER_NULL;
}

void CFrame_init(CFrame* frame, uint8_t* data, uint32_t size) {
    frame->Header.PacketSize = size;
    frame->Data.Data = data;
}

uint32_t CFrame_len(CFrame* frame) {
    return frame->Header.PacketSize + sizeof(BasicFrame_Header) + sizeof(uint32_t);
}

Codec_LayerImpl* CFrame_baseLayer(void) {
    return (Codec_LayerImpl*) &CFRAME_HEADER_IMPL;
}

uint32_t Test_Frame_CFrame(void) {
    #define testCFrame(PAT, N)              PRINTF(#PAT " %dx\n", N);\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                for (index = 0; index < N; index++) {\
                                                    CFrame_init(&frame, PAT, sizeof(PAT));\
                                                    Codec_encodeFrame(&codec, &frame, &ostream, Codec_EncodeMode_Normal);\
                                                    assert(Num, OStream_pendingBytes(&ostream), (index + 1) * CFrame_len(&frame));\
                                                }\
                                                Stream_readStream(&ostream.Buffer, &istream.Buffer, OStream_pendingBytes(&ostream));\
                                                for (index = 0; index < N; index++) {\
                                                    status = Codec_decodeFrame(&codec, &tempFrame, &istream);\
                                                    assert(Status, status, Codec_Status_Done);\
                                                    assert(CFrame, &tempFrame, &frame);\
                                                }\
                                            }


    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;
    OStream ostream;
    IStream istream;
    Codec codec;
    CFrame frame;
    CFrame tempFrame;

    uint8_t txBuff[42];
    uint8_t rxBuff[42];
    uint8_t tempBuff[30];

    OStream_init(&ostream, NULL, txBuff, sizeof(txBuff));
    IStream_init(&istream, NULL, rxBuff, sizeof(rxBuff));
    Codec_init(&codec, CFrame_baseLayer());
    CFrame_init(&tempFrame, tempBuff, sizeof(tempBuff));

    testCFrame(PAT1, 1);
    testCFrame(PAT1, 2);
    testCFrame(PAT1, 3);

    testCFrame(PAT2, 1);
    testCFrame(PAT2, 2);
    testCFrame(PAT2, 3);

    testCFrame(PAT3, 1);
    testCFrame(PAT3, 2);
    testCFrame(PAT3, 3);

    testCFrame(PAT4, 1);
    testCFrame(PAT4, 2);
    testCFrame(PAT4, 3);

    testCFrame(PAT5, 1);
    testCFrame(PAT5, 2);
    testCFrame(PAT5, 3);

    return 0;
}
uint32_t Test_Buffer_CFrame(void) {
    #undef testCFrame
    #define testCFrame(PAT)                 PRINTF(#PAT "\n");\
                                            for (cycles = 0; cycles < CYCLES_NUM; cycles++) {\
                                                CFrame_init(&frame, PAT, sizeof(PAT));\
                                                status = Codec_encodeBuffer(&codec, &frame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                status = Codec_decodeBuffer(&codec, &tempFrame, buffer, sizeof(buffer));\
                                                assert(Status, status, Codec_Status_Done);\
                                                assert(CFrame, &tempFrame, &frame);\
                                            }


    static uint8_t PAT1[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    static uint8_t PAT2[3] = {0x1A, 0x1B, 0x1C};
    static uint8_t PAT3[4] = {0x2A, 0x2B, 0x2C, 0x2D};
    static uint8_t PAT4[2] = {0x3A, 0x3B};
    static uint8_t PAT5[6] = {0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};

    Codec_Status status;

    Codec codec;
    CFrame frame;
    CFrame tempFrame;

    uint8_t buffer[37];
    uint8_t tempBuff[30];


    Codec_init(&codec, CFrame_baseLayer());
    CFrame_init(&tempFrame, tempBuff, sizeof(tempBuff));

    index = 0;

    testCFrame(PAT1);
    testCFrame(PAT2);
    testCFrame(PAT3);
    testCFrame(PAT4);
    testCFrame(PAT5);

    return 0;
}

// -------------------------- Assert Functions ------------------------
void Codec_onDecodeBasicFrame(Codec* codec, Codec_Frame* frame) {
    if ((errorCode = Assert_BasicFrame((BasicFrame*) pFrame, (BasicFrame*) frame, line, cycles, index)) == 0) {
        frameCount++;
    }
}
void Codec_onDecodePacket(Codec* codec, Codec_Frame* frame) {
    if ((errorCode = Assert_Packet((Packet*) pFrame, (Packet*) frame, line, cycles, index)) == 0) {
        frameCount++;
    }
}
void Codec_onDecodeErrorPacket(Codec* codec, Codec_Frame* frame, Codec_LayerImpl* layer, Codec_Error error) {
    errorCount++;
}
void Codec_onEncodePacket(Codec* codec, Codec_Frame* frame) {
    frameCplt = 1;
}
void Codec_onEncodeErrorPacket(Codec* codec, Codec_Frame* frame, Codec_LayerImpl* layer, Codec_Error error) {
    PRINTF("[Encode Error] %u\n", error);
}
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
uint32_t Assert_CFrame(CFrame* a, CFrame* b, uint16_t line, uint8_t cycles, uint8_t index) {
    if (!(a->Header.PacketSize == b->Header.PacketSize &&
        memcmp(a->Data.Data, b->Data.Data, a->Header.PacketSize / CFRAME_DIV) == 0)) {
        PRINTF("[CFrame] Expected Len: %d, Found Len: %d\n",
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
    if (!(a->Len == b->Len &&
        memcmp(a->Data, b->Data, a->Len) == 0)) {
        PRINTF("[Packet] Expected Len: %d, Found Len: %d\n",
               b->Len, a->Len);
        PRINTF("    Expected: ");
        printArray(b->Data, b->Len);
        PRINTF("\n    Found: ");
        printArray(a->Data, a->Len);
        PUTS("");

        return compress(line, cycles, index);
    }
    return 0;
}
