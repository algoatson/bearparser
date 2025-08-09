bearparser
==========
[![Build status](https://ci.appveyor.com/api/projects/status/8p6wp0bcq2mx8208?svg=true)](https://ci.appveyor.com/project/hasherezade/bearparser)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/bc1bdddf14244559ab4786939c6f9569)](https://app.codacy.com/gh/hasherezade/bearparser/dashboard?branch=master)

### Portable Executable Parsing Library with Added Linux ELF Support

`bearparser` is a library designed for parsing Portable Executables (PE) and Executable and Linkable Format (ELF) files. Originally built for PE file parsing, it has now been extended to support ELF files, making it versatile for cross-platform executable analysis.

## Requirements:

* **Qt6 SDK**
* **Qt6 Core**
* **CMake** ([http://www.cmake.org/](http://www.cmake.org/))

  Optionally, you can build with older versions of Qt (Qt4, Qt5) by setting an appropriate CMake flag:

  ```
  cmake -G "Unix Makefiles" -D USE_QT4=ON ../
  ```

## New Features for Linux (ELF Support):

The following features have been added to `bearparser` to support ELF files on Linux:

* **ELF Bitness Detection (32/64-bit)**: Automatically detects whether an ELF file is 32-bit or 64-bit.
* **Entry Point Parsing**: Parses the entry point address from ELF headers.
* **Image Base Calculation**: Calculates the image base of ELF files from the `PT_LOAD` section header.
* **Raw and Virtual Size Calculation**: Computes both raw and virtual sizes for ELF sections.
* **Raw and Virtual Alignment Calculation**: Provides accurate alignment values for ELF sections.
* **ELFFileBuilder**: Introduced a basic `ELFFileBuilder` to assist with building ELF structures.
* **32/64-bit Header Support via `std::variant`**: Uses `std::variant` to handle the different ELF header structures based on bitness.
* **Lazy Caching**: Implements lazy caching for expensive operations to improve performance.
* **Core ELFFile Encapsulation**: Introduced encapsulation of core ELF data structures within the `ELFFile` class for better modularity and management.

## Example:

To build and compile the library:

```
cmake -G "Unix Makefiles" -D USE_QT4=ON ../
make
```

For detailed ELF file parsing examples, see the [Wiki](https://github.com/hasherezade/bearparser/wiki).

## PE-bear

In addition to `bearparser`, check out **PE-bear**, a GUI application built using `bearparser`.

![PE-bear logo](https://github.com/hasherezade/pe-bear/blob/main/logo/main_ico.png)

More info: [PE-bear GitHub](https://github.com/hasherezade/pe-bear)
