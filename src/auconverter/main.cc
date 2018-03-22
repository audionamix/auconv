#include <iostream>
#include <system_error>

#include <cxxopts.hpp>

#include "auconv/file.h"

int main(int argc, char* argv[]) {
  // declare program options
  cxxopts::Options options("AuConverter",
                           "Compressed audio to wav PCM converter");
  options
      .add_options()
        ("h,help", "produce help message")
        ("i,input", "input file path", cxxopts::value<std::string>())
        ("o,output", "output file path", cxxopts::value<std::string>()->default_value("out.wav"));
  options.parse_positional("input");
  auto result = options.parse(argc, argv);

  // display help if requested
  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return 1;
  }

  // get sources and output paths
  std::string output_path, source_path;
  if (result.count("input")) {
    source_path = result["input"].as<std::string>();
  }
  if (result.count("output")) {
    output_path = result["output"].as<std::string>();
  }

  // open file
  auconv::File file;
  std::error_code err;
  file.Open(source_path, err);
  if (err) {
    std::cerr << "Error when opening " << source_path << " : " << err.message()
              << std::endl;
    return -1;
  }

  // export file
  file.Export(output_path, auconv::Format::kWav, err);
  if (err) {
    std::cerr << "Error when exporting to " << output_path << " : "
              << err.message() << std::endl;
    return -1;
  }

  return 0;
}
