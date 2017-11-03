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
#include "auconv/file.h"

#include <iostream>
#include <vector>

// ffmpeg
#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif
#include "wave/file.h"

namespace auconv {

using AVFormatContextPtr = std::shared_ptr<AVFormatContext>;
using AVCodecContextPtr = std::shared_ptr<AVCodecContext>;
using SwrContextPtr = std::shared_ptr<SwrContext>;
using AVFramePtr = std::shared_ptr<AVFrame>;
using FrameHandler = std::function<void(AVFramePtr)>;

// ------------------
// Utility functions (TODO: move in some other file)
// ------------------
namespace internal {

/**
 * @brief initialize all muxers, demuxers and protocols for libavformat
 * (does nothing if called twice during the course of one program execution)
 */
void Init() { av_register_all(); }

/**
 * @brief Extract the format context from the audio file defined at path
 * @param path location of input audio file
 * @param err
 * @return AVFormatContext
 */
AVFormatContextPtr GetFormat(const std::string& path, std::error_code& err) {
  AVFormatContext* format = avformat_alloc_context();
  if (avformat_open_input(&format, path.c_str(), NULL, NULL) != 0) {
    std::cerr << "Could not open file " << path << std::endl;
    err = std::make_error_code(std::errc::io_error);
    return nullptr;
  }
  if (avformat_find_stream_info(format, NULL) < 0) {
    std::cerr << "Could not retrieve stream info from file " << path
              << std::endl;
    err = std::make_error_code(std::errc::io_error);
    return nullptr;
  }
  return AVFormatContextPtr(format, avformat_free_context);
}

/**
 * @brief Extract the codec out of a format
 * @param format the AVFormatContextPtr obtained from GetFormat function
 * @param err
 * @return AVCodecContext
 */
AVCodecContextPtr GetCodec(const AVFormatContextPtr format,
                           std::error_code& err) {
  // Find the index of the first audio stream
  int stream_index = -1;
  for (int stream_idx = 0; stream_idx < format->nb_streams; stream_idx++) {
    if (format->streams[stream_idx]->codecpar->codec_type ==
        AVMEDIA_TYPE_AUDIO) {
      stream_index = stream_idx;
      break;
    }
  }

  if (stream_index == -1) {
    err = std::make_error_code(std::errc::io_error);
    return nullptr;
  }
  AVStream* stream = format->streams[stream_index];

  // find & open codec
  AVCodecContext* codec = avcodec_alloc_context3(NULL);
  if (avcodec_parameters_to_context(codec, stream->codecpar) < 0) {
    err = std::make_error_code(std::errc::io_error);
    return nullptr;
  }
  if (avcodec_open2(codec, avcodec_find_decoder(codec->codec_id), NULL) < 0) {
    err = std::make_error_code(std::errc::io_error);
    return nullptr;
  }

  return AVCodecContextPtr(codec, avcodec_close);
}

/**
 * @brief Initialize a resampler that will be used to convert read frames
 * @param codec the codec of the input file obtained from GetCodec
 * @param sample_rate desired output sample rate
 * @param mono set to true to export as a mono file
 * @param err
 * @return SwrContext
 */
SwrContextPtr InitResampling(const AVCodecContextPtr codec,
                             uint32_t sample_rate, std::error_code& err) {
  // TODO: may be unsafe ! Understand why swr_free use SwrContext** ...
  auto swr =
      SwrContextPtr(swr_alloc(), [](SwrContext* ptr) { swr_free(&ptr); });
  auto channel_count = codec->channels;
  auto channel_layout = codec->channel_layout;

  av_opt_set_int(swr.get(), "in_channel_count", codec->channels, 0);
  av_opt_set_int(swr.get(), "out_channel_count", channel_count, 0);

  av_opt_set_int(swr.get(), "in_channel_layout", codec->channel_layout, 0);
  av_opt_set_int(swr.get(), "out_channel_layout", channel_layout, 0);

  av_opt_set_int(swr.get(), "in_sample_rate", codec->sample_rate, 0);
  av_opt_set_int(swr.get(), "out_sample_rate", sample_rate, 0);

  av_opt_set_sample_fmt(swr.get(), "in_sample_fmt", codec->sample_fmt, 0);
  av_opt_set_sample_fmt(swr.get(), "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

  // initialize using previous parameter set
  swr_init(swr.get());
  if (!swr_is_initialized(swr.get())) {
    err = std::make_error_code(std::errc::interrupted);
    return nullptr;
  }
  return swr;
}

/**
 * @brief run an handler on each read frame from format object
 * @param format obtained from input file (GetFormat function)
 * @param codec obtained from input file (GetCodec function)
 * @param handle_frame the function to be ran on each frame
 * @param err
 */
void HandleEachFrame(const AVFormatContextPtr format,
                     const AVCodecContextPtr codec, FrameHandler handle_frame,
                     std::error_code& err) {
  // prepare to read data
  AVPacket packet;
  av_init_packet(&packet);

  // initialize frame
  auto frame_ptr = av_frame_alloc();
  if (!frame_ptr) {
    err = std::make_error_code(std::errc::not_enough_memory);
    return;
  }

  // TODO: may be unsafe ! Understand why av_frame_free use AVFrame** ...
  AVFramePtr frame(frame_ptr, [](AVFrame* ptr) { av_frame_free(&ptr); });

  // iterate through frames
  while (av_read_frame(format.get(), &packet) >= 0) {
    auto error = avcodec_send_packet(codec.get(), &packet);
    if (error < 0) {
      err = std::make_error_code(std::errc::io_error);
      return;
    }
    error = avcodec_receive_frame(codec.get(), frame.get());
    if (error < 0) {
      err = std::make_error_code(std::errc::io_error);
      return;
    }

    handle_frame(frame);
  }
  // reposition to beginning of file
  av_seek_frame(format.get(), -1, 0, AVSEEK_FLAG_ANY);
}

}  // namespace internal

// ------------------
// Pimpl Idiom class
// ------------------
class File::Impl {
 public:
  AVFormatContextPtr format;
  AVCodecContextPtr codec;
};

// ------------------
// File class
// ------------------
File::File() : impl_(std::make_unique<File::Impl>()) {}

void File::Open(const std::string& path, std::error_code& err) {
  // initialize ffmpeg
  internal::Init();

  // read format
  impl_->format = internal::GetFormat(path, err);
  if (err) {
    return;
  }

  // read codec
  impl_->codec = internal::GetCodec(impl_->format, err);
  if (err) {
    return;
  }

  // initialize sample rate and channel count from codec
  sample_rate_ = impl_->codec->sample_rate;
  channel_count_ = impl_->codec->channels;
  // default bits per sample is 16
  bits_per_samples_ = 16;
  mono_ = false;
}

uint32_t File::sample_rate() const { return sample_rate_; }
void File::set_sample_rate(uint32_t value) { sample_rate_ = value; }
uint8_t File::bits_per_samples() const { return bits_per_samples_; }
void File::set_bits_per_samples(uint8_t value) { bits_per_samples_ = value; }

uint8_t File::channel_count() const {
  if (mono_) {
    return 1;
  }
  return channel_count_;
}
void File::set_mono(bool value) { mono_ = value; }

void File::Export(const std::string& path, Format format,
                  std::error_code& err) {
  // check if initialized
  if (!impl_->codec) {
    err = std::make_error_code(std::errc::operation_not_permitted);
    return;
  }

  // make sure we want to export wav format
  if (format != Format::kWav) {
    err = std::make_error_code(std::errc::not_supported);
    return;
  }

  // init conversion parameters
  auto swr = internal::InitResampling(impl_->codec, sample_rate_, err);
  if (err) {
    return;
  }

  // init output file
  wave::File wavefile;
  auto waveerror = wavefile.Open(path, wave::kOut);
  if (waveerror) {
    // TODO: find the right error...
    err = std::make_error_code(std::errc::file_exists);
    return;
  }

  // set parameters
  wavefile.set_channel_number(channel_count());
  wavefile.set_sample_rate(sample_rate());
  wavefile.set_bits_per_sample(bits_per_samples_);

  // TODO: should be moved to internal namespace
  // Function to be ran on each frame
  auto frame_writer = [this, &wavefile, swr](AVFramePtr frame) {
    // convert frame to raw data
    float* buffer;
    av_samples_alloc((uint8_t**)&buffer, NULL, channel_count_,
                     frame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
    int frame_count =
        swr_convert(swr.get(), (uint8_t**)&buffer, frame->nb_samples,
                    (const uint8_t**)frame->data, frame->nb_samples);

    // Write raw data to file
    std::vector<float> data;
    for (int frame_idx = 0; frame_idx < frame_count; frame_idx++) {
      float mono_value = 0;
      for (int channel_idx = 0; channel_idx < channel_count_; channel_idx++) {
        auto value = buffer[frame_idx * channel_count_ + channel_idx];
        if (mono_) {
          // compute mono value
          mono_value += (value / channel_count_);
          // if the channel idx is last channel, push mono value to data
          if (channel_idx == channel_count_ - 1) {
            data.push_back(mono_value);
          }
        } else {
          data.push_back(value);
        }
      }
    }
    auto waveerror = wavefile.Write(data);
    if (waveerror != wave::kNoError) {
      std::cerr << "Failed to write to output file. Result won't be valid..."
                << std::endl;
    }
  };

  // Write each frame to file
  internal::HandleEachFrame(impl_->format, impl_->codec, frame_writer, err);
}
}  // namespace auconv
