//
// @author magicxqq <xqq@xqq.im>
//

#ifdef _WIN32
    #include <Windows.h>
#endif

#include <fstream>
#include <yaml-cpp/yaml.h>
#include "IBonDriver.h"
#include "bon_driver.hpp"
#include "config.hpp"
#include "log.hpp"
#include "library.hpp"

static HMODULE hmodule = nullptr;
static Config config;

static bool LoadConfigYamlFile() {
    Log::InfoF(LOG_FILE_FUNCTION);
    // Get dll file path
    char path_buffer[_MAX_PATH] = {0};
    if (GetModuleFileNameA(hmodule, path_buffer, _MAX_PATH) == 0) {
        DWORD ret = GetLastError();
        Log::ErrorF("GetModuleFileName failed, error = %#010x", ret);
        return false;
    }

    char drive[_MAX_DRIVE] = {0};
    char dir[_MAX_DIR] = {0};
    char fname[_MAX_FNAME] = {0};
    char ext[_MAX_EXT] = {0};

    _splitpath_s(path_buffer, drive, dir, fname, ext);

    // Generate yaml file path from dll path
    std::string yaml_path(_MAX_PATH, '\0');
    sprintf(&yaml_path[0], "%s%s%s.yml", drive, dir, fname);
    Log::InfoF("Yaml FilePath: %s", yaml_path.c_str());

    return config.LoadYamlFile(yaml_path);
}

extern "C" EXPORT_API IBonDriver* CreateBonDriver() {
    Log::InfoF(LOG_FILE_FUNCTION);

    if (!config.IsLoaded()) {
        if (!LoadConfigYamlFile()) {
            return nullptr;
        }
    }

    return new BonDriver(config);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            Log::Info("DLL_PROCESS_ATTACH");
            hmodule = hinstDLL;
            LoadConfigYamlFile();
            break;
        case DLL_PROCESS_DETACH:
            Log::Info("DLL_PROCESS_DETACH");
            break;
    }
    return TRUE;
}