this.vertices = {
	/**
	 * @return {Object} {x,y,width,height}
	 */
	getClipRectangle: () => ({
		x: m.get_working_area_x()>>>0,
		y: m.get_working_area_y()>>>0,
		width: m.get_working_area_width()>>>0,
		height: m.get_working_area_height()>>>0
	}),

	/**
	 * Builds rect shape which fully covers the current layer
	 * @return {Boolean}
	 */
	buildDefault: () => onErr(m.vertices_build_default()),

	/**
	 * Prepares to build the vertices
	 * @return {Boolean}
	 */
	prepareBuild: () => 1===m.vertices_prepare_build(),

	/**
	 * @param {Number} x
	 * @param {Number} y
	 * @return {Boolean}
	 */
	add: (x, y) => 1===m.vertices_add(x|0, y|0),

	/**
	 * Finishes building of the vertices
	 * @return {Boolean}
	 */
	finishBuild: () => onErr(m.vertices_finish_build()),

	/**
	 * @param {Number} x offset
	 * @param {Number} y offset
	 * @return {Boolean}
	 */
	move: (x, y) => onErr(m.vertices_move(x|0, y|0)),

	/**
	 * @return {Number}
	 */
	getMaxCount: () => m.vertices_get_max_count(),

	/**
	 * @return {Number}
	 */
	getCount: () => m.vertices_get_count(),

	/**
	 * Warn: Provides a raw view onto the internal vertices buffer
	 * @return {Int16Array} list of x,y sequences
	 */
	getRaw: () => new Int16Array(m.memory.buffer, m.vertices_get(), m.vertices_get_count()<<1)
};
