const cloak = require('./build/Release/cloak.node');

cloak.reveal('/Users/guy/temp/flowers_out.png', '/Users/guy/temp/LICENSE.txt', 1, 'AES-256', false, 'password');

module.exports = cloak;
