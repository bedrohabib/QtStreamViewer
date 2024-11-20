#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QSocketNotifier>
#include <QDebug>

class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(QObject* parent = nullptr) : QObject(parent), server(new QTcpServer(this)) {}

    ~Worker() {
        server->close();
        delete server;
    }

public slots:
    void startServer() {
        const int PORT = 12345;  // Replace with your desired port
        if (!server->listen(QHostAddress::Any, PORT)) {
            qDebug() << "Server failed to start:" << server->errorString();
            emit errorOccurred(server->errorString());
            return;
        }
        connect(server, &QTcpServer::newConnection, this, &Worker::handleConnection);
        qDebug() << "Server started on port" << PORT;
    }

    void handleConnection() {
        QTcpSocket* clientSocket = server->nextPendingConnection();
        if (clientSocket) {
            qDebug() << "Client connected!";
            connect(clientSocket, &QTcpSocket::readyRead, [clientSocket]() {
                QByteArray data = clientSocket->readAll();

                clientSocket->write("Echo: " + data);  // Echo the received data
            });
            connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
        }
    }

signals:
    void errorOccurred(const QString& error);

private:
    QTcpServer* server;
};
