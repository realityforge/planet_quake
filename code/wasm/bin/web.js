const express = require('express');

var app = express()

app.use(express.static(__dirname));
app.use(express.static(__dirname + '/../../../build/debug-js-js'));

app.listen(8080, ()=> {
  console.log(`Server is running on 8080` )
})
