// express app

const express = require('express');
const app = express();

const api = require('./api');

app.use(express.json());

app.use('/api/v1', api);

app.use(express.static('public'));

app.get('/', (req, res) => {
  res.sendFile(__dirname + '/public/index.html');
});

app.listen(3000, () => {
  console.log('server is running on port 3000');
});