# libCloudSync

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/jothepro/libcloudsync)](https://github.com/jothepro/libcloudsync/releases/latest)
[![GitHub](https://img.shields.io/github/license/jothepro/libcloudsync)](https://github.com/jothepro/libcloudsync/blob/main/LICENSE)

A simple-to-use C++ interface to interact with cloud storage providers.

## Features

- ☁️ Supported Cloud Providers:
  - Nextcloud
  - Dropbox
  - Onedrive
  - GDrive
  - WebDav

## Installation

To use this library in you project, you can install it in the following ways:

### Conan
```sh
# Add conan remote:
conan remote add gitlab-cloudsync https://gitlab.com/api/v4/projects/15425736/packages/conan
```

```ini
[requires]
libcloudsync/0.0.1@jothepro/stable
```

If you don't want to build & run tests when building from source, set the [CONAN_RUN_TESTS](https://docs.conan.io/en/latest/reference/env_vars.html#conan-run-tests) variable:
```sh
conan install -if build --build missing . -e CONAN_RUN_TESTS=0
```

## Quick start

```cpp
// if this is the users first login, provide the authorization code to get a fresh access_token & refresh_token
auto credentials = OAuth2Credentials::from_authorization_code(client_id, authorization_code, redirect_uri, code_verifier);
// if you already have an access token that is still valid, create a session that will automatically refresh when the token gets outdated
auto credentials = OAuth2Credentials::from_access_token(access_token, refresh_token, valid_until);
// if the access_token that you stored is already expired, just initialize with the refresh token, it will get a new access_token for you
auto credentials = OAuth2Credentials::from_refresh_token(refresh_token);
// listen on any token update. This will be called with a fresh access_token & refresh_token if you just provided an authorization_code.
// Otherwise just the access_token will change and the refresh token will be unchanged
credentials->on_token_update([this](const std::string& access_token, std::chrono::time_point expires, const std::string& refresh_token){
    // store the new access_token/refresh_token 
})
auto cloud = CloudFactory().create_dropbox(credentials);
try {
    auto root_directory = cloud->root();
    root_directory->get_file("test.txt");
} catch(const CloudException &e) {
    std::cerr << "Sth went wrong: " << e.what() << std::endl;
}
```

```cpp
auto credentials = BasicCredentials::from_username_password("john", "password123");
auto cloud = CloudFactory().create_nextcloud("https://my.nextcloud-cloud.com", credentials);
try {
    auto file = cloud->root()->create_file("test.txt");
} catch(const CloudException &e) {
    std::cerr << "Sth went wrong: " << e.what() << std::endl;
}
```

## Development

### Build Requirements

- Conan >= 1.40
- CMake >= 3.15
- Doxygen >= 1.9.1 (optional)

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

### Integration Test

To ensure that the library behaves consistently with all cloud providers, the integration test runs a series of 
filesystem operations and checks for the correct results while being connected to the real cloud provider.

To execute the tests you need to provide valid credentials to an account for each of the supported providers.

### Example CLI

A small CLI example implementation is provided, to show the capabilities of the library.

```sh
# build the example 
cmake --build build --target CloudSyncExample
# execute the program
cd build/example
./CloudSyncExample --help
# usage example: connect to a dropbox app folder
./CloudSyncExample --dropbox --token <your-access-token>
```

#### Using the example CLI

##### GDrive

1. Get an access token from the [OAuth 2.0 Playground](https://developers.google.com/oauthplayground) with the following
   permissions:
  - `https://www.googleapis.com/auth/userinfo.profile` for loading the user profile.
  - `https://www.googleapis.com/auth/drive` if you want access to your whole gdrive space.
  - or `https://www.googleapis.com/auth/drive.appdata` if you want access an the OAuth 2.0 Playground appfolder.
2. When calling the `CloudSyncExample`, provide which root you want to access. Must match the permissions you requested:
   ```sh
   ./CloudSyncExample --gdrive --root=<root/appDataFolder> --token=<access_token>
   ```

##### Nextcloud

1. Create an app-password for your session.
2. Call `CloudSyncExample` with the domain, username and the app-password:
   ```
   ./CloudSyncExample --nextcloud --domain=https://<your_domain> --username=<username> --password=<app-password>
   ```

### Documentation

This project uses [Doxygen](https://www.doxygen.nl/index.html) for documentation.

To generate the docs, run `doxygen Doxyfile` or execute the `doxygen` target defined in the `CMakeLists.txt`.

### CI/CD

This template uses [Github Actions](https://github.com/features/actions) for automating the release of a new library version.

- The workflow `configureBuildTestCreateAndUpload.yaml` configures, builds, tests the library automatically on each push.
  When a new release is created in Github, the resulting artifact is automatically uploaded to [a public  artifactory repository](https://gitlab.com/jothepro/libcloudsync/-/packages)
- The workflow `publish-pages.yaml` automatically builds and publishes the documentation to [Github Pages](https://jothepro.github.io/libcloudsync/) when a new release is created in Github.

### Coding Conventions

The C++ code should follow a derivation of [cppbestpractices code style](https://github.com/lefticus/cppbestpractices/blob/master/03-Style.md):

- Types start with upper case and are CamelCase: `MyClass`
- Functions and variables are snake_case: `my_method`
- Constants are all upper case: `const double PI=3.14159265358979323;`
- Name private data with a `m_` prefix to distinguish it from public data. `m_` stands for "member" data
- Never use `using namespace` in a header file
- `{}` are required for blocks.
- Use `""` for including local files, `<>` is reserved for system includes
- Initialize member variables with the member initializer list
- Always use namespaces
- Use `.hpp` and `.cpp` for your file extensions
- mark single parameter constructors as `explicit`, which requires them to be explicitly called.
- do not provide any of the functions that the compiler can provide (copy constructor, copy assignment operator, 
  move constructor, move assignment operator, destructor) unless the class you are constructing does some novel form 
  of ownership.
- Indentation is 4 spaces

## Useful Links

### API Documentation

- [Dropbox](https://www.dropbox.com/developers/documentation/http/documentation)
- [OneDrive](https://docs.microsoft.com/en-us/onedrive/developer/rest-api/)
- [GDrive](https://developers.google.com/drive/api/v2/about-sdk)
- [Nextcloud](https://docs.nextcloud.com/server/18/developer_manual/client_apis/WebDAV/index.html)

### Getting developer tokens

#### Dropbox

Visit [`https://www.dropbox.com/developers/apps`](https://www.dropbox.com/developers/apps) and create a new app. 
In the tab `Settings` under the section `OAuth 2` there is a button to create an access token. 
It seems to be valid infinitely.

#### OneDrive

Create an app in the [Azure Portal](https://portal.azure.com/). Copy the client-Id from the 'General'-Section.
Follow [these](https://docs.microsoft.com/en-us/onedrive/developer/rest-api/getting-started/graph-oauth?view=odsp-graph-online) instructions to obtain a token.

#### GDrive

Go to `https://developers.google.com/oauthplayground`, select the required scope, e.g. `https://www.googleapis.com/auth/drive.appdata` 
and let the tool do its job.

#### Nextcloud

Does not support OAuth2, requires Username/Password login.

In your Nextcloud installation go to <kbd>settings</kbd> >  <kbd>user</kbd> > <kbd>security</kbd> to create a specific app-password (recommended).

## Roadmap

- Implement equality operators for Cloud, Directory, File
- Add tests for CURL wrapper (e.g. by going against a (mocked?) http server)
- implementing support for big file upload/download (chunked upload/download)
- Improve integration-test coverage
- Improve unit-test coverage
