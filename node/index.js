const cloak = require('./build/Release/cloak.node');

var version = cloak.getVersion();
console.log('version = ' + version);

var filelength = cloak.getFilelength('../LICENSE');
console.log('File length of input file is ' + filelength);

cloak.hide('../Lenna.png', '../LICENSE', '~/temp/Lenna_out.png', 1, 'PNG', 'AES-256', 'password');
cloak.reveal('~/temp/Lenna_out.png', '/Users/guy/temp/LICENSE.txt', 1, 'AES-256', false, 'password');

module.exports = cloak;
