var MAINTAB_MODULE = (function() {
  'use strict';

  var self = {};

  self.createTab = function(payload, element) {
    this.tabName = function() {
      return "Index";
    };
    $.getJSON('/json/systems/contents/', function(data) {
      var el = $('<table>').appendTo(element);
      el.deeptable({data:data.data, schema:data.schema});
    });

    return this;
  };

  return self;
})();