#include <zen/curl/generate_pdb.hpp>

#include <fstream>
#include <thread>

#include <zen/filesystem.hpp>
#include <zen/curl/retrieve_data.hpp>
#include <zen/os/invoke_executable.hpp>

#include <nlohmann_json.hpp>
#include <zip/zip.h>

namespace zen::curl {
    using json = nlohmann::json;

    // internal callbacks
    int
    on_extract_entry(const char *filename, void *arg) {
        static int i = 0;
        int n = *(int *)arg;
        printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

        return 0;
    }



    void
    generate_pdb() {
        char module_buff[MAX_PATH] {};
        GetModuleFileNameA(NULL, module_buff , MAX_PATH);
        if(get_cv2pdb(module_buff)) {
            std::string out_args { "-n " };
            auto bin_path = fs::path { module_buff }.parent_path() / "extra" / "bin.exe";
            if(fs::exists(bin_path)) {
                out_args += (fs::path { module_buff }.parent_path() / "extra" / "bin.exe").string();
                auto cmds = os::invoke_executable("extra/cv2pdb.exe", out_args);

                std::string new_name { std::string { fs::path { module_buff }.filename().stem().string() + ".pdb" } };
                if(not fs::exists(new_name)) fs::rename("extra/bin.pdb", new_name);
                fs::remove(bin_path);
            }
        }
        else throw custom_exception {
            "error when getting pdb generator."
        };
    }

    bool
    get_cv2pdb(fs::path module_path) {
        auto parentpath = module_path.parent_path();
        auto filepath = parentpath / "cv2pdb.exe";

        if(not fs::exists(parentpath / "extra" / "cv2pdb.exe") || not fs::exists(parentpath / "extra" / "dviewhelper.dll")) {
            std::string raw_json { api::release_request("rainers", "cv2pdb") };
            auto parsed_json = json::parse(raw_json);

            std::string link = parsed_json["assets"][0]["browser_download_url"];
            std::string version = "<null>";
            if(link.empty()) {
                std::cout << "zip link could not be found.\n";
                return false;
            }
            else {
                version = fs::path { link }.parent_path().filename().string();
                auto bytes { api::download_bytes(link) };

                if(bytes.empty()) {
                    std::cout << "zip data could not be read.\n";
                    return false;
                }
                else {
                    auto zip_path = parentpath / "cv2pdb.zip";
                    std::ofstream of(zip_path, std::ios::out | std::ios::binary);
                    of.write(bytes.data(), bytes.size());
                    of.close();

                    return unzip_and_move_cv2pdb(parentpath, "cv2pdb.zip", (parentpath / "tmp").string());
                }
            }
        }
        return true;
    }

    bool
    unzip_and_move_cv2pdb(fs::path parent_dir, std::string path, std::string to) {
        int args = 2;
        auto ret = zip_extract(path.c_str(), to.c_str(), on_extract_entry, &args);
        switch(ret) {
            case ZIP_EINVZIPNAME:
                std::cout << "cannot parse zip archive name.\n";
                return false;
            case ZIP_EMEMSET:
                std::cout << "cannot memset zip archive.\n";
                return false;
            case ZIP_ENOINIT:
                std::cout << "cannot initialize zip archive reader: " << get_last_error() << ".\n";
                return false;
        }

        fs::rename(fs::path { to } / "cv2pdb.exe", parent_dir / "extra" / "cv2pdb.exe");
        fs::rename(fs::path { to } / "dviewhelper.dll", parent_dir / "extra" / "dviewhelper.dll");

        fs::remove(parent_dir / "cv2pdb.zip");
        fs::remove_all(parent_dir / "tmp");

        std::cout << "extracted to " << (parent_dir / "extra") << '\n';

        return true;
    }
}