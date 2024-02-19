#include "wiiu.h"

#include <stdlib.h>
#include <malloc.h>

#include <coreinit/thread.h>
#include <nn/nets2/somemopt.h>

// 0x300000 is the maximum for somemopt
#define NET_MEMORY_SIZE 0x300000

static OSThread memoryThread;

static int wiiu_net_memory_thread()
{
  // This provides a bunch of memory to the net stack,
  // which we can use for buffering the videostream socket
  void* mem = memalign(0x40, NET_MEMORY_SIZE);
  int rc = somemopt(SOMEMOPT_REQUEST_INIT, mem, NET_MEMORY_SIZE, SOMEMOPT_FLAGS_NONE);
  printf("SOMEMOPT_REQUEST_INIT: %d\n", rc);

  return 0;
}

static void wiiu_net_thread_deallocator(OSThread* thread, void* stack)
{
  free(stack);
}

void wiiu_net_init(void)
{
  const int stack_size = 4 * 1024 * 1024;
  uint8_t* stack = (uint8_t*)memalign(16, stack_size);
  if (!stack) {
    return;
  }

  if (!OSCreateThread(&memoryThread,
                      wiiu_net_memory_thread,
                      0, NULL,
                      stack + stack_size, stack_size,
                      0x10,
                      OS_THREAD_ATTRIB_AFFINITY_ANY | OS_THREAD_ATTRIB_DETACHED))
  {
    free(stack);
    return;
  }

  OSSetThreadName(&memoryThread, "NetMemory");
  OSSetThreadDeallocator(&memoryThread, wiiu_net_thread_deallocator);
  OSResumeThread(&memoryThread);

  // wait for somemopt to be initialized
  int rc = somemopt(SOMEMOPT_REQUEST_WAIT_FOR_INIT, NULL, 0, SOMEMOPT_FLAGS_NONE);
  printf("SOMEMOPT_REQUEST_WAIT_FOR_INIT: %d\n", rc);
}

void wiiu_net_shutdown(void)
{
  // nothing here yet
}
