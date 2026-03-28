#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>

void hexDump(const char* label, const uint8_t* data, size_t len) {
    std::cout << label << " (" << len << " bytes): ";
    for (size_t i = 0; i < len; ++i)
        std::cout << std::hex << std::setfill('0') << std::setw(2)
                  << static_cast<int>(data[i]) << " ";
    std::cout << std::dec << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <IP> <PORT>" << std::endl;
        return 1;
    }

    const char* ip = argv[1];
    int port = std::atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(sock);
        return 1;
    }

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    std::cout << "Connected to " << ip << ":" << port << std::endl;

    // Step 1: Start IR Learn
    uint8_t start_cmd[] = { 0x20, 0xA1, 0x80, 0x01, 0x01, 0x00 };
    if (send(sock, start_cmd, sizeof(start_cmd), 0) != sizeof(start_cmd)) {
        perror("Failed to send start command");
        close(sock);
        return 1;
    }
    hexDump("SENT", start_cmd, sizeof(start_cmd));

    uint8_t ack[4];
    ssize_t received = recv(sock, ack, sizeof(ack), 0);
    if (received != 4) {
        perror("Failed to receive 4-byte ACK");
        close(sock);
        return 1;
    }
    hexDump("ACK", ack, 4);

    // Step 2: Wait 5 seconds
    std::cout << "Fire remote now..." << std::endl;
    sleep(5);

    // Step 3: Send Learn IR command
    uint8_t learn_cmd[] = { 0x20, 0xA2, 0x80, 0x00 };
    if (send(sock, learn_cmd, sizeof(learn_cmd), 0) != sizeof(learn_cmd)) {
        perror("Failed to send learn command");
        close(sock);
        return 1;
    }
    hexDump("SENT", learn_cmd, sizeof(learn_cmd));

    //todo use timeout, not bytes
    uint8_t result[18];
    received = recv(sock, result, sizeof(result), 0);
    if (received != 18) {
        //todo wait for timeout in server, flush rx/tx
        perror("Failed to receive 18-byte IR data");
        close(sock);
        return 1;
    }
    hexDump("IR DATA", result, 18);

    // Save binary output
    std::ofstream fout("ir_capture.bin", std::ios::binary);
    if (fout.is_open()) {
        fout.write(reinterpret_cast<char*>(result), 18);
        fout.close();
        std::cout << "Saved IR data to ir_capture.bin" << std::endl;
    } else {
        std::cerr << "Failed to write to ir_capture.bin" << std::endl;
    }

    // Step 4: Send Stop command
    uint8_t stop_cmd[] = { 0x20, 0xA4, 0x80, 0x00 };
    send(sock, stop_cmd, sizeof(stop_cmd), 0);  // even if it fails, continue
    hexDump("SENT", stop_cmd, sizeof(stop_cmd));

    close(sock);
    std::cout << "IR learning session completed." << std::endl;
    return 0;
}
