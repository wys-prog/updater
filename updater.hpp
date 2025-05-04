#pragma once

#include "../wmmbase.hpp"
#include "../wyland-runtime/wylrt.hpp"

WYLAND_BEGIN

class UpdateException : public runtime::wyland_runtime_error, std::runtime_error {
public:
  UpdateException(const std::string &what, const std::string &from) 
    : runtime::wyland_runtime_error(what.c_str(), "upadte exception", from.c_str(), typeid(this).name(), 0, 0, nullptr, nullptr, 0), 
      std::runtime_error(what + "\nfrom: " + from)
  {
    std::cerr << "[e]: update exception (uncaughtable):\n" << this->fmterr() << std::endl;
    throw this;
  }
};

#ifdef _WIN32
#include <windows.h>

void update(const std::string &updater_path) {
  STARTUPINFOA si = { sizeof(si) };
  PROCESS_INFORMATION pi;

  CreateProcessA(
    updater_path.c_str(), 
    NULL,
    NULL, NULL, FALSE, 0,
    NULL, NULL,
    &si, &pi
  );


  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  ExitProcess(0);
}

#else
#include <unistd.h>

void update(const std::string &updater_path, const std::string &update_file) {
  execl(updater_path.c_str(), updater_path.c_str(), update_file.c_str(), (char*)nullptr);

  throw UpdateException("cannot update", __func__);
}

#endif // ? _WIN32 


WYLAND_END