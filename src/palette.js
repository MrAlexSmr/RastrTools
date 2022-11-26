this.palette = {
	/**
	 * Reduces colors of the given layer using simple Euclidean distance
	 * @param {Number} lyr u32 address on the layer
	 * @param {Boolean} [skipAlpha]
	 */
	apply: (lyr, skipAlpha) => m.palette_apply(lyr>>>0, skipAlpha|0),

	/**
	 * Resize palette to the new value. Limitted by max size.
	 * @see getMaxSize
	 * @param {Number} size
	 */
	resize: size => m.palette_resize(size>>>0),

	/**
	 * @return {Number} size of the palette
	 */
	getSize: () => m.palette_get_size(),

	/**
	 * @return {Number} max possible size of the palette
	 */
	getMaxSize: () => m.palette_get_max_size(),

	/**
	 * @param {Number} clr
	 * @param {Boolean} [skipAlpha] - flag to skip alpha channel from calculations
	 * @return {Number} most nearest color inside of the palette to the given
	 */
	findNearestColor: (clr, skipAlpha) => m.palette_find_nearest_color(clr>>>0, skipAlpha|0),

	/**
	 * Warn: Provides a raw view onto the internal palette buffer
	 * @return {Uint32Array} list of u32 color values
	 */
	getRaw: () => new Uint32Array(m.memory.buffer, m.palette_get(), m.palette_get_size()<<2)
};
