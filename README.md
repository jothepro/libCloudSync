# libCloudSync

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/jothepro/libcloudsync)](https://github.com/jothepro/libcloudsync/releases/latest)
[![GitHub](https://img.shields.io/github/license/jothepro/libcloudsync)](https://github.com/jothepro/libcloudsync/blob/main/LICENSE)

A simple to use C++ interface to interact with cloud storage providers.

## Features

- ☁️ Supported Cloud Providers:
  - Nextcloud
  - Owncloud
  - Dropbox
  - Box
  - Onedrive
  - Gdrive
  

## Installation

To use this library in you project, you can install it in the following ways:

### Conan
```sh
# Add artifactory repository as remote:
conan remote add jothepro-conan-public https://jothepro.jfrog.io/artifactory/api/conan/conan-public
# Install  a release of `libcloudsync`
conan install --remote jothepro-conan-public libcloudsync/0.1.7@jothepro/stable
```

If you don't want to build & run tests when building from source, set the [CONAN_RUN_TESTS](https://docs.conan.io/en/latest/reference/env_vars.html#conan-run-tests) variable:
```sh
install --remote jothepro-conan-public libcloudsync/0.1.7@jothepro/stable -e CONAN_RUN_TESTS=0
```

Pre-Releases are available in the `beta` channel:
```sh
conan install --remote jothepro-conan-public libcloudsync/0.1.8@jothepro/beta
```
   

## Development

### Build Requirements

- Conan >= 1.40
- CMake >= 3.15
- Doxygen 1.9.2 (optional)

### Build

- **Commandline**:
  ```sh
  # install dependencies with Conan
  conan install -if build --build missing .
  # configure, build & test with Conan
  conan build -bf build .
  # or configure & build directly with CMake
  cmake -S . -B build && cmake --build build
  # and execute the tests with ctest
  ctest --test-dir build/test
  ```
- **Clion**: Install the [Conan Plugin](https://plugins.jetbrains.com/plugin/11956-conan) before configuring & building the project as usual.

### Test

This library uses [Catch2](https://github.com/catchorg/Catch2) for testing. The Unit-tests are defined in `test`.

- **Commandline**: To run just the unit-tests, you can run `conan build -bf build --test .`.
- **CLion**: Execute the `CloudSyncTest` target

### Example

A small CLI example implementation is provided, to show the capabilities of the library.

```bash
# build the example 
cmake --build build --target CloudSyncExample
# execute the program
cd build/example
./CloudSyncExample --help
# usage example: connect to a dropbox app folder
./CloudSyncExample --dropbox --token <your-access-token>
```

### Documentation

This project uses [Doxygen](https://www.doxygen.nl/index.html) for documenation.

To generate the docs, run `doxygen Doxyfile` or execute the `doxygen` target defined in the `CMakeLists.txt`.

### CI/CD

This template uses [Github Actions](https://github.com/features/actions) for automating the release of a new library version.

- The workflow `configureBuildTestCreateAndUpload.yaml` configures, builds, tests the library automatically on each push.
  When a new release is created in Github, the resulting artifact is automatically uploaded to [a public  artifactory repository](https://jothepro.jfrog.io/ui/repos/tree/General/conan-public%2F_%2Flibcloudsync)
- The workflow `publish-pages.yaml` automatically builds and publishes the documentation to [Github Pages](https://jothepro.github.io/libcloudsync/) when a new release is created in Github.


## Useful Links

### API Documentation

- [Box](https://developer.box.com/reference/)
- [Dropbox](https://www.dropbox.com/developers/documentation/http/documentation)
- [OneDrive](https://docs.microsoft.com/en-us/onedrive/developer/rest-api/)
- [GDrive](https://developers.google.com/drive/api/v3/reference)
- [Nextcloud](https://docs.nextcloud.com/server/18/developer_manual/client_apis/WebDAV/index.html)

### Getting developer tokens

#### Box

Go to [`https://app.box.com/developers/console`](https://app.box.com/developers/console) and create a new app. In the left-navigation click on `Configuration`. You can find a button there that will create a developer-token for you. The generated token will be valid for 60 minutes.

#### Dropbox

Visit [`https://www.dropbox.com/developers/apps`](https://www.dropbox.com/developers/apps) and create a new app. In the tab `Settings` under the section `OAuth 2` there is a button to create an access token. It seems to be valid infinitely.

#### OneDrive

Create an app in the [Azure Portal](https://portal.azure.com/). Copy the client-Id from the 'General'-Section.
Follow [these](https://docs.microsoft.com/en-us/onedrive/developer/rest-api/getting-started/graph-oauth?view=odsp-graph-online) instructions to obtain a token.

#### GDrive

Go to `https://developers.google.com/oauthplayground`, select the required scope, e.g. `https://www.googleapis.com/auth/drive.appdata` and let the tool do its job.

#### Nextcloud

Does not support OAuth2, requires Username/Password login.

In your Nextcloud installation go to `/settings/user/security` to create a specific app-password (recommended).