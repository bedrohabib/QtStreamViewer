#include <iostream>
#include <cstring>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <QImage>
#include <QLabel>
#include <QApplication>
#include <QTimer>
#include <QTime>
#include "ReceptionWorker.h"
#include "DecodingWorker.h"
#include "RenderingWorker.h"
#include "ThreadSafeQueue.h"


#define PORT 8080  

AVFormatContext* format_context = nullptr;
AVCodecContext* codec_context = nullptr;
AVStream* stream = nullptr;
const AVCodec* codec;

void cleanup() {
    if (codec_context) {
        avcodec_free_context(&codec_context);
    }
}


bool setupdec() {
    codec = avcodec_find_decoder(AV_CODEC_ID_H264); 
    if (!codec) {
        std::cerr << "Codec not found" << std::endl;
        return false;
    }

    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        std::cerr << "Could not allocate codec context" << std::endl;
        return false;
    }
    if (avcodec_open2(codec_context, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return false;
    }

    return true;
}

class FPSDisplay {
public:
    FPSDisplay() : frameCount(0), lastTime(QTime::currentTime()) {}

    void update() {
        frameCount++;
        if (lastTime.msecsTo(QTime::currentTime()) >= 1000) {
            fps = frameCount;
            frameCount = 0;
            lastTime = QTime::currentTime();
        }
    }

    int getFPS() const {
        std::cout<<"the fps is: "<<fps<<std::endl;
        return fps;
    }

private:
    int frameCount;
    int fps = 0;
    QTime lastTime;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    qRegisterMetaType<AVPacket>("AVPacket");

    int server_fd, new_socket;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    if (!setupdec()) {
        return 1;
    }

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    int addrlen = sizeof(address);
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        std::cerr << "Accept failed" << std::endl;
        return 1;
    }

    std::cout << "Connection established with client!" << std::endl;

    ThreadSafeQueue<QImage> frameQueue;

    // QLabel for rendering
    QLabel label;
    label.setFixedSize(1920, 1080);
    label.show();

    int targetFPS = 100;

    QThread processingThread;

    ReceptionWorker* receptionWorker = new ReceptionWorker(new_socket);
    DecodingWorker* decodingWorker = new DecodingWorker(codec_context, frameQueue);
    RenderingWorker* renderingWorker = new RenderingWorker(&label, frameQueue, targetFPS);

    receptionWorker->moveToThread(&processingThread);
    decodingWorker->moveToThread(&processingThread);

    QObject::connect(&processingThread, &QThread::started, receptionWorker, &ReceptionWorker::getPacket);
    QObject::connect(receptionWorker, &ReceptionWorker::packetReady, decodingWorker, &DecodingWorker::getFrame);

    processingThread.start();
    renderingWorker->start();

    int result = app.exec();

    processingThread.quit();
    renderingWorker->quit();
    processingThread.wait();
    renderingWorker->wait();

    delete receptionWorker;
    delete renderingWorker;
    delete decodingWorker;

    return result;
}
