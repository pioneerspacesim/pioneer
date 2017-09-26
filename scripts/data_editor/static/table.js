(function($) {
  'use strict';

  // Array which resizes itself.
  function Array2D() {
    this.data = [];
    this.cols = 0;
  }
  Array2D.prototype.Set = function(row, col, val) {
    while (this.data.length <= row) this.data.push([]);
    while (this.data[row].length <= col) this.data[row].push(undefined);
    this.data[row][col] = val;
    if (this.cols < col + 1) this.cols = col + 1;
  };
  Array2D.prototype.Get = function(row, col) {
    var r = this.data[row];
    if (col < r.length) return r[col];
    return undefined;
  };
  Array2D.prototype.Rows = function() {
    return this.data.length;
  };
  Array2D.prototype.Cols = function() {
    return this.cols;
  };

  function IterateSchema(schema, opt_terminal, opt_msg_begin, opt_msg_end) {
    for (var i = 0; i < schema.length; ++i) {
      var item = schema[i];
      if (item.hasOwnProperty('subfields')) {
        if (opt_msg_begin) opt_msg_begin(item);
        IterateSchema(
          item['subfields'], opt_terminal, opt_msg_begin, opt_msg_end);
        if (opt_msg_end) opt_msg_end(item);
      } else {
        if (opt_terminal) opt_terminal(item);
      }
    }
  }

  function RenderHeader(schema, title) {
    var cells = new Array2D();
    var curRow = 0;
    var curCol = 0;

    IterateSchema(
      schema,
      function(item) {
        cells.Set(curRow, curCol, item.title);
        ++curCol;
      },
      function(item) {
        cells.Set(curRow, curCol, item.title);
        ++curRow;
      },
      function(item) {
        --curRow;
      });

    var element = $('<thead>');
    if (title) {
      var tr = $('<tr>').appendTo(element);
      $('<th>').attr('colspan', cells.Cols()).addClass('deeptable-title')
        .text(title).appendTo(tr);
    }
    for (var i = 0; i < cells.Rows(); ++i) {
      var tr = $('<tr>');
      for (var j = 0; j < cells.Cols(); ++j) {
        if (cells.Get(i, j) === undefined) continue;
        var text = cells.Get(i, j);
        var col = j;
        var rowspan = 1;
        while (i + rowspan < cells.Rows() &&
          cells.Get(i + rowspan, j) === undefined) {
          ++rowspan;
        }
        while (j + 1 < cells.Cols() && cells.Get(i, j + 1) === undefined) ++j;
        var colspan = j - col + 1;
        var th = $('<th>')
          .attr('colspan', colspan)
          .attr('rowspan', rowspan)
          .text(text)
          .appendTo(tr);
      }
      tr.appendTo(element);
    }
    return element;
  }

  $.widget('ui.deeptable', {
    defaultElement: '<table>',
    options: {
      title: 'Table',
      schema: [],
      data: [],
    },
    _create: function() {
      this.element.addClass('deeptable');
      RenderHeader(this.options.schema, this.options.title)
        .appendTo(this.element);
    },
  });

})(jQuery);
