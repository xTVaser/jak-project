
#include "FileUtil.h"

std::string FileUtil::get_file_path(std::vector<std::string> input) {
	// filesystem is unfortunately pretty new (C++17) so unless we upgrade, you'll have to find another way to get the CWD
	// http://www.codebind.com/cpp-tutorial/c-get-current-directory-linuxwindows/ maybe?
  std::string currentPath = "temp"; // std::filesystem::current_path();
  char dirSeparator;

#ifdef _WIN32
  dirSeparator = '\\';
#else
  dirSeparator = '/';
#endif

  std::string filePath = currentPath;
  for (int i = 0; i < input.size(); i++) {
    if (input.size() == i + 1) {
      filePath = filePath + input[i];
    } else {
      filePath = filePath + input[i] + dirSeparator;
    }
  }

  return filePath;
}
