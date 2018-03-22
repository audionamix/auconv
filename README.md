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
licensed under the
[LGPLv2.1](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html).  
On OSX, The install target creates `auconv.framework` which contains the shared
version of [FFmpeg](https://ffmpeg.org/) libraries and a zipped file containing
the sources, as requested by the legal information found
[here](https://ffmpeg.org/legal.html).


## Example

The `auconverter` target is a command line tool to use the `auconv` library.  
See `src/auconverter/main.cc` for more details
