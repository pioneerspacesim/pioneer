var MAINTAB_MODULE = (function() {
  'use strict';

  var self = {};

  self.createTab = function(payload, element) {
    this.tabName = function() {
      return "Index";
    };
    return this;
  };

  return self;
})();