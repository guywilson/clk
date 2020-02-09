const cloak = require('./build/Release/cloak.node');

var filelength = cloak.getFilelength('../LICENSE');

console.log('File length of input file is ' + filelength);

cloak.hide('../flowers.png', '../LICENSE', '/Users/guy/temp/flowers_out.png', 1, 'PNG', 'AES-256', 'password');

module.exports = cloak;
