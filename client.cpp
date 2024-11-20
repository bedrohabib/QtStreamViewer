#include "Decoder.h"
#include "Encoder.h"
#include <iostream>
#include <unistd.h> // For close()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <server_ip> <server_port>" << std::endl;
        return 1;
    }

    const char* server_ip = argv[2];     // Server IP address
    int server_port = std::stoi(argv[3]); // Server port number

    // Create a TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Setup server address structure
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid server IP address" << std::endl;
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection to server failed" << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "Connected to server at " << server_ip << ":" << server_port << std::endl;

    // Create the Decoder, Encoder, and Processor objects
    Decoder decoder;
    Encoder encoder;

    if (!decoder.open(argv[1])) {
        close(sockfd);
        return 1;
    }

    // Initialize encoder with socket_fd for video transmission
    if (!encoder.initialize("dump.mp4",decoder.getWidth(), decoder.getHeight(), sockfd)) {
        close(sockfd);
        return 1;
    }

  

    AVFrame* input_frame = av_frame_alloc();
    AVFrame* output_frame = av_frame_alloc();

    output_frame->format = AV_PIX_FMT_YUV420P;
    output_frame->width = decoder.getWidth();
    output_frame->height = decoder.getHeight();
    av_frame_get_buffer(output_frame, 0);

    while (decoder.readFrame(input_frame)) {
            encoder.writeFrame(input_frame);  
    
    
    std::cout<<"everyframe is written"<<std::endl;
    }
    av_frame_free(&input_frame);

    // Close the socket after processing is done
    close(sockfd);

    return 0;
}