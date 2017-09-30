var SYSTEMTAB_MODULE = (function() {
  'use strict';

  var self = {};

  self.createTab = function(element, entry) {
    this.tabName = entry.name;

    $.getJSON('/json/systems/get/', {
      'file': entry.filename,
      'system': entry.name
    }, function(data) {
      var el = $('<div>').appendTo(element);
      el.deeptable({
        data: data.data,
        schema: data.schema,
      });
    });

    return this;
  }
  return self;
})();
