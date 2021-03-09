// TODO - hopefully theres a way to hide this in one file...damn macOS being 10 years behind everything else
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || (defined(__cplusplus) && __cplusplus >= 201703L)) && defined(__has_include)
#if __has_include(<filesystem>) && (!defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500)
#define GHC_USE_STD_FS
#include <filesystem>
namespace fs = std::filesystem;
#endif
#endif
#ifndef GHC_USE_STD_FS
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#endif

#include "gtest/gtest.h"

#include "common/util/FileUtil.h"
#include "common/log/log.h"

// Running subsets of tests, see:
// -
// https://github.com/google/googletest/blob/620659ed92829a88ee34134c782bf5b5aa5a0a0c/googletest/docs/advanced.md#running-a-subset-of-the-tests
// This can set via:
// - --gtest_filter="" CLI arg
// - 'GTEST_FILTER' environment variable,
// - or below in code by adding `::testing::GTEST_FLAG(filter) = "Test_Cases1*";` below
//
// I've set things up so VS has a run configuration that runs all tests with "Draft" in the name
// to make it easier to test a subset of tests

int main(int argc, char** argv) {
  lg::initialize();

  ::testing::InitGoogleTest(&argc, argv);

  // Re-init failed folder
  std::string failedFolder = file_util::get_file_path({"test/goalc/source_generated/failed/"});
  if (fs::exists(failedFolder)) {
    fs::remove_all(failedFolder);
  }
  fs::create_directory(failedFolder);

  return RUN_ALL_TESTS();
}
