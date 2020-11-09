# QA Labs

[![Actions Status](https://github.com/JC-Labs/qa_labs/workflows/LLVM%20Clang%20(Ubuntu)/badge.svg)](https://github.com/JC-labs/qa_labs/actions?query=workflow%3A"LLVM+Clang+(Ubuntu)")
[![Actions Status](https://github.com/JC-Labs/qa_labs/workflows/GNU%20GCC%20(Ubuntu)/badge.svg)](https://github.com/JC-labs/qa_labs/actions?query=workflow%3A"GNU+GCC+(Ubuntu)")  
[![Actions Status](https://github.com/JC-Labs/qa_labs/workflows/Microsoft%20Visual%20C++%20(Windows)/badge.svg)](https://github.com/JC-labs/qa_labs/actions?query=workflow%3A"Microsoft+Visual+C%2B%2B+(Windows)")
[![Actions Status](https://github.com/JC-Labs/qa_labs/workflows/Xcode%20(MacOS)/badge.svg)](https://github.com/JC-labs/qa_labs/actions?query=workflow%3A"Xcode+(MacOS)")

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
