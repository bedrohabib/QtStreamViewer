#include <QThread>
#include <QLabel>
#include <QPixmap>
#include <QCoreApplication>
#include <iostream>
#include <chrono>
#include <thread>
#include "ThreadSafeQueue.h"

using namespace std::chrono;
class RenderingWorker : public QThread {
    Q_OBJECT
public:
    RenderingWorker(QLabel* label, ThreadSafeQueue<QImage>& queue, int targetFPS)
        : label(label), frameQueue(queue), targetFPS(targetFPS) {}

    void run() override {
        std::cout << "Rendering started." << std::endl;
        int frameCount = 0;
        auto sleepDuration = milliseconds(1000 / targetFPS); 
        while (true) {
            QImage frame = frameQueue.pop();
            label->setPixmap(QPixmap::fromImage(frame));
            QCoreApplication::processEvents();
            if (sleepDuration.count() > 0) {
                std::this_thread::sleep_for(sleepDuration);
            }
        }
    }

private:
    QLabel* label;
    ThreadSafeQueue<QImage>& frameQueue;
    int targetFPS;  
};
