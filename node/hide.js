const cloak = require('./build/Release/cloak.node');

cloak.hide('../flowers.png', '../LICENSE', '/Users/guy/temp/flowers_out.png', 1, 'PNG', 'AES-256', 'password');

module.exports = cloak;
