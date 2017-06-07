# AuConv

Yet another audio format convertion library

## Usage

```c++
#include <system_error>

#include "auconv/file.h"

int main(int argc, char* argv[]) {
  std::error_code err;
  auconv::File file;

  file.Open("/path/to/some/file.mp3", err);
  if (err) {
    return 1;
  }

  file.Export("/path/to/new/file.wav", auconv::Format::kWav, err);
  if (err) {
    return 1;
  }
  return 0;
}
```

## License

This project uses libraries from the [FFmpeg project](https://ffmpeg.org/)
licensed under the [GPLv2.0](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

[FFmpeg](https://ffmpeg.org/) libraries are compiled using the `--enable-gpl`
(see CMakeLists.txt) so sources that use this project does not need to be
public. However, credits must be given to their team. For more information, see
[this](https://ffmpeg.org/legal.html).


## Example

The `auconverter` target is a command line tool to use the `auconv` library.  
See `src/auconverter/main.cc` for more details
