#pragma once

#include <string>
#include <vector>

#include "common/util/FileUtil.h"

struct IsoFile {
  struct Entry {
    bool is_dir = false;
    std::string name;

    // if file
    size_t offset_in_file = 0;
    size_t size = 0;
    uint64_t hash = 0;

    // if dir
    std::vector<Entry> children;
    void print(std::string* result, const std::string& prefix) const;
  };

  std::string print() const;

  Entry root;

  int files_extracted = 0;
  bool shouldHash = false;
  uint64_t combined_hash = 0;

  IsoFile();
};

IsoFile find_files_in_iso(FILE* fp);
void unpack_iso_files(FILE* fp, IsoFile& layout, const fs::path& dest);
IsoFile unpack_iso_files(FILE* fp,
                         const fs::path& dest,
                         bool print_progress,
                         const bool hashFiles = false);
