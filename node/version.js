const cloak = require('./build/Release/cloak.node');

var version = cloak.getVersion();

console.log('version = ' + version);

module.exports = cloak;
