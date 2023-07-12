/*
export WASI_SDK_DIR=/opt/bin/wasi-sdk

$WASI_SDK_DIR/bin/clang --sysroot $WASI_SDK_DIR/share/wasi-sysroot \
    --target=wasm32-wasi-threads -pthread \
    -Wl,--import-memory,--export-memory,--max-memory=67108864 \
    pthread_test.c -o pthread_test.wasm

/home/shr/Code/LibrarySandboxing/wabt/build/wasm2c pthread_test.wasm -o pthread_test.wasm.c --enable-threads

*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define NUM_THREADS 10

void *thread_entry_point(void *ctx) {
  int id = (int) ctx;
  printf(" in thread %d\n", id);
  return 0;
}

int main(int argc, char **argv) {
  pthread_t threads[10];
  for (int i = 0; i < NUM_THREADS; i++) {
    int ret = pthread_create(&threads[i], NULL, &thread_entry_point, (void *) i);
    if (ret) {
      printf("failed to spawn thread: %s", strerror(ret));
    }
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  return 0;
}
