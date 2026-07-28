#include "../include/Configuration/Configuration.h"
#include <filesystem>
#if defined(_WIN32)
#include <windows.h>
#endif

std::filesystem::path Configuration::resolveConfigDirectory() {
  namespace fs = std::filesystem;
  const auto hasDb = [](const fs::path& dir) {
    return fs::is_regular_file(dir / "DataBase.json");
  };

#if defined(_WIN32)
  wchar_t buf[MAX_PATH];
  const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
  if (n > 0 && n < MAX_PATH) {
    const fs::path exeDir = fs::path(buf).parent_path();
    const fs::path nextToExe = exeDir / "Config";
    const fs::path repoStyle = exeDir.parent_path() / "Config";
    if (hasDb(nextToExe))
      return fs::weakly_canonical(nextToExe);
    if (hasDb(repoStyle))
      return fs::weakly_canonical(repoStyle);
  }
#endif

  const fs::path cwdConfig = fs::current_path() / "Config";
  if (hasDb(cwdConfig))
    return fs::weakly_canonical(cwdConfig);

  return fs::weakly_canonical(cwdConfig);
}

AppConfig Configuration :: loadConfig () {
  DeserializeConfig d;  
  config.database = d.loadDataBase(configDir / "DataBase.json");
  config.network = d.loadNetwork(configDir / "Network.json");
  config.jwt = d.loadJwt(configDir / "Jwt.json");
  return config;
}