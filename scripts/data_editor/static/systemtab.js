var SYSTEMTAB_MODULE = (function() {
  'use strict';

  var self = {};

  self.createTab = function(element, entry) {
    this.tabName = entry.name;
    return this;
  }
  return self;
})();