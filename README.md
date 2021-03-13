BonDriver_EPGStation
======
BonDriver for [EPGStation](https://github.com/l3tnun/EPGStation) that could be used in BonDriver-based software like TVTest, EDCB, BonRecTest, etc.

## Features
- Both EPGStation v1 and v2 are supported
- http / https / http2 supported
- Proxy configuration
- UserAgent & custom request headers configuration
- CRT is static-linked so does not require extra VCRuntime
- Maintainable code

## Prebuilt binaries
Built in Visual Studio 2019. You must choose the right architecture (x86/x64).

https://github.com/xqq/BonDriver_EPGStation/releases

## Config
You must put `BonDriver_EPGStation.yml` in the same folder as the dll and keep the same file name.

```
baseURL: http://192.168.1.101   # required
version: v2                     # required, v1 or v2
mpegTsStreamingMode: 0          # required
showInactiveServices: false     # optional, default to false
userAgent: BonDriver_EPGStation # optional
proxy: socks5://127.0.0.1:1080  # optional, protocol could be http/https/socks4/socks4a/socks5/socks5h
headers:                        # optional
  X-Real-Ip: 114.514.810.893
basicAuth:                      # optional, deprecated
  user: admin
  password: admin
```

## Build
### Preparing
CMake >=3.13 and a C++17 compatible compiler is necessary.
```bash
git clone https://github.com/xqq/BonDriver_EPGStation.git
cd BonDriver_EPGStation
git submodule update --init --recursive
```

### CMake generate
The cmake configuring of libcurl could be extremely slow and you should just wait patiently.
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=MinSizeRel -A Win32 ..     # Build for x86 (Win32)
cmake -DCMAKE_BUILD_TYPE=MinSizeRel -A x64 ..       # or Build for x64 (x64)
```

### Compiling
```bash
cmake --build . --config MinSizeRel -j8
```

Visual Studio 2019 (CMake development) or CLion (MSVC toolchain) is recommended.

## License
```
MIT License

Copyright (c) 2021 magicxqq <xqq@xqq.im>
```
