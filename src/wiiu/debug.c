#ifdef DEBUG
#include <sys/iosupport.h>
#include <coreinit/debug.h>

#include <stdlib.h>
#include <stdio.h>

static ssize_t wiiu_log_write(struct _reent* r, void* fd, const char* ptr, size_t len) {
  OSReport("%*.*s", len, len, ptr);
  return len;
}

static const devoptab_t dotab_stdout = {
  .name = "stdout_console",
  .write_r = wiiu_log_write,
};

void Debug_Init(){
  devoptab_list[STD_OUT] = &dotab_stdout;
  devoptab_list[STD_ERR] = &dotab_stdout;
}
#endif
