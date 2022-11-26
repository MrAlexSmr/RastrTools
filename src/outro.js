}

/**
 * @param {Number} r 0..255
 * @param {Number} g 0..255
 * @param {Number} b 0..255
 * @param {Number} a 0..255
 * @return {Number} 32bit LE number (color representation)
 */
RastrTools.rgba = function(r, g, b, a) {
	return ((r&255)|((g&255)<<8)|((b&255)<<16)|((a&255)<<24))>>>0;
};

/**
 * @param {Number} clr 0..0xFFFFFFFF
 * @return {Number} [0..255] Red channel value
 */
RastrTools.getR = function(clr) {
	return (clr>>>0)&255;
};

/**
 * @param {Number} clr 0..0xFFFFFFFF
 * @return {Number} [0..255] Green channel value
 */
RastrTools.getG = function(clr) {
	return (clr>>>8)&255;
};

/**
 * @param {Number} clr 0..0xFFFFFFFF
 * @return {Number} [0..255] Blue channel value
 */
RastrTools.getB = function(clr) {
	return (clr>>>16)&255;
};

/**
 * @param {Number} clr 0..0xFFFFFFFF
 * @return {Number} [0..255] Alpha channel value
 */
RastrTools.getA = function(clr) {
	return (clr>>>24)&255;
};

/**
 * Custom XMLHttpRequest "get" wrapper.
 * @param {String} url
 * @param {String} type - response type
 * @param {Function} cb - callback without an argument is success indicator
 */
RastrTools.fetch = function(url, type, cb, user, pswrd) {
	var r = new XMLHttpRequest();
	r.open("GET", url, true, user, pswrd);
	r.onload = () => cb(r.response);
	r.onerror = () => cb(new Error("Unable to fetch the module"));
	r.ontimeout = () => cb(new Error("Request timed out"));
	r.timeout = 10000; // 10 seconds, just magic number, should be enough to fetch things
	r.responseType = type;
	r.send(null);
};
