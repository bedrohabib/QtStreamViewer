#include "Encoder.h"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

Encoder::Encoder()
    : format_context(nullptr), codec_context(nullptr), stream(nullptr), codec(nullptr),socket_fd(-1){
}

Encoder::~Encoder() {
    close();
}

bool Encoder::initialize(const std::string& output_file, int width, int height, int socket_fd,
                            AVRational time_base, AVRational framerate) {
    this->socket_fd = socket_fd;
    if (!setupCodec(width, height, time_base, framerate)) {
        return false;
    }

    if (!openOutputFile(output_file)) {
        return false;
    }

    return true;
}

bool Encoder::setupCodec(int width, int height, AVRational time_base, AVRational framerate) {
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "Codec not found" << std::endl;
        return false;
    }

    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        std::cerr << "Could not allocate codec context" << std::endl;
        return false;
    }

    codec_context->width = width;
    codec_context->height = height;
    codec_context->time_base = time_base;
    codec_context->framerate = framerate;
    codec_context->gop_size = 10;
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    if (avcodec_open2(codec_context, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return false;
    }

    return true;
}

bool Encoder::openOutputFile(const std::string& output_file) {
    if (avformat_alloc_output_context2(&format_context, nullptr, nullptr, output_file.c_str()) < 0) {
        std::cerr << "Could not create output context" << std::endl;
        return false;
    }

    stream = avformat_new_stream(format_context, nullptr);
    if (!stream) {
        std::cerr << "Could not allocate output stream" << std::endl;
        return false;
    }

    if (avcodec_parameters_from_context(stream->codecpar, codec_context) < 0) {
        std::cerr << "Could not copy codec parameters to output stream" << std::endl;
        return false;
    }

    stream->time_base = codec_context->time_base;

    if (!(format_context->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&format_context->pb, output_file.c_str(), AVIO_FLAG_WRITE) < 0) {
            std::cerr << "Could not open output file" << std::endl;
            return false;
        }
    }

    if (avformat_write_header(format_context, nullptr) < 0) {
        std::cerr << "Error occurred when writing header" << std::endl;
        return false;
    }

    return true;
}

bool Encoder::writeFrame(AVFrame* frame) {
    if (avcodec_send_frame(codec_context, frame) < 0) {
        return false;
    }

    while (true) {
        AVPacket packet = {};
        if (avcodec_receive_packet(codec_context, &packet) < 0) {
            break;
        }
        int64_t pts_net = htonll(packet.pts);
        int64_t dts_net = htonll(packet.dts);
        int32_t stream_index_net = htonl(packet.stream_index);
        int32_t size_net = htonl(packet.size);

        // Send metadata (pts, dts, stream_index, size)
        if (send(socket_fd, &pts_net, sizeof(pts_net), 0) < 0 ||
            send(socket_fd, &dts_net, sizeof(dts_net), 0) < 0 ||
            send(socket_fd, &stream_index_net, sizeof(stream_index_net), 0) < 0 ||
            send(socket_fd, &size_net, sizeof(size_net), 0) < 0) {
            std::cerr << "Failed to send packet metadata" << std::endl;
            return false;
        }

        // Send packet data
        if (send(socket_fd, packet.data, packet.size, 0) < 0) {
            std::cerr << "Failed to send packet data" << std::endl;
            return false;
        }


        av_packet_unref(&packet);
    }

    return true;
}

// Helper function to convert int64_t to network byte order (big-endian)

void Encoder::close() {
    if (format_context && format_context->pb) {
        av_write_trailer(format_context);
        if (!(format_context->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&format_context->pb);
        }
    }
    if (codec_context) {
        avcodec_free_context(&codec_context);
    }
    if (format_context) {
        avformat_free_context(format_context);
    }
}


// Helper function to convert int64_t to network byte order (big-endian)
int64_t htonll(int64_t value) {
    return (((int64_t)htonl(value & 0xFFFFFFFF)) << 32) | htonl(value >> 32);
}
