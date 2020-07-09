<img align="left" width="60" height="60" src="logo/DemBones.png">

# Dem Bones
[![BSD3 Clause](https://img.shields.io/badge/license-BSD3_Clause-blue.svg)](LICENSE.md)
[![Version](https://img.shields.io/badge/version-1.1.0-green.svg)](VERSION.md)

This repository contains an implementation of [Smooth Skinning Decomposition with Rigid Bones](http://binh.graphics/papers/2012sa-ssdr/), 
an automated algorithm to extract the *Linear Blend Skinning* (LBS) with bone transformations from a set of example meshes. 
*Skinning Decomposition* can be used in various tasks:
- converting any animated mesh sequence, e.g. geometry cache, to LBS, which can be replayed in popular game engines,
- solving skinning weights from shapes and skeleton poses, e.g. converting blendshapes to LBS,
- solving bone transformations for a mesh animation given skinning weights.

This project is named after "The Skeleton Dance" by Super Simple Songs.

## Contents
- `include/DemBones`: C++ header-only core library using [Eigen](http://eigen.tuxfamily.org) and [OpenMP](https://www.openmp.org/). Check out the documentations in [docs/index.html](docs/index.html).
- `bin`: pre-compiled command line tools for Windows, Linux, and MacOS that read and write [FBX](https://www.autodesk.com/products/fbx/overview) and [Alembic](https://www.alembic.io/) files. Check out the usage by running `DemBones --help`.
- `src/command`: source code for the command line tool. Check out `AbcReader.cpp`, `FbxReader.cpp`, and `FbxWriter.cpp` for the usage of the core library.
- `data`: input/output test data for the command line tool. Run and check out the scripts `run.bat` (Windows) or `./run.sh` (Linux/MacOS).

## Compiling
Tested platforms:
- Visual Studio 2019 on Windows 10 x64
- g++ 9.3.0 on Ubuntu Linux 20.14
- LLVM 10.0.0 (Homebrew) on MacOS 10.13.6

Compiling steps:
1. Install [cmake](https://cmake.org/)
2. Copy the following libraries to their respective folders in `ExtLibs` so that [cmake](https://cmake.org/) can find these paths:
    - [Eigen 3.3.7](https://eigen.tuxfamily.org/) with path `ExtLibs/Eigen/Eigen/Dense`,
    - [Alembic (from Maya 2020 Update 2 DevKit)](https://www.autodesk.com/developer-network/platform-technologies/maya) with path `ExtLibs/Alembic/include/Alembic/Abc/All.h`,
    - [FBXSDK 2020.0.1](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0) with path `ExtLibs/FBXSDK/include/fbxsdk.h`,
    - [tclap 1.2.2](http://tclap.sourceforge.net/) with path `ExtLibs/tclap/include/tclap/CmdLine.h`,
3. Run cmake:
```
mkdir build
cd build
cmake ..
```
4. Build: `$ cmake --build . --config Release --target install`

> **Notes for Linux** 
>   - You may need to install some libriries: [libxml2-dev](http://xmlsoft.org/) (run `$ sudo apt-get install libxml2-dev`) and [zlib-dev](https://zlib.net/) (run `$ sudo apt-get install zlib1g-dev`).
>
>   - `bin/Linux/DemBones` was compiled with flag [`-D_GLIBCXX_USE_CXX11_ABI=0`](https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html) 
>	  for compatibility with the pre-complied [Alembic (from Maya 2020 Update 2 DevKit)](https://www.autodesk.com/developer-network/platform-technologies/maya).
>
> **Notes for MacOS** 
>   - Apple Clang (default compiler) does not support OpenMP and Eigen. We recommend using [LLVM](https://formulae.brew.sh/formula/llvm). 
>     Assuming LLVM is installed in the default path: `/usr/local/opt/llvm/bin`, instead of `cmake ..`, you can run: 
>     ```
>     $ cmake -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ ..
>     ```
>
>   - The pre-compiled tool `bin/DemBones` requires dynamic [libomp for LLVM](https://openmp.llvm.org/). If you have an error messeage related to OpenMP, 
>     please install `libomp`, e.g. with [Homebrew](https://brew.sh/) using  `$ brew install libomp`.
>
>   - `bin/MacOS/DemBones` was compiled with the optimization flag `-O3`. It looks like LLVM uses [fast math](http://eigen.tuxfamily.org/bz/show_bug.cgi?id=950) 
>     so the results are slightly different with those generated by Windows version.
>     Removing optimization flags (in `CMakeLists.txt`) helps to reproduce the same results with Windows version but the tool will run 10x slower.


## References

If you use the library or the command line tool, please cite the paper:  

> *Binh Huy Le* and *Zhigang Deng*. **[Smooth Skinning Decomposition with Rigid Bones](http://binh.graphics/papers/2012sa-ssdr/)**. ACM Transactions on Graphics 31(6), Proceedings of ACM SIGGRAPH Asia 2012.

BibTeX:

```
@article{LeDeng2012,
    author = {Le, Binh Huy and Deng, Zhigang},
    title = {Smooth Skinning Decomposition with Rigid Bones},
    journal = {ACM Trans. Graph.},
    volume = {31},
    number = {6},
    year = {2012}
} 
```

The skinning weights smoothing regularization was published in the paper:

> *Binh Huy Le* and *Zhigang Deng*. **[Robust and Accurate Skeletal Rigging from Mesh Sequences](http://binh.graphics/papers/2014s-ske/)**. ACM Transactions on Graphics 33(4), Proceedings of ACM SIGGRAPH 2014.

## Authors

<p align="center"><a href="https://seed.ea.com"><img src="logo/SEED.jpg" width="150px"></a><br>
<b>Search for Extraordinary Experiences Division (SEED) - Electronic Arts <br> http://seed.ea.com</b><br>
We are a cross-disciplinary team within EA Worldwide Studios.<br>
Our mission is to explore, build and help define the future of interactive entertainment.</p>

Dem Bones was created by Binh Le (ble@ea.com). The [logo](logo/DemBones.png) was designed by Phuong Le.

## Contributing

Before you can contribute, EA must have a Contributor License Agreement (CLA) on file that has been signed by each contributor.
You can sign here: http://bit.ly/electronic-arts-cla

## Licenses

- The source code, including `include/DemBones` and `src/command`, uses *BSD 3-Clause License* as detailed in [LICENSE.md](LICENSE.md)
- The pre-compiled command line tool `bin/DemBones`(`.exe`) uses third party libraries: Eigen, tclap, Alembic, FBXSDK, and zlib with licenses in [3RDPARTYLICENSES.md](3RDPARTYLICENSES.md)

