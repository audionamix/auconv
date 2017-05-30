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
  
  file.Export("/tmp/file.wav", auconv::Format::kWav, err);
  ASSERT_FALSE(err);
  
  file.set_sample_rate(44100);
  file.set_mono(true);
  
  file.Export("/tmp/file_mono.wav", auconv::Format::kWav, err);
  ASSERT_FALSE(err);
}
