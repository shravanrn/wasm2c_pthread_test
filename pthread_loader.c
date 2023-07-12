#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "pthread_test.wasm.h"

#include "wasm2c_rt_minwasi.h"

#include <sys/mman.h>

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#define PAGE_SIZE 65536

struct w2c_env {
    wasm_rt_memory_t* memory_ref;
};

wasm_rt_memory_t memory;

wasm_rt_memory_t* w2c_env_memory(struct w2c_env* mem_env) {
    return mem_env->memory_ref;
}

//threads env
struct w2c_wasi {
    w2c_pthread__test* main_thread_env;
};

// returns status where the status is a unique non-negative integer thread ID
// (TID) of the new thread (see Design choice: thread IDs) or a negative number
// representing an error if the host failed to spawn the thread. The host
// implementing wasi_thread_spawn will call a predetermined function export
// (wasi_thread_start) in a new WebAssembly instance --- in the new thread, call
// the child instance's exported entry function with the thread ID and the start
// argument: wasi_thread_start(tid, start_arg)

struct loader_thread_arg {
    struct w2c_wasi* thread_env;
    u32 start_arg;
};

static void* thread_main(void* arg) {
    struct loader_thread_arg* thread_arg_ptr = (struct loader_thread_arg*) arg;

    struct loader_thread_arg thread_arg;
    memcpy(&thread_arg, thread_arg_ptr, sizeof(thread_arg));
    free(thread_arg_ptr);

    long tid = syscall(SYS_gettid);
    // void w2c_pthread__test_wasi_thread_start(w2c_pthread__test* instance, u32 tid, u32 start_arg);
    w2c_pthread__test_wasi_thread_start(thread_arg.thread_env->main_thread_env, (u32) tid, thread_arg.start_arg);
    return NULL;
}

/* import: 'wasi' 'thread-spawn' */
u32 w2c_wasi_thread0x2Dspawn(struct w2c_wasi* thread_env, u32 start_arg) {
    pthread_t tid;
    struct loader_thread_arg* arg = (struct loader_thread_arg*) malloc(sizeof(struct loader_thread_arg));
    arg->thread_env = thread_env;
    arg->start_arg = start_arg;
    if (pthread_create(&tid, NULL, thread_main, arg) != 0) {
        return -1;
    }

    return (u32) tid;
}

int main(int argc, char** argv) {

  assert(WASM_RT_USE_MMAP);
  memset(&memory, 0, sizeof(memory));
  memory.max_pages = wasm2c_pthread__test_max_env_memory;
  memory.data = mmap(NULL, memory.max_pages * PAGE_SIZE, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  memory.pages = wasm2c_pthread__test_min_env_memory;
  memory.size = memory.pages * PAGE_SIZE;
  mprotect(memory.data, memory.size, PROT_READ | PROT_WRITE);

  struct w2c_env mem_env;
  mem_env.memory_ref = &memory;

  wasm_rt_init();
  minwasi_init();

  w2c_wasi__snapshot__preview1 wasi_data;
  memset(&wasi_data, 0, sizeof(wasi_data));
  wasi_data.instance_memory = &memory;

  minwasi_init_instance(&wasi_data);

  w2c_pthread__test m;

  struct w2c_wasi thread_env;
  thread_env.main_thread_env = &m;

  wasm2c_pthread__test_instantiate(&m, &mem_env, &thread_env, &wasi_data);
  w2c_pthread__test_0x5Fstart(&m);
  printf("Done\n");
  wasm2c_pthread__test_free(&m);

  minwasi_cleanup_instance(&wasi_data);

  wasm_rt_free();

  return 0;
}
