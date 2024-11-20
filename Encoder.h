#include <string>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}
class Encoder {
public:
    Encoder();
    ~Encoder();
    bool initialize(const std::string& output_file, int width, int height, int socket_fd,
                   AVRational time_base = {1, 15}, AVRational framerate = {15, 1});
    bool writeFrame(AVFrame* frame);
    void close();
    

private:
    AVFormatContext* format_context;
    AVCodecContext* codec_context;
    AVStream* stream;
    const AVCodec* codec;
    int socket_fd; 
    bool openOutputFile(const std::string& output_file);
    bool setupCodec(int width, int height, AVRational time_base, AVRational framerate);
};
int64_t htonll(int64_t value);