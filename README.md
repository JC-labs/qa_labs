# QA Labs

### Requirements
- premake5 ([page](https://premake.github.io/), [repo](https://github.com/premake/premake-core))
- doctest ([repo](https://github.com/onqtam/doctest), will be automaticly downloaded using `premake`)

### Build steps
- Acquire premake 5
- Run `premake5 [action]` ([docs](https://github.com/premake/premake-core/wiki/Using-Premake)):
    - on `windows`: `premake5 vs2019`
    - on `linux`: `premake5 gmake2`
    - on `macos`: `premake5 xcode4`
- Build application using selected tools:
    - on `windows`: open `project/*.sln` using `Visual Studio`
    - on `linux`: run `cd project && make && cd ..`
    - on `macos`: open `project` directory using `XCode`
- Run tests from `output/{configuration}/{filename}`
    - for example `output/window_x64_release/lab_1_test.exe`
- Enjoy using the library in your projects :)
