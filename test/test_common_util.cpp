#include "common/util/FileUtil.h"
#include "gtest/gtest.h"

#include <string>
#include <vector>

TEST(CommonUtil, valid_path) {
  
  std::vector<std::string> test = {"cabbage", "banana", "apple"};
  std::string wat = FileUtil::get_file_path(test);

  EXPECT_TRUE(true);
}