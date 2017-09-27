var PIONEER_MODULE = (function() {
  'use strict';

  var self = {};
  var tabs = [];
  var tab_counter = 0;

  self.tabDispatcher = function(element, id, payload) {
    switch (id) {
      case 'index':
        return new MAINTAB_MODULE.createTab(element);
      case 'system':
        return new SYSTEMTAB_MODULE.createTab(element, payload);
      default:
        $(element).text("Unknown tab type: " + id);
        return { tabName: "Error" };
    }
  }

  self.newTab = function(id, payload) {
    var tab_idx = ++tab_counter;
    var newTab = $('<li><a href="#tab-' + tab_idx +
      '"></a><span class="ui-icon ui-icon-close">Remove Tab</span></li>');
    var newElement = $('<div id="tab-' + tab_idx + '"></div>');

    var el = $('#tabs');
    el.find('.ui-tabs-nav').append(newTab);
    el.append(newElement);
    var newTabContents = self.tabDispatcher(newElement, id, payload);
    newTab.find('a').text(newTabContents.tabName);
    newTab.find('span').click(function() {
      newTab.remove();
      newElement.remove();
      el.tabs("refresh");
    });
    el.tabs("refresh");
    $('#tabs').tabs("option", "active", -1);
  };

  self.freshTab = function() {
    self.newTab('index');
  }

  self.init = function() {
    $('#tabs').tabs();
    $('#new-tab-button').click(self.freshTab);
    self.freshTab();
  };

  return self;
})();
