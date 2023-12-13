const fs = require('fs');
const { exec } = require('child_process');


const filePath = './data';

/**
 * get data
 * 
 * get data from "../data"
 * 
 * data has just 3 lines
 * 
 * 1. power
 * 2. rotation
 * 3. timer
 * 
 * @param {string} type
 * @return {string} data of type
 */

function getData(type) {
  const types = ['power', 'rotation', 'timer'];

  

  const data = fs.readFileSync(filePath, 'utf8');
  const lines = data.split('\n');
  if (type == 'powerAuto') {
   return lines[0] == '17' ? 1 : 0; 
  }
  if (type == 'rotateAuto') {
    return lines[1] == '17' ? 1 : 0;
  }

  const index = types.indexOf(type);

  return lines[index] == '17' ? 0 : lines[index];
}

/**
 * set data
 * 
 * set data to "../data"
 * 
 * data has just 3 lines
 * 
 * 1. power
 * 2. rotation
 * 3. timer
 */

function setData(type, value) {
  const types = ['power', 'rotation', 'timer']

  const data = fs.readFileSync(filePath, 'utf8');
  const lines = data.split('\n');
  const index = types.indexOf(type);
  lines[index] = value;
  const newData = lines.join('\n');
  fs.writeFileSync(filePath, newData);
}


function runProcess(type, mode) {
  exec(`sudo ./web_control ${type}${mode}`, (err, stdout, stderr) => {
    if (err) {
      console.log(err);
      return;
    }
    console.log(stdout);
  });
}

module.exports = {
  getData,
  setData,
  runProcess
};