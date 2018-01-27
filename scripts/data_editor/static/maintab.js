var MAINTAB_MODULE = (function() {
  'use strict';

  var self = {};

  self.createTab = function(element) {
    this.tabName = "Index";
    $.getJSON('/json/systems/contents/', function(data) {
      var el = $('<div>').appendTo(element);
      el.deeptable({
        data: [data.data],
        schema: data.schema,
        onClick: function(x) { PIONEER_MODULE.newTab('system', x); }
      });
    });

    return this;
  };

  return self;
})();
