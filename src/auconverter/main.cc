#include <iostream>
#include <system_error>

#include <boost/program_options.hpp>

#include "auconv/file.h"

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
  // declare program options
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")(
      "input", po::value<std::string>(), "Path to input file")(
      "output,o", po::value<std::string>()->default_value("out.wav"),
      "Path to result")("mono", po::bool_switch()->default_value(false),
                        "Convert to mono")("sample-rate", po::value<int>(),
                                           "Output sampling rate");

  po::positional_options_description p;
  p.add("input", -1);

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  if (vm.count("help") != 0 || vm.count("input") == 0) {
    std::cout << desc << std::endl;
    return 1;
  }

  // parse input args
  auto input_file = vm["input"].as<std::string>();
  auto output_file = vm["output"].as<std::string>();
  auto mono = vm["mono"].as<bool>();

  // open file
  auconv::File file;
  std::error_code err;
  file.Open(input_file, err);
  if (err) {
    std::cerr << "Error when opening " << input_file << " : " << err.message()
              << std::endl;
    return -1;
  }
  
  // set new paramerters
  file.set_mono(mono);
  if (vm.count("sample-rate") != 0) {
    file.set_sample_rate(vm["sample-rate"].as<int>());
  }

  // export file
  file.Export(output_file, auconv::Format::kWav, err);
  if (err) {
    std::cerr << "Error when exporting to " << output_file << " : "
              << err.message() << std::endl;
    return -1;
  }

  return 0;
}
