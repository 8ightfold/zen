#ifndef ENGINE_GENERATE_PDB_HPP
#define ENGINE_GENERATE_PDB_HPP

#include "curl_global_init.hpp"

#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

#include <zen/filesystem.hpp>

namespace zen::curl {
    void
    generate_pdb();

    bool
    get_cv2pdb(fs::path);

    bool
    unzip_and_move_cv2pdb(fs::path parent_dir, std::string path, std::string to);
}

#endif //ENGINE_GENERATE_PDB_HPP
