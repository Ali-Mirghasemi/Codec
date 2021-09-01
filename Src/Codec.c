#include "Codec.h"

typedef Codec_Result (*Codec_EncodeFiledFn)(Codec* codec, OStream* stream, Codec_Frame* frame);
typedef Codec_Result (*Codec_DecodeFiledFn)(Codec* codec, OStream* stream);

static Codec_Result Codec_encodeHeader(Codec* codec, OStream* stream, Codec_Frame* frame);
static Codec_Result Codec_encodeData(Codec* codec, OStream* stream, Codec_Frame* frame);
static Codec_Result Codec_encodeFooter(Codec* codec, OStream* stream, Codec_Frame* frame);

static Codec_Result Codec_decodeHeader(Codec* codec, IStream* stream);
static Codec_Result Codec_decodeData(Codec* codec, IStream* stream);
static Codec_Result Codec_decodeFooter(Codec* codec, IStream* stream);

static const Codec_EncodeFiledFn CODEC_ENCODE_FIELDS[] = {
    Codec_encodeHeader,
    Codec_encodeData,
    Codec_encodeFooter,
};

static const Codec_DecodeFiledFn CODEC_DECODE_FIELDS[] = {
    Codec_decodeHeader,
    Codec_decodeData,
    Codec_decodeFooter,
};

void Codec_init(Codec* codec, Codec_HeaderImpl* header, Codec_DataImpl* data, Codec_FooterImpl* footer, Codec_Frame* frame) {
    codec->HeaderImpl = header;
    codec->DataImpl = data;
    codec->FooterImpl = footer;
    codec->Frame = frame;
    codec->State = Codec_State_Header;
}

void Codec_decode(Codec* codec, IStream* stream) {
    Codec_Result result;
    IStream_setLimit(stream, STREAM_NO_LIMIT);
    while ((result = CODEC_DECODE_FIELDS[(uint8_t) codec->State](codec, stream)) == Codec_InProcess) {
        IStream_setLimit(stream, STREAM_NO_LIMIT);
    }
}

Codec_Result Codec_encode(Codec* codec, OStream* stream, Codec_Frame* frame) {
    Codec_Result result;
    Codec_State state;
    for (state = Codec_State_Header; state <= Codec_State_Footer; state++) {
        if ((result = CODEC_ENCODE_FIELDS[(uint8_t) state](codec, stream, frame)) != Codec_Ok) {
            return result;
        }
    }
    // done
    return Codec_Ok;
}

Codec_Result Codec_encodeFlush(Codec* codec, OStream* stream, Codec_Frame* frame, uint8_t flushPart) {
    Codec_Result result;
    Codec_State state;
    for (state = Codec_State_Header; state <= Codec_State_Footer; state++) {
        if ((result = CODEC_ENCODE_FIELDS[(uint8_t) state](codec, stream, frame)) != Codec_Ok) {
            return result;
        }
        if (flushPart) {
            OStream_flush(stream);
        }
    }
    // done
    return Codec_Ok;
}

static Codec_Result Codec_encodeHeader(Codec* codec, OStream* stream, Codec_Frame* frame) {
    if (codec->HeaderImpl) {
        // set limit for header
        OStream_setLimit(stream, codec->HeaderImpl->getHeaderLen(codec, frame));
        return codec->HeaderImpl->write(codec, frame, stream);
    }
    else {
        return Codec_NoHeaderImpl;
    }
}
static Codec_Result Codec_encodeData(Codec* codec, OStream* stream, Codec_Frame* frame) {
    if (codec->DataImpl) {
        // set limit for data
        OStream_setLimit(stream, codec->HeaderImpl->getPacketSize(codec, frame));
        return codec->DataImpl->write(codec, frame, stream);
    }
    else {
        return Codec_NoDataImpl;
    }
}
static Codec_Result Codec_encodeFooter(Codec* codec, OStream* stream, Codec_Frame* frame) {
    if (codec->FooterImpl) {
        // set limit for footer
        OStream_setLimit(stream, codec->FooterImpl->getFooterLen(codec, frame));
        return codec->FooterImpl->write(codec, frame, stream);
    }
    else {
        return Codec_NoFooterImpl;
    }
}

static Codec_Result Codec_decodeHeader(Codec* codec, IStream* stream) {
    if (codec->HeaderImpl) {
        Stream_LenType len = IStream_available(stream);
        Stream_LenType headerLen = codec->HeaderImpl->getHeaderLen(codec, codec->Frame);
        if (len >= headerLen) {
            Stream_LenType index;
            // find
            if (codec->HeaderImpl->find) {
                IStream_setLimit(stream, STREAM_NO_LIMIT);
                if ((index = codec->HeaderImpl->find(codec, stream)) < 0) {
                    IStream_ignore(stream, len);
                    return Codec_ProcessDone;
                }
                else {
                    IStream_ignore(stream, index);
                }
            }
            // check stream
            do {
                IStream_setLimit(stream, headerLen);
                if (codec->HeaderImpl->parse(codec, codec->Frame, stream) == Codec_Ok) {
                    codec->State = Codec_State_Data;
                    return Codec_InProcess;
                }
                IStream_ignore(stream, 1);
            } while (IStream_available(stream) > headerLen);
        }
        return Codec_ProcessDone;
    }
    else {
        return Codec_NoFooterImpl;
    }
}
static Codec_Result Codec_decodeData(Codec* codec, IStream* stream) {
    if (codec->DataImpl) {
        Stream_LenType packetSize = codec->HeaderImpl->getPacketSize(codec, codec->Frame);
        if (IStream_available(stream) >= packetSize) {
            IStream_setLimit(stream, packetSize);
            if (codec->DataImpl->parse(codec, codec->Frame, stream) == Codec_Ok) {
                codec->State = Codec_State_Footer;
                return Codec_InProcess;
            }
            else {
                codec->State = Codec_State_Header;
            }
        }
        return Codec_ProcessDone;
    }
    else {
        return Codec_NoDataImpl;
    }
}
static Codec_Result Codec_decodeFooter(Codec* codec, IStream* stream) {
    if (codec->FooterImpl) {
        Stream_LenType footerLen = codec->FooterImpl->getFooterLen(codec, codec->Frame);
        if (IStream_available(stream) >= footerLen) {
            IStream_setLimit(stream, footerLen);
            if (codec->FooterImpl->parse(codec, codec->Frame, stream) == Codec_Ok) {
                if (codec->onFrame) {
                    codec->onFrame(codec, codec->Frame);
                }
                codec->State = Codec_State_Header;
                return Codec_InProcess;
            }
            else {
                codec->State = Codec_State_Header;
            }
        }
        return Codec_ProcessDone;
    }
    else {
        return Codec_NoFooterImpl;
    }
}
