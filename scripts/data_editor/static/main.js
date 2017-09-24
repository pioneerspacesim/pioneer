function BaseXVarintEncoder() {
  var dict = '0123456789abcdefghijklmnopqrstuvwxyz' +
      'ABCDEFGHIJKLMNOPQRSTUVWXYZ~-_.!*\'(),$';
  var res = '';
  var headSpace = Math.floor(dict.length / 2);
  var trunkSpace = dict.length - headSpace;

  function addCodePoint(x) {
    res += dict[x];
  }

  this.value = function() {
    return res;
  };

  this.addInteger = function(x) {
    while (x >= headSpace) {
      x -= headSpace;
      addCodePoint(headSpace + x % trunkSpace);
      x = Math.floor(x / trunkSpace);
    }
    addCodePoint(x);
  };

  this.addString = function(x) {
    this.addInteger(x.length);
    for (var i = 0; i < x.length; ++i) this.addInteger(x.charCodeAt(i));
  };

  this.addSet = function(x) {
    var y = x;
    y.sort(function(a, b) {
      return a - b;
    });
    this.addInteger(y.length);
    for (var i = 0; i < y.length; ++i) {
      if (i === 0)
        this.addInteger(y[0]);
      else if (y[i] != y[i - 1])
        this.addInteger(y[i] - y[i - 1] - 1);
    }
  };

  this.addFlags = function(x) {
    var val = 0;
    x.forEach(function(y) {
      val |= 1 << y;
    });
    this.addInteger(val);
  };
  this.addBool = function(x) {
    this.addInteger(x ? 1 : 0);
  };

  this.addHeader = function(typ, val) {
    this.addInteger(val * 16 + typ);
  };
}

var PIONEER_MODULE = (function() {
  'use strict';

  var self = {};
  var tabs = [];
  var tab_counter = 0;

  self.tabDispatcher = function(id, payload, element) {
    switch(id) {
      case 0:
        return new MAINTAB_MODULE.createTab(payload, element);
      default:
        $(element).text("Unknown tab type");
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
    var newTabContents = self.tabDispatcher(id, payload, newElement);
    newTab.find('a').text(newTabContents.tabName());
    newTab.find('span').click(function(){
      newTab.remove();
      newElement.remove();
      el.tabs("refresh");
    });
    el.tabs("refresh");
  };

  self.freshTab = function() {
    self.newTab(0, "");
    $('#tabs').tabs("option", "active", -1);
  }

  self.init = function() {
    $('#tabs').tabs();
    $('#new-tab-button').click(self.freshTab);
    self.freshTab();
  };

  return self;
})();