build/tfs.wasm build/tfs.js build/tfs.data: bridge.c
	emcc -o build/tfs.js bridge.c --preload-file FluidR3_GM.sf2 -s ALLOW_MEMORY_GROWTH=1

.PHONY: wasm
wasm: build/tfs.wasm build/tfs.js build/tfs.data

.PHONY: clean
clean:
	rm -rf build/*
