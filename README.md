# C++ Library Template

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/jothepro/cmake-conan-library-template)](https://github.com/jothepro/cmake-conan-library-template/releases/latest)
[![GitHub](https://img.shields.io/github/license/jothepro/cmake-conan-library-template)](https://github.com/jothepro/cmake-conan-library-template/blob/main/LICENSE)

A basic **C++ library template** utilizing [CMake](https://cmake.org/) and [Conan](https://conan.io/).

## Features

- 🎣 Dependency management with **Conan**
- 🍭 Build configuration with **CMake**
- 🧩 Automatic publishing of artifacts to **Artifactory** with Github Actions
- 📑 Automatic publishing of **Doxygen** documentation with Github Actions
- 🚀 Preconfigured for Unit-Testing with **Catch2**

## Installation

To use this library in you project, you can install it in the following ways:

### Conan
```sh
# Add artifactory repository as remote:
conan remote add jothepro-conan-public https://jothepro.jfrog.io/artifactory/api/conan/conan-public
# Install  a release of `mylibrary`
conan install --remote jothepro-conan-public mylibrary/0.1.7@jothepro/stable
```

If you don't want to build & run tests when building from source, set the [CONAN_RUN_TESTS](https://docs.conan.io/en/latest/reference/env_vars.html#conan-run-tests) variable:
```sh
install --remote jothepro-conan-public mylibrary/0.1.7@jothepro/stable -e CONAN_RUN_TESTS=0
```

Pre-Releases are available in the `beta` channel:
```sh
conan install --remote jothepro-conan-public mylibrary/0.1.8@jothepro/beta
```
   

## Development

### Build Requirements

- Conan >= 1.30
- CMake >= 3.15
- Doxygen 1.9.1 (optional)

### Build

- **Commandline**:
  ```sh
  # Create build folder for out-of-source build
  mkdir build && cd build
  # Install Dependencies with Conan
  conan install ..
  # Configure, Build & Test
  conan build ..
  ```
- **Clion**: Install the [Conan Plugin](https://plugins.jetbrains.com/plugin/11956-conan) before configuring & building the project as usual.

### Test

This template uses [Catch2](https://github.com/catchorg/Catch2) for testing. The Unit-tests are defined in `test`.

- **Commandline**: To run just the unit-tests, you can run `conan build .. --test`.
- **CLion**: Execute the `MyLibraryTest` target

### Documentation

This template uses [Doxygen](https://www.doxygen.nl/index.html) for documenation.

To generate the docs, run `doxygen Doxyfile` or execute the `doxygen` target defined in the `CMakeLists.txt`.

### CI/CD

This template uses [Github Actions](https://github.com/features/actions) for automating the release of a new library version.

- The workflow `configureBuildTestCreateAndUpload.yaml` configures, builds, tests the library automatically on each push.
  When a new release is created in Github, the resulting artifact is automatically uploaded to [a public  artifactory repository](https://jothepro.jfrog.io/ui/repos/tree/General/conan-public%2F_%2Fmylibrary)
- The workflow `publish-pages.yaml` automatically builds and publishes the documentation to [Github Pages](https://jothepro.github.io/cpp-library-template/) when a new release is created in Github.

## Directory Structure

```
.
├── CMakeLists.txt (1)
├── Doxyfile (2)
├── LICENSE (3)
├── README.md (4)
├── conanfile.py (5)
├── docs (6)
│   ├── doxygen-awesome-css (7)
│   ├── doxygen-custom (8)
│   │   └── ...
│   ├── example-page.dox (9)
│   └── img (10)
│       └── ...
├── src (11)
│   ├── CMakeLists.txt (12)
│   ├── example.cpp (13)
│   └── include (14)
│       └── MyLibrary (15)
│           └── example.hpp (16)
├── test (17)
│   ├── CMakeLists.txt (18)
│   └── mylibrarytest.cpp (19)
└── test_package (20)
    ├── CMakeLists.txt (21)
    ├── conanfile.py (22)
    └── example.cpp (23)

```

1. Root `CMakeLists.txt`. Includes Library Sources (11) and unit tests (18).
2. Doxyfile for documentation generation. `CMakeLists.txt` (1) defines a target `doxygen` to build the documentation.
3. License file.
4. The Readme you currently read.
5. Conanfile. Used to install dependencies & publishing the package.
6. Documentation subdirectory. Generated docs will pe placed under `docs/html`.
7. Submodule containing the custom-css for doxygen.
8. Project-specific doxygen customizations.
9. Example documentation file. All `.dox` files in this dir will be automatically included in the documentation.
10. Images for documentation.
11. Library sources folder.
12. `CMakeLists.txt` for library.
13. Private source file.
14. Public headers folder.
15. Library namespace.
16. Public header file example.
17. Unit tests folder.
18. `CMakeLists.txt` that defines unit tests.
19. Example unit test file.
20. Conan linking test directory.
21. CMakeLists.txt that defines an example project that links the library.
22. Conanfile that defines linking test.
23. Example sources that require the library to build & run successfully.


## Credits

This template is inspired by these talks:

- [C++Now 2017: Daniel Pfeifer “Effective CMake"](https://www.youtube.com/watch?v=bsXLMQ6WgIk)
  
- [CppCon 2018: Mateusz Pusz “Git, CMake, Conan - How to ship and reuse our C++ projects”](https://www.youtube.com/watch?v=S4QSKLXdTtA)