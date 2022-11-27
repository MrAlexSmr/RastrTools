var at = (x, y, w) => (x + y * w)|0;

var lyrBytes = addr => new Uint8ClampedArray(m.memory.buffer, addr, m.layers_get_size()<<2);

this.layers = {
	/**
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} ch r===0, g===1, b===2, a===3
	 * @param {Number} val
	 */
	channelMove: (lyr, ch, val) => m.channel_move(lyr>>>0, ch|0, val|0),

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} ch r===0, g===1, b===2, a===3
	 * @param {Number} val
	 */
	channelSet: (lyr, ch, val) => m.channel_set(lyr>>>0, ch|0, val>>>0),

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} ch r===0, g===1, b===2, a===3
	 * @param {Number} val
	 */
	channelClampUp: (lyr, ch, val) => m.channel_clamp_up(lyr>>>0, ch|0, val>>>0),

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} ch r===0, g===1, b===2, a===3
	 * @param {Number} val
	 */
	channelClampDown: (lyr, ch, val) => m.channel_clamp_down(lyr>>>0, ch|0, val>>>0),

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @return {Number} colors count of the given layer
	 */
	colorGetCount: lyr => m.colors_get_count(lyr>>>0)>>>0,

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @return {Uint32Array} list of all unique colors of the given layer
	 */
	colorGetAll: lyr => new Uint32Array(
		// relies on fact, that colors_get_count stores unique colors found in the extra memory:
		lyrBytes(m.layers_get_extra()).slice(0, m.colors_get_count(lyr>>>0)<<2).buffer
	),

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} x
	 * @param {Number} y
	 * @param {Number} badColor will be returned back in case of error
	 * @return {Number} found color from position or badColor in case of bad position
	 */
	colorGetAt: (lyr, x, y, badColor) => m.color_get_at(
		lyr>>>0, x>>>0, y>>>0, badColor>>>0)>>>0,

	/**
	 * Force-fills destination layer by given color
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} clr
	 */
	colorClear: (lyr, clr) => m.clear(lyr>>>0, clr>>>0),

	/**
	 * Replaces "dest" colors of destination layer by "src"
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} dest color
	 * @param {Number} src color
	 * @return {Boolean}
	 */
	colorReplace: (lyr, dest, src) => 1===m.replace(lyr>>>0, dest>>>0, src>>>0),

	/**
	 * Copies colors from given source index into the given layer
	 * @param {Number} dest u32 address on the layer
	 * @param {Number} src u32 address on the layer
	 * @return {Boolean}
	 */
	copyLayer: (dest, src) => 1===m.copy(dest>>>0, src>>>0, 0),

	/**
	 * Moves pixels of destination layer be given offset
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} x
	 * @param {Number} y
	 * @param {Number} frame_x - limit by x
	 * @param {Number} frame_y - limit by y
	 * @param {Number} frame_w - limit by width
	 * @param {Number} frame_h - limit by height
	 * @param {Boolean} clear whether algo should clear data in old place
	 * @param {Number} clrColor - clear color
	 */
	moveClip: (lyr, x, y, frame_x, frame_y, frame_w, frame_h, clear, clrColor) => m.move(
		lyr>>>0,
		x|0, y|0,
		frame_x>>>0, frame_y>>>0, frame_w>>>0, frame_h>>>0,
		clear|0, clrColor>>>0),

	/**
	 * Flood-fills destination layer by given color starting at x:y position
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} clr
	 * @param {Number} x
	 * @param {Number} y
	 * @return {Boolean}
	 */
	floodFill: (lyr, clr, x, y) => 1===m.fill(lyr>>>0, x>>>0, y>>>0, clr>>>0),

	/**
	 * Flips pixels of destination layer.
	 * Warn: Make sure vertices are rectangle-like, or result will be unexpected
	 * @param {Number} lyr u32 address on the layer
	 */
	flip: lyr => m.flip(lyr>>>0),

	/**
	 * Mirrors pixels of destination layer
	 * Warn: Make sure vertices are rectangle-like, or result will be unexpected
	 * @param {Number} lyr u32 address on the layer
	 */
	mirror: lyr => m.mirror(lyr>>>0),

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @return {ImageData} view of given layer image
	 */
	getImageData: lyr => new ImageData(lyrBytes(lyr>>>0), m.layers_get_width(), m.layers_get_height()),

	/**
	 * The given imageData will be filled by the pixels from x:y - imageData.width:imageData.height rect
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} x
	 * @param {Number} y
	 * @param {ImageData} imageData
	 * @return {Boolean} whether the imageData and offset are correct
	 */
	copyToImageData: (lyr, x, y, imageData) => {
		var i, j, lw, lh, bits;
		lw = m.layers_get_width();
		lh = m.layers_get_height();
		if (0 === x && 0 === y &&
			lw === imageData.width && lh === imageData.height) {
			imageData.data.set(lyrBytes(lyr>>>0), 0);
			return true;
		} else if (x >= 0 && y >= 0 &&
			x + imageData.width <= lw && y + imageData.height <= lh) {
			j = 0;
			i = at(x, y, lw);
			bits = lyrBytes(lyr>>>0);
			y = 0;
			while (y++ < imageData.height) {
				imgData.data.set(bits.subarray(i << 2, (i + imageData.width) << 2), j << 2);
				i += lw;
				j += imageData.width;
			}
			return true;
		}
		return false;
	},

	/**
	 * Copies imageData into the given layer
	 * @param {Number} lyr u32 address on the layer
	 * @param {ImageData} imageData
	 * @param {Number} x
	 * @param {Number} y
	 * @param {Boolean} force into the given layer ignoring clip and compose
	 */
	putImageData: (lyr, imageData, x, y, force) => {
		var i, j, x_, y_, addr, bits, extra, imgWidth, imgHeight, lyrWidth, lyrHeight, offsetX, offsetY;
		x_ = x|0;
		y_ = y|0;
		addr = lyr>>>0;
		extra = m.layers_get_extra();
		imgWidth = imageData.width;
		imgHeight = imageData.height;
		bits = force ? lyrBytes(addr) : lyrBytes(extra);
		lyrWidth = m.layers_get_width();
		lyrHeight = m.layers_get_height();
		if (0 === x_ && 0 === y_ && imgWidth === lyrWidth && imgHeight === lyrHeight) {
			bits.set(imageData.data);
		} else {
			if (x_ >= 0 && y_ >= 0 &&
				lyrWidth >= (x_ + imgWidth) && lyrHeight >= (y_ + imgHeight)) {
				j = 0;

				// May be necessary to clear extra layer
				// to clean it up from previos artifacts,
				// especially if the layer has been saved in history before putting image data:
				m.clear_in_range(extra, extra + lyrWidth * lyrHeight, 0);
			} else {
				offsetX = offsetY = 0;
				if ((x + imgWidth) > lyrWidth) {
					imgWidth = Math.abs(lyrWidth - x);
				}
				if ((y + imgHeight) > lyrHeight) {
					imgHeight = Math.abs(lyrHeight - y);
				}
				if (x < 0) {
					offsetX = -x;
					imgWidth -= offsetX;
					x = 0;
				}
				if (y < 0) {
					offsetY = -y;
					imgHeight -= offsetY;
					y = 0;
				}
				j = at(offsetX, offsetY, imageData.width);
			}
			i = at(x, y, lyrWidth);
			y_ = 0;
			while (y_++ < imgHeight) {
				bits.set(imageData.data.subarray(j << 2, (j + imgWidth) << 2), i << 2);
				i += lyrWidth;
				j += imageData.width;
			}
		}
		if (!force) {
			m.copy(addr, extra, 0);
		}
	},

	/**
	 * Changes the size of the given layers
	 * (Clip vertices will be ignored and reset to default)
	 * @param {Array} lyrs list of u32 layer addresses
	 * @param {Number} offsetX
	 * @param {Number} offsetY
	 * @param {Number} width
	 * @param {Number} height
	 * @return {Boolean} whether the operation compleetes successfuly
	 */
	resizeAll: (lyrs, offsetX, offsetY, width, height) => {
		var i, lyr, oldW, oldH, extra, oldSize;
		oldW = m.layers_get_width();
		oldH = m.layers_get_height();
		if (oldW === width && oldH === height) return false;
		m.layers_resize(width>>>0, height>>>0);
		if (Array.isArray(lyrs) && lyrs.length <= lyrsCnt) {
			i = lyr = 0>>>0;
			extra = m.layers_get_extra();
			oldSize = (oldW * oldH)<<2;
			while (i < lyrs.length) {
				lyr = lyrs[i++]>>>0;
				m.copy_in_range(extra, extra + oldSize, lyr);
				m.clear_in_range(lyr, lyr + oldSize, 0);
				m.put_image(lyr, extra, offsetX|0, offsetY|0, oldW>>>0, oldH>>>0);
			}
		}
		return onErr(m.vertices_build_default());
	},

	/**
	 * @return {Number}
	 */
	getWidth: () => m.layers_get_width()>>>0,

	/**
	 * @return {Number}
	 */
	getHeight: () => m.layers_get_height()>>>0,

	/**
	 * @return {Number} max allowed side size
	 */
	getMaxSideSize: () => maxSide>>>0,

	/**
	 * @return {Number} min allowed side size
	 */
	getMinSideSize: () => 1>>>0,

	/**
	 * https://developer.mozilla.org/en-US/docs/Web/CSS/filter
	 * Implements some of those (link above) native filters
	 * Allowed "op":
	 *  1===FILTER_BRIGHTNESS
	 *  2===FILTER_CONTRAST
	 *  3===FILTER_INVERT
	 *  4===FILTER_SEPIA
	 *  5===FILTER_SATURATE
	 *  6===FILTER_GRAYSCALE
	 *  7===FILTER_HUE_ROTATE
	 *  8===FILTER_GAMMA
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} op
	 * @param {Number} v amount in [0..1]
	 */
	filter: (lyr, op, v) => m.filter(lyr>>>0, op|0, +v),

	/**
	 * R = v0,  v1,  v2,  0,  0,
	 * G = v5,  v6,  v7,  0,  0,
	 * B = v10, v11, v12, 0,  0,
	 * A = 0,   0,   0,   1,  0,
	 *
	 * Filter custom matrix
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} v0 [0..1]
	 * @param {Number} v1 [0..1]
	 * @param {Number} v2 [0..1]
	 * @param {Number} v5 [0..1]
	 * @param {Number} v6 [0..1]
	 * @param {Number} v7 [0..1]
	 * @param {Number} v10 [0..1]
	 * @param {Number} v11 [0..1]
	 * @param {Number} v12 [0..1]
	 */
	filterCustom: (
		lyr,
		v0, v1, v2,
		v5, v6, v7,
		v10, v11, v12) => m.filter_custom_matrix(
			lyr>>>0,
			+v0, +v1, +v2,
			+v5, +v6, +v7,
			+v10, +v11, +v12),

	/**
	 * https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/globalCompositeOperation
	 * Implements some of those (link above) native composition operations
	 * Allowed "op":
	 *   0===COPY
	 *   1===SOURCE_OVER
	 *   2===SOURCE_IN
	 *   3===SOURCE_OUT
	 *   4===SOURCE_ATOP
	 *   5===DESTINATION_OVER
	 *   6===DESTINATION_IN
	 *   7===DESTINATION_OUT
	 *   8===DESTINATION_ATOP
	 *   9===LIGHTER
	 *  10===XOR
	 *  11===MULTIPLY
	 *  12===SCREEN
	 *  13===OVERLAY
	 *  14===DARKEN
	 *  15===LIGHTEN
	 *  16===COLOR_DODGE
	 *  17===COLOR_BURN
	 *  18===HARD_LIGHT
	 *  19===SOFT_LIGHT
	 *  20===DIFFERENCE
	 *  21===EXCLUSION
	 *
	 * Not implemented yet:
	 *  22===HUE
	 *  23===SATURATION
	 *  24===COLOR
	 *  25===LUMINOSITY
	 * @param {Number} op
	 */
	globalCompositeOperationSet: op => m.compose_set_operation(op|0),

	/**
	 * @return {Number} enum integer representation
	 */
	globalCompositeOperationGet: () => m.compose_get_operation()|0,

	/**
	 * Call one before any stroke operation (or sequence) of operations
	 */
	strokePrepare: () => m.stroke_prepare(),

	/**
	 * Call one after stroke operation (or operations) are done
	 */
	strokeFinish: () => m.stroke_finish(),

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} x0
	 * @param {Number} y0
	 * @param {Number} x1
	 * @param {Number} y1
	 * @param {Number} w line width
	 * @param {Number} clr
	 */
	strokeLine: (lyr, x0, y0, x1, y1, w, clr) => m.stroke_line(lyr>>>0, x0|0, y0|0, x1|0, y1|0, w>>>0, clr>>>0),

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} x0
	 * @param {Number} y0
	 * @param {Number} x1
	 * @param {Number} y1
	 * @param {Number} clr
	 */
	strokeRectangle: (lyr, x0, y0, x1, y1, clr) => m.stroke_rectangle(lyr>>>0, x0|0, y0|0, x1|0, y1|0, clr>>>0),

	/**
	 * @param {Number} lyr u32 address on the layer
	 * @param {Number} x0
	 * @param {Number} y0
	 * @param {Number} x1
	 * @param {Number} y1
	 * @param {Number} clr
	 */
	strokeEllipse: (lyr, x0, y0, x1, y1, clr) => m.stroke_ellipse(lyr>>>0, x0|0, y0|0, x1|0, y1|0, clr>>>0),

	/**
	 * @param {Number} ind index of layer between 0 and getMaxCount
	 * @return {Number} layer's addres (0 in case of wrong index)
	 */
	getAddr: ind => (ind >= 0 || ind < lyrsCnt ? m.layers_get() + lyrSize * ind : 0)>>>0,

	/**
	 * @return {Number} max possible layers count. Depends on useSingleLayer flag in "reset" method 
	 */
	getMaxCount: () => lyrsCnt>>>0
};
