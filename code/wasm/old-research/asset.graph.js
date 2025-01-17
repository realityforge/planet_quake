function DirectedGraph() {
	this._vertices = {};
	this._edges = {};
}

DirectedGraph.prototype.getVertices = function () {
	var self = this;
	return Object.keys(this._vertices).map(function (key) { return self._vertices[key]; });
};

DirectedGraph.prototype.getVertex = function (id) {
	return this._vertices[id]
};

DirectedGraph.prototype.addVertex = function (id, data) {
	var v = this._vertices[id] = new Vertex(id);

	if (data) {
		Object.keys(data).forEach(function (key) {
			v.data[key] = data[key];
		});
	}

	return v;
};

DirectedGraph.prototype.addEdge = function (a, b) {
	var id = a.id + '-' + b.id;

	var e = this._edges[id] = new Edge(id, a, b);

	a.outEdges.push(e);
	b.inEdges.push(e);

	return e;
};

DirectedGraph.prototype.removeEdge = function (e) {
	var outIdx = e.outVertex.outEdges.indexOf(e);
	if (outIdx === -1) {
		throw new Error('edge not found on out vertex');
	}

	var inIdx = e.inVertex.inEdges.indexOf(e);
	if (inIdx === -1) {
		throw new Error('edge not found on in vertex');
	}

	e.outVertex.outEdges.splice(outIdx, 1);
	e.inVertex.inEdges.splice(inIdx, 1);

	delete this._edges[e.id];
};

function Vertex(id) {
	this.id = id;
	this.data = {};
	this.inEdges = [];
	this.outEdges = [];
}

Vertex.prototype.getOutVertices = function () {
	return this.outEdges.map(function (inE) {
		return inE.inVertex;
	});
};

Vertex.prototype.getInVertices = function () {
	return this.inEdges.map(function (inE) {
		return inE.outVertex;
	});
};

function getLeaves(v, depth) {
  var result = Array.isArray(v) ? v : [v]
  do {
    level = result
      .map(v => v.outEdges.map(e => e.inVertex))
      .flat(1)
      .filter((v) => !result.includes(v))
    result.push.apply(result, level)
  } while(!depth && depth !== 0 && level.length > 0 || --depth > 0)
  return result.map(v => v.id)
		.filter((f, i, arr) => arr.indexOf(f) === i)
}

Vertex.prototype.getLeaves = function (depth) {
	return getLeaves(this, depth)
}

function Edge(id, outVertex, inVertex) {
	this.id = id;
	this.outVertex = outVertex;
	this.inVertex = inVertex;
}

DirectedGraph.prototype.getLeaves = getLeaves

module.exports = {
	DirectedGraph,
	getLeaves
}
