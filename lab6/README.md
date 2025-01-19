# Project Build Instructions

## Requirements
To successfully build and run this project, ensure the following dependencies are installed on your system:

- **Qt**: Version >= 5.15 (for GUI)
- **CMake**: Version >= 3.10 (for Server)
- **Make**: Standard build tool

---

## Build GUI
```sh
$ cd GUI/build
$ qmake ..
$ make
```

## Build Server
```sh
$ cd Server/build
$ cmake ..
$ make
```
