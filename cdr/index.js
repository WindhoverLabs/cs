'use strict';

var path = require('path');

const CdrPlugin = require(path.join(global.CDR_INSTALL_DIR, '/commander/classes/CdrPlugin')).CdrPlugin;

module.exports = class CfeCdrPlugin extends CdrPlugin {
  constructor(urlBase) {
    super('cs', path.join(__dirname, 'web', urlBase));
  }

  getContent() {
    var result = {
      shortDescription: 'Checksum Services',
      longDescription: 'Checksum Services',
      nodes: {
				main: {
					type: CdrPlugin.ContentType.LAYOUT,
					shortDescription: 'Main',
					longDescription: 'Main AMC.',
					filePath: '/main_layout.lyt'
				},
				hk: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'CS Housekeeping',
					longDescription: 'Checksum Services Housekeeping',
					filePath: '/hk.pug'
				}
      }
    };

    return result;
  }
};

//var plugin = new CfeCdrPlugin();

//let CfeCdrPlugin = new class extends CdrPlugin {
