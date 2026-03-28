#include <vector>
#include <cstdint>
#include <fstream>
#include <string>

namespace parser
{

struct block
{
    uint16_t mark_us;
    uint16_t segment_us;

    uint16_t off_us() const
    {
      return segment_us - mark_us;
    }
};

struct first_block
{
    uint16_t unknown1;
    uint16_t mark_us;
    uint16_t unknown2;
    uint16_t segment_us;

    uint16_t off_us() const
    {
      return segment_us - mark_us;
    }
};

struct frame
{
    first_block first;
    std::vector<block> remaining;
};

// Parse streaming mode: sequence of mark + segment pairs
std::vector<block> parse_streaming_mode(const std::vector<uint16_t> &raw_data)
{
  std::vector<block> stream;

  for (size_t i = 0; i < raw_data.size(); i += 2) {
    if (i + 1 < raw_data.size()) {
      stream.push_back(
          { .mark_us = raw_data[i], .segment_us = raw_data[i + 1] });
    }
  }

  return stream;
}

// Parse single frame mode: first_block (4 values) + remaining blocks (2 values each)
frame parse_single_frame_mode(const std::vector<uint16_t> &raw_data)
{
  frame f;

  // First block has 4 values (2 unknowns, mark, segment)
  if (raw_data.size() >= 4) {
    f.first = {
      .unknown1 = raw_data[0],
      .mark_us = raw_data[1],
      .unknown2 = raw_data[2],
      .segment_us = raw_data[3]
    };

    // Remaining blocks: pairs of mark + segment
    for (size_t i = 4; i + 1 < raw_data.size(); i += 2) {
      f.remaining.push_back(
          { .mark_us = raw_data[i], .segment_us = raw_data[i + 1] });
    }
  }

  return f;
}

// Convert streaming mode to gnuplot format (time, amplitude pairs)
// amplitude: 1 = mark (ON), 0 = segment (OFF/pause)
std::vector<std::pair<uint32_t, int>> to_gnuplot_streaming(
    const std::vector<block> &stream)
{
  std::vector<std::pair<uint32_t, int>> plot_data;
  uint32_t time_us = 0;

  for (const auto &block : stream) {
    // Start of mark (ON state)
    plot_data.push_back( { time_us, 1 });

    // End of mark, start of segment (OFF state)
    time_us += block.mark_us;
    plot_data.push_back( { time_us, 0 });

    // End of segment (prepare for next mark)
    time_us += block.off_us();
  }

  return plot_data;
}

// Convert single frame mode to gnuplot format
std::vector<std::pair<uint32_t, int>> to_gnuplot_frame(const frame &f)
{
  std::vector<std::pair<uint32_t, int>> plot_data;
  uint32_t time_us = 0;

  // First block
  plot_data.push_back( { time_us, 1 });
  time_us += f.first.mark_us;
  plot_data.push_back( { time_us, 0 });
  time_us += f.first.off_us();

  // Remaining blocks
  for (const auto &block : f.remaining) {
    plot_data.push_back( { time_us, 1 });
    time_us += block.mark_us;
    plot_data.push_back( { time_us, 0 });
    time_us += block.off_us();
  }

  return plot_data;
}

// Write gnuplot data file
void write_gnuplot_data(const std::string &filename,
    const std::vector<std::pair<uint32_t, int>> &plot_data)
{
  std::ofstream file(filename);

  if (!file.is_open()) {
    return;
  }

  file << "time_us amplitude\n";
  for (const auto& [time, amp] : plot_data) {
    file << time << " " << amp << "\n";
  }

  file.close();
}

// Example usage:
/*
 int main() {
 // Streaming mode example
 std::vector<uint16_t> streaming_raw = {
 882, 1779, 1769, 2666, 882, 2667, 882, 1779, 881, 1778, 1770, 2667
 // ... more pairs
 };

 auto stream = parse_streaming_mode(streaming_raw);
 auto plot_data = to_gnuplot_streaming(stream);
 write_gnuplot_data("streaming_ir.dat", plot_data);

 // Single frame mode example
 std::vector<uint16_t> frame_raw = {
 6, 861, 31, 1779, 1769, 2659, 889, 2674, 882, 1779
 // ... more pairs
 };

 auto f = parse_single_frame_mode(frame_raw);
 auto plot_data_frame = to_gnuplot_frame(f);
 write_gnuplot_data("frame_ir.dat", plot_data_frame);

 return 0;
 }
 */
}
