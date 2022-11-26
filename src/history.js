var cursor = 0;
var items = [];

var push = item => {
	if (items.length >= 100) items.shift().free();
	return (cursor = items.push(item));
};

var on_size_swap = err => this.history.onSizeSwap && this.history.onSizeSwap(err);

var on_layer_swap = i => this.history.onLayerSwap && this.history.onLayerSwap(i>>>0);

var on_custom_swap = (type, data) => this.history.onCustomSwap && this.history.onCustomSwap(type, data);

var swap = i => onErr(-1 === i ? 0 : items[i].swap());

this.history = {
	/**
	 * @property onLayerSwap
	 * @type {Function}
	 */
	onLayerSwap: null,

	/**
	 * @property onSizeSwap
	 * @type {Function}
	 */
	onSizeSwap: null,

	/**
	 * Will be called with previosly saved data.
	 * Result of this function will be saved in history.
	 *
	 * @property onCustomSwap
	 * @type {Function}
	 */
	onCustomSwap: null,

	/**
	 * @param {Number} lyr address
	 * @return {Boolean}
	 */
	saveLayer: lyr => {
		var item = new Layer(lyr);
		return item.save() && !!push(item);
	},

	/**
	 * @param {Array} lyrs list of layer addresses to save
	 * @param {Number} width
	 * @param {Number} height
	 * @return {Boolean}
	 */
	saveSize: (lyrs, width, height) => {
		var i, item;
		i = 0;
		item = new Size(width, height);
		while (i < lyrs.length)
			if (!item.save(new Layer(lyrs[i++]))) {
				item.free(); // clear memory for all already saved layers
				return false; // and let "external logic" to decide what to do
			}
		return !!push(item);
	},

	/**
	 * @param {Any} type indicator
	 * @param {Any} data
	 * @return {Boolean}
	 */
	saveCustom: (type, data) => {
		var item = new Custom(type);
		item.save(data);
		return !!push(item);
	},

	/**
	 * Triggers respective swap callback
	 * May trigger error callback (if occured, the swap callback will not be called)
	 * @return {Boolean}
	 */
	undo: () => swap(cursor > 0 ? --cursor : -1),

	/**
	 * Triggers respective swap callback
	 * May trigger error callback (if occured, the swap callback will not be called)
	 * @return {Boolean}
	 */
	redo: () => swap(cursor < items.length ? cursor++ : -1),

	/**
	 * @param {Number} count of items to drop
	 */
	drop: cnt => {
		var i = cnt|0;
		if (i < 0) {
			i = -i;
			while (i-- && items.length) items.pop().free();
		} else {
			while (i-- && items.length) items.shift().free();
		}
		if (cursor > items.length) cursor = items.length;
	},

	/**
	 * @return {Number} number of currently stored items
	 */
	getItemsCount: () => items.length>>>0,

	/**
	 * @return {Number} number of current history item
	 */
	getCursor: () => cursor>>>0,

	/**
	 * Call it before any other history operation to get max possible bytes count
	 * @return {Number} the less it returns, the less free space for undo is
	 */
	getFreeBytes: () => m.get_free_bytes()>>>0,

	/**
	 * @return {Number} max possible length of strings to save
	 */
	getMaxStrLength: () => 255
};


// 
function Layer(addr) {
	this.page = 0>>>0;
	this.layer = addr>>>0;
};

Layer.prototype._saveLayer = function(flag) {
	var addr = m.layers_get_extra()>>>0;
	return m.history_save(
		addr,
		m.compression_compress(
			addr, addr + lyrSize,
			this.layer, flag|0))>>>0;
};

Layer.prototype.save = function(compressWholeLayer) {
	return !!(this.page = this._saveLayer(compressWholeLayer));
};

Layer.prototype.swap = function(width, height) {
	var err, oldW, oldH, page, hasWH;

	hasWH = arguments.length > 1;
	page = this._saveLayer(hasWH); // if got width and height, then ignore clip_indexes

	if (0 === page) {
		return 0;
	}
	if (hasWH) {
		oldW = m.layers_get_width();
		oldH = m.layers_get_height();
		m.layers_resize(width, height);
	}
	err = m.compression_decompress(
		m.get_history_data_at(this.page), this.layer);

	if (0 !== err) {
		// Got error, free occupied new item's data
		Layer.prototype.free.call({ page: page>>>0 });
		return err;
	} else if (hasWH) {
		m.layers_resize(oldW>>>0, oldH>>>0);
	}

	// No error, free old item's data
	this.free();
	// and put new one on its place
	this.page = page>>>0;

	on_layer_swap(this.layer);

	return 0;
};

Layer.prototype.free = function() {
	m.history_mark_pages(this.page, 0>>>0);
};


function Size(width, height) {
	this.width = width>>>0;
	this.height = height>>>0;
	this.layerItems = [];
};

Size.prototype.save = function(layerItem) {
	if (layerItem.save(true)) {
		this.layerItems.push(layerItem);
		return true;
	}
	return false;
};

Size.prototype.swap = function() {
	let i = 0;
	let err = 0;
	while (i < this.layerItems.length) {
		err = this.layerItems[i++].swap(this.width, this.height);
		if (0 !== err) {
			// do no try to revert, this may end up even worse
			void(onErr(err));
			on_size_swap(err);
		}
	}

	const width = m.layers_get_width();
	const height = m.layers_get_height();

	m.layers_resize(this.width, this.height);
	m.vertices_build_default();

	this.width = width;
	this.height = height;

	// everything is ok here,
	// but if not, this callback was already been called above ^
	if (0 === err) on_size_swap(err);
	return err;
};

Size.prototype.free = function() {
	while (this.layerItems.length) this.layerItems.pop().free();
};


function Custom(type) {
	this.type = type;
	this.free();
}

Custom.prototype.save = function(data) {
	this.data = data;
};

Custom.prototype.swap = function() {
	this.data = on_custom_swap(this.type, this.data);
	return 0;
};

Custom.prototype.free = function() {
	this.data = null;
};
