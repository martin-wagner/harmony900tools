#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <csignal>
#include <atomic>

#include "lib.h"
#include "start.h"
#include "single.h"
#include "stream.h"
#include "stop.h"
#include "data.h"


using namespace std;
using namespace literals;
// enables literal suffixes, e.g. 24h, 1ms, 1s.

const int HEADER_SIZE = 4;

atomic<int> sock = -1;

bool writeFile(const string &filename, const string &data)
{
  ofstream file(filename);

  if (!file.is_open()) {
    return false;
  }

  file << data;

  file.close();

  return true;
}

void handleSigint(int)
{
  int s = sock.exchange(-1);
  if (s < 0) {
    return;
  }

  frame::Stop stop;
  auto tx = stop.get();

  send(s, tx.data(), tx.size(), 0);

  cout << "ctrl+c detected, close connection" << endl;

  close(s);
}

bool open(const string &host, uint16_t port)
{
  //handle ctrl+c
  signal(SIGINT, handleSigint);

  sock = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in a { };
  a.sin_family = AF_INET;
  a.sin_port = port;
  if (inet_pton(AF_INET, host.c_str(), &a.sin_addr) <= 0) {
    cerr << "Invalid IP/host address: " << host << endl;
    close(sock);
    return false;
  }

  if (connect(sock, (sockaddr*) &a, sizeof(a)) < 0) {
    cerr << "Connection failed" << endl;
    close(sock);
    return false;
  }
  return true;
}

bool readFrame(vector<uint8_t> &frame)
{
  uint8_t hdr[HEADER_SIZE];
  uint8_t buf[4096];

  frame.clear();

  auto bytes = recv(sock, hdr, HEADER_SIZE, MSG_WAITALL);
  if (bytes != HEADER_SIZE) {
    return false;
  }
  frame.assign(hdr, hdr + HEADER_SIZE);

  bytes = recv(sock, buf, sizeof(buf), MSG_DONTWAIT);
  if (bytes > 0) {
    frame.insert(frame.end(), buf, buf + bytes);
  }
  return true;
}

bool sendFrame(const vector<uint8_t> &frame)
{
  auto bytes = send(sock, frame.data(), frame.size(), 0);
  if (bytes == frame.size()) {
    return true;
  }
  return false;
}

bool pollSingleFrame(const string &file)
{
  frame::Single single;
  vector<uint8_t> rx;

  cout << "poll single frame" << endl << "-------------" << endl;

  while (true) {
    auto tx = single.get();
    cout << lib::writeHex("-> poll chunk: ", tx);
    sendFrame(tx);

    if (!readFrame(rx)) {
      cout << "<- no/error data rx, abort" << endl;
      break;
    }

    cout << lib::writeHex("<- rx chunk: ", rx);

    auto ret = single.addChunk(rx);
    if (ret == frame::Single::Status::DONE) {
      break;
    }
    switch (ret) {
      case frame::Single::Status::OK:
        break;
      case frame::Single::Status::ERR_TIMEOUT:
        cout << lib::writeTime() << "<- server returned timeout" << endl;
        return false;
      case frame::Single::Status::ERR_RETURN:
        cout << "<- server returned error: " << to_string(single.getError(rx))
            << endl;
        return false;
      case frame::Single::Status::ERR_SIZE:
        cout << "<- invalid size" << endl;
        return false;
      case frame::Single::Status::ERR_RESPONSE_FORMAT:
        cout << "<- invalid msg format" << endl;
        return false;
      case frame::Single::Status::ERR_PAYLOAD_FORMAT:
        cout << "<- invalid payload format" << endl;
        return false;
      default:
        cout << "<- unexpected: " << to_string((int) ret) << endl;
        return false;
    }
  }

  auto payload = single.getPayload();
  if (payload.empty()) {
    cout << lib::writeTime() << "<- payload empty" << endl;
    return false;
  }
  frame::TimingStream timingStream(payload);

  cout << "single frame (hex): " << timingStream.convertHexString() << endl;
  cout << "single frame (tµs): " << timingStream.convertIntString() << endl;
  cout << timingStream.convertAsciiPlot(lib::getTerminalWidth()) << endl;

  //todo find what those two words do. might have something to do with the corresponding signal (??)
  auto leftover = single.getQ();
  cout << lib::writeHex("<- leftover words (hex): ", leftover);
  cout << lib::writeData("<- leftover words (dec): ", leftover);

  if (!file.empty()) {
    //gnuplot
    auto str = timingStream.convertGnuplot();
    writeFile(file, str);
  }

  return true;
}

