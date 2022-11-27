# 'make'				build binarry file
# 'make clean'			removes .wasm and binarry files
# 'make disassemble'	makes a "human-readable" .wat file from .wasm

COMPILER = clang

SOURCES = $(wildcard *.c)

# 4096x4096x4 or 1024x65536 or 64*1048576 (64Mb)
# ----------- or 512x65536 or 32*1048576 (32Mb)
TOTAL_MEMORY=67108864

# Pretty enough to execute few funcs at same time;
# (all recursions should be simulated using custom "stack" algo);
# Warn: probably, data section will take same amount of stack mem,
# so in reality data+stack will eat 2048:
TOTAL_STACK=1024

CUSTOM_FLAGS = \
	-D TOTAL_BYTES=${TOTAL_MEMORY} \
	-D WASM32=1 \
	-D NDEBUG=1 \
	-D LZW_MAX_BITS=14 \

# https://lld.llvm.org/WebAssembly.html
# https://github.com/llvm/llvm-project/blob/master/clang/lib/Headers/wasm_simd128.h
COMPILER_FLAGS = \
	--target=wasm32-unknown-unknown-wasm \
	-Wall \
	-Werror \
	-Wno-strict-prototypes \
	-Wno-incompatible-library-redeclaration \
	-Wno-gnu-union-cast \
	-Oz \
	-flto \
	-std=c11 \
	-nostdlib \
	-nostartfiles \
	-fvisibility=hidden \
	-pedantic-errors \
#	-msimd128 \ <--- consider this (test closely)
#	-fdata-sections \
#	-ffunction-sections \
#	-mbulk-memory \

LINKER_FLAGS = \
	-Wl,--lto-O3 \
	-Wl,-error-limit=0 \
	-Wl,--export=__heap_base \
	-Wl,--export-dynamic \
	-Wl,-z,stack-size=$(TOTAL_STACK) \
	-Wl,--initial-memory=$(TOTAL_MEMORY) \
	-Wl,--max-memory=$(TOTAL_MEMORY) \
	-Wl,--allow-undefined \
	-Wl,--no-entry \
#	-Wl,--no-gc-sections \
#	-Wl,--no-merge-data-segments \
#	-Wl,--shared-memory,--no-check-features \
#	-Wl,--passive-segments \


build:
	$(COMPILER) $(CUSTOM_FLAGS) $(COMPILER_FLAGS) $(LINKER_FLAGS) $(SOURCES) -o module.wasm

disassemble:
	wasm2wat module.wasm -o module.wat

clean: ; rm *.wasm *.wat

.PHONY: clean
