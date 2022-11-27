# RastrTools (v0.2.1-beta)

Self-sufficient repo containing source code of the drawing library.

## About

JavaScript low-level drawing library. Works on top of WebAssembly module. All operations are applied into the internal self-updated ImageData.

	Designed for:
		- Easy to use. Should provide simple but powerful features in combination;
		- Work with non-antialiased images (pixel-art);
		- Drawing shapes and lines;
		- Color blending and filtering (standart browser compositioning and filtering operations);
		- Layering and tiling;
		- Undo/redo operations.

	Features that makes it fast and lite:
		- Module allocates all required memory on upstart and keeps it till the end;
		- Possibility to use 32Mb or 64Mb modules;
		- All complex logic (except handling external images) is done inside of the module;
		- Does not require external libs. At all.

	Since it should be fast and not consuming too much memory, it has few limitations:
		- Max layers count is 12;
		- Max size of a layer is limited to 1024x1024 pixels (For single-layer mode the max size is 2484x2484);
		- Up to 100 history iteration items;
		- Limited vertices count.

## Building

In case there is a need to make changes for the js wrapper file (also, it is required for local server binarry):

```sh
./run.sh
```

In case there is a need to make changes in wasm module:

```sh
./run.sh build
```

## Feedback

For almost any questions write me at smorchkov.oleksandr@gmail.com
