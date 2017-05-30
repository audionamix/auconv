// Copyright (C) 2017 Audionamix
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
// USA.
#ifndef AUCONV_SRC_AUCONV_FILE_H_
#define AUCONV_SRC_AUCONV_FILE_H_

#include <cstdint>

#include <string>
#include <system_error>
#include <vector>
#include <memory>

// TODO: impl class
struct AVFormatContext;
struct AVCodecContext;

namespace auconv {

enum class Format : uint8_t { kWav = 0, kMp3 };

class File {
 public:
  File();

  void Open(const std::string& path, std::error_code& err);

  uint32_t sample_rate() const;
  void set_sample_rate(uint32_t value);

  uint8_t channel_count() const;
  void set_mono(bool value);

  void Export(const std::string& path, Format format, std::error_code& err);

 private:
  uint32_t sample_rate_;
  uint8_t channel_count_;
  bool mono_;

  class Impl;
  std::shared_ptr<Impl> impl_;
};

}  // namespace auconv

#endif  // AUCONV_SRC_AUCONV_FILE_H_
