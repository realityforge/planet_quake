const express = require('express');
const fs = require('fs');
const serveIndex = require('serve-index');

const app = express()

app.use(express.static(__dirname + '/../http/'));
app.use(express.static(__dirname + '/../../../build/debug-js-js/'));

// layer alternatives because WASM loads QVM
app.use('/multigame/vm/', serveIndex(__dirname + '/../../../build/release-js-js/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(__dirname + '/../../../build/debug-js-js/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(__dirname + '/../../../build/release-darwin-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(__dirname + '/../../../build/debug-darwin-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(__dirname + '/../../../build/release-linux-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(__dirname + '/../../../build/debug-linux-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(__dirname + '/../../../build/release-win-x86_64/multigame/vm/'));
app.use('/multigame/vm/', serveIndex(__dirname + '/../../../build/debug-win-x86_64/multigame/vm/'));


app.use('/multigame/vm/', express.static(__dirname + '/../../../build/release-js-js/multigame/vm/'));
app.use('/multigame/vm/', express.static(__dirname + '/../../../build/debug-js-js/multigame/vm/'));

app.use('/multigame/vm/', express.static(__dirname + '/../../../build/release-darwin-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(__dirname + '/../../../build/debug-darwin-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(__dirname + '/../../../build/release-linux-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(__dirname + '/../../../build/debug-linux-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(__dirname + '/../../../build/release-win-x86_64/multigame/vm/'));
app.use('/multigame/vm/', express.static(__dirname + '/../../../build/debug-win-x86_64/multigame/vm/'));



app.use('/multigame/', serveIndex(__dirname + '/../../../games/multigame/assets/'));
app.use('/multigame/', express.static(__dirname + '/../../../games/multigame/assets/'));



app.listen(8080, ()=> {
  console.log(`Server is running on 8080` )
})
