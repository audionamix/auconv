#include <gtest/gtest.h>
#include "auconv/file.h"

const std::string gResourcePath(TEST_RESOURCES_PATH);

TEST(File, Convert) {
  std::string path = gResourcePath + "/gtr-jazz.mp3";
  std::error_code err;

  auconv::File file;
  file.Open(path, err);
  ASSERT_FALSE(err);
  ASSERT_EQ(file.sample_rate(), 48000);
  ASSERT_EQ(file.channel_count(), 2);
  ASSERT_EQ(file.bits_per_samples(), 16);

  file.Export("/tmp/file.wav", auconv::Format::kWav, err);
  ASSERT_FALSE(err);

  file.set_sample_rate(44100);
  file.set_bits_per_samples(24);
  file.set_mono(true);

  file.Export("/tmp/file_mono.wav", auconv::Format::kWav, err);
  ASSERT_FALSE(err);
}

TEST(File, ConvertErrorM4a) {
  std::string path = gResourcePath + "/EverythingWasEverything.m4a";
  std::error_code err;

  auconv::File file;
  file.Open(path, err);
  ASSERT_FALSE(err);
  ASSERT_EQ(file.sample_rate(), 44100);
  ASSERT_EQ(file.channel_count(), 2);

  file.Export("/tmp/file-m4a.wav", auconv::Format::kWav, err);
  ASSERT_FALSE(err);
}

TEST(File, ConvertErrorMP3) {
  std::string path = gResourcePath + "/MajorLazer-LeanOn.mp3";
  std::error_code err;

  auconv::File file;
  file.Open(path, err);
  ASSERT_FALSE(err);
  ASSERT_EQ(file.sample_rate(), 44100);
  ASSERT_EQ(file.channel_count(), 2);
  //  ASSERT_EQ(file.bits_per_samples(), 16);

  file.Export("/tmp/file-mp3.wav", auconv::Format::kWav, err);
  ASSERT_FALSE(err);
}

TEST(File, ConvertErrorAIFF) {
  std::string path = gResourcePath + "/The_Chainsmokers-Something_Just_Like_Thi-LLS.aif";
  std::error_code err;

  auconv::File file;
  file.Open(path, err);
  ASSERT_FALSE(err);
  ASSERT_EQ(file.sample_rate(), 44100);
  ASSERT_EQ(file.channel_count(), 2);
  //  ASSERT_EQ(file.bits_per_samples(), 16);

  file.Export("/tmp/file-aif.wav", auconv::Format::kWav, err);
  ASSERT_FALSE(err);
}

TEST(File, ConvertErrorWhiteNoise) {
  std::string path = gResourcePath + "/White-Noise-At-Beginning-After-Conversion.aif";
  std::error_code err;
  
  auconv::File file;
  file.Open(path, err);
  ASSERT_FALSE(err);
  ASSERT_EQ(file.sample_rate(), 44100);
  ASSERT_EQ(file.channel_count(), 2);
  //  ASSERT_EQ(file.bits_per_samples(), 16);
  
  file.Export("/tmp/file-noise.wav", auconv::Format::kWav, err);
  ASSERT_FALSE(err);
}

TEST(File, Clicking) {
  std::string path = gResourcePath + "/clicking.m4a";
  std::error_code err;
  
  auconv::File file;
  file.Open(path, err);
  ASSERT_FALSE(err);
  ASSERT_EQ(file.sample_rate(), 44100);
  ASSERT_EQ(file.channel_count(), 2);
  //  ASSERT_EQ(file.bits_per_samples(), 16);
  
  file.Export("/tmp/file-clicking.wav", auconv::Format::kWav, err);
  ASSERT_FALSE(err);
}