bool pollStream(const string &file, chrono::milliseconds timeout)
{
  frame::Stream stream;
  vector<uint8_t> rx;

  cout << "poll stream" << endl << "-------------" << endl;

  auto t_start = chrono::steady_clock::now();

  while (true) {
    auto tx = stream.get();
    cout << lib::writeHex("-> poll chunk: ", tx);
    sendFrame(tx);

    if (!readFrame(rx)) {
      cout << "<- no/error data rx, abort" << endl;
      break;
    }

    cout << lib::writeHex("<- rx chunk: ", rx);

    auto ret = stream.addChunk(rx);
    switch (ret) {
      case frame::Stream::Status::OK:
        break;
      case frame::Stream::Status::ERR_SIZE:
        cout << "<- invalid size" << endl;
        return false;
      case frame::Stream::Status::ERR_FORMAT:
        cout << "<- invalid msg format" << endl;
        return false;
      case frame::Stream::Status::ERR_TERM:
        cout << "<- terminator missing" << endl;
        return false;
      default:
        cout << "<- unexpected: " << to_string((int) ret) << endl;
        return false;
    }

    auto t_now = chrono::steady_clock::now();
    if ((t_now - t_start) > timeout) {
      break;
    }
  }

  auto payload = stream.getPayload();
  if (payload.empty()) {
    cout << lib::writeTime() << "<- payload empty" << endl;
    return false;
  }
  frame::TimingStream timingStream(payload);

  cout << "single frame (hex): " << timingStream.convertHexString() << endl;
  cout << "single frame (tµs): " << timingStream.convertIntString() << endl;
  cout << timingStream.convertAsciiPlot(lib::getTerminalWidth()) << endl;

  if (!file.empty()) {
    //gnuplot
    auto str = timingStream.convertGnuplot();
    writeFile(file, str);
  }

  return true;
}

int main(int argc, char **argv)
{
  vector<uint8_t> frame;
  vector<uint16_t> data;
  bool received_command = false;

  if (argc != 3) {
    cout << "usage: ircap ip port" << endl;
    return EXIT_SUCCESS;
  }

  auto host = string(argv[1]);
  auto port = htons(atoi(argv[2]));
  auto res = open(host, port);
  if (!res) {
    return EXIT_FAILURE;
  }

  cout << "connected" << endl;

  cout << endl << "opening connection" << endl << "-------------" << endl;

  frame::Start start;
  auto txStart = start.get();

  cout << lib::writeHex("-> capture started: ", txStart);

  sendFrame(txStart);
  if (!readFrame(frame)) {
    cout << "<- no confirmation, abort" << endl;
    return EXIT_FAILURE;
  }
  cout << lib::writeHex("<- confirmation frame: ", frame);

  res = start.check(frame);
  if (!res) {
    cout << "<- invalid confirmation, abort" << endl;
    return EXIT_FAILURE;
  }

  cout << "press remote" << endl;

  received_command = pollSingleFrame("frame_ir.dat");

  pollStream("streaming_ir.dat", 5s);

  cout << "closing connection" << endl << "-------------" << endl;

  frame::Stop stop;
  auto txStop = stop.get();

  cout << lib::writeHex("-> close connection: ", txStop);
  sendFrame(txStop);
  frame.clear();
  if (!readFrame(frame)) {
    cout << "<- no confirmation, abort" << endl;
    return 1;
  }
  cout << lib::writeHex("<- confirmation frame: ", frame);
  //ignore data

  close(sock);

  if (received_command) {
    cout << "command received!" << endl;
    return 0;
  }
  cout << "silence..." << endl;
  return 0;
}
