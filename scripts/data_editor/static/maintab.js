var MAINTAB_MODULE = (function() {
  'use strict';

  var self = {};

  self.createTab = function(element) {
    this.tabName = "Index";
    $.getJSON('/json/systems/contents/', function(data) {
      var buttonBar = $('<div>').addClass('button-bar').appendTo(element);
      var newSystem = $('<button>')
        .text('Create new system').appendTo(buttonBar);

      newSystem.click(function() {
        PIONEER_MODULE.newTab('system', {name: 'New System', file: ''});
      })

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
