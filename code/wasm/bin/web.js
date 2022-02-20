const express = require('express');
var serveIndex = require('serve-index');

var app = express()

app.use(express.static(__dirname + '/../http/'));
app.use(express.static(__dirname + '/../../../build/debug-js-js/'));
app.use('/multigame/vm/', express.static(__dirname + '/../../../build/debug-js-js/multigame/vm/'));
app.use('/multigame/', express.static(__dirname + '/../../../games/multigame/assets/'));
app.use('/multigame/vm/', serveIndex(__dirname + '/../../../build/debug-js-js/multigame/vm/'));
app.use('/multigame/', serveIndex(__dirname + '/../../../games/multigame/assets/'));

app.listen(8080, ()=> {
  console.log(`Server is running on 8080` )
})
