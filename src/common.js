var m, maxSide, lyrSize, lyrsCnt, onErr;

// calls an error callback (if defined) in case if error code param is non zero:
onErr = code => (code > 0
	? (void(this.onError && this.onError(code)), false)
	: true);

/**
 * User should define the handler to get error notifications
 * @property onError
 * @type {Function}
 */
this.onError = null;

/**
 * @param {Boolean} [useSingleLayer] - will force to use single layer only with max possible layer size
 */
this.reset = function(useSingleLayer) {
	m.mem_internal_init(useSingleLayer);
	maxSide = m.layers_get_max_side_size();
	lyrsCnt = m.layers_get_max_layers_count();
	lyrSize = maxSide * maxSide * 4;
};

/**
 * Loads respective wasm module (makes it primary) and inits the memory.
 * @param {String} url - path to the specific wasm module
 * @param {Function} cb - fired back with error object in case of error or null if everything is OK
 */
this.reload = function(url, cb, user, pswrd) {
	// instantiateStreaming is good,
	// but not supported by all vendors and has caveats
	RastrTools.fetch(url, "arraybuffer", b => {
		if (WebAssembly.validate(b)) {
			WebAssembly.instantiate(b, { env: {} }).then(res => {
				// TODO: before unloading old module, copy all data of old module into the tmp buffer,
				// Then unload old module and emplace copied data right into the correct place of the new module

				m = res.instance.exports;
				cb(null); // everything is good
			}).catch(cb);
		} else {
			cb(new Error("Module is invalid"));
		}
	}, user, pswrd);
};
