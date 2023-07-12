WASI_SDK_DIR=/opt/wasi-sdk
WABT=/home/shr/Code/LibrarySandboxing/wabt/
PTHREAD_FLAGS=-g -O0 -I $(WABT)/wasm2c

build: pthread

pthread_test.wasm: pthread_test.c
	$(WASI_SDK_DIR)/bin/clang --sysroot $(WASI_SDK_DIR)/share/wasi-sysroot \
		--target=wasm32-wasi-threads -pthread \
		-Wl,--import-memory,--export-memory,--max-memory=2147483648 \
		pthread_test.c -o $@

pthread_test.wasm.c: pthread_test.wasm
	$(WABT)/build/wasm2c $^ -o $@ --enable-threads

pthread_test.wasm.o: pthread_test.wasm.c
	clang $(PTHREAD_FLAGS) -c $^ -o $@

pthread_loader.o: pthread_loader.c
	clang $(PTHREAD_FLAGS) -c $^ -o $@

wasm2c_rt_minwasi.o: wasm2c_rt_minwasi.c
	clang $(PTHREAD_FLAGS) -c $^ -o $@

wasm-rt-impl.o: $(WABT)/wasm2c/wasm-rt-impl.c
	clang $(PTHREAD_FLAGS) -c $^ -o $@

wasm-rt-threads-impl.o: $(WABT)/wasm2c/wasm-rt-threads-impl.cpp
	clang++ -std=c++20 $(PTHREAD_FLAGS) -c $^ -o $@

pthread: pthread_test.wasm.o pthread_loader.o wasm2c_rt_minwasi.o wasm-rt-impl.o wasm-rt-threads-impl.o
	clang++ -std=c++20 $(PTHREAD_FLAGS) $^ -o $@ -lpthread

clean:
	rm -f pthread_test.wasm pthread_test.wasm.c pthread_test.wasm.o pthread_loader.o wasm2c_rt_minwasi.o wasm-rt-impl.o wasm-rt-threads-impl.o