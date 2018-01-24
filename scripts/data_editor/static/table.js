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
  Array2D.prototype.IsUndefinedAbove = function(row, col) {
    for (var i = 0; i <= row; ++i)
      if (this.Get(i, col) !== undefined) return false;
    return true;
  }

  function FormatWithSiPrefix(val) {
    if (val == 0) return "0";
    var digits = "";
    if (val < 0) {
      digits = "-";
      val = val;
    }

    var exponent = Math.floor(Math.log10(val));
    var mantissa = Math.round(val / (10 ** (exponent - 2))).toString();
    var suffix = '';

    var prefixes = [
      'y', 'z', 'a', 'f', 'p', 'n', 'µ', 'm', '',
      'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y',
    ];

    if (exponent == -1) {
      // Special case, "0.1" appears more readable to me than "100m".
      digits += "0." + mantissa;
    } else {
      var x = exponent % 3;
      if (x < 0) x += 3;
      digits += mantissa.slice(0, x + 1) + '.' + mantissa.slice(x + 1);
      suffix = prefixes[Math.floor(exponent / 3) + 8];
    }

    return digits.replace(/\.?0*$/, '') + suffix;
  }

  function FormatToDecimal(val) {
    return val.toFixed(1).replace(/\.?0*$/, '');
  }

  var formatters = {
    'int': function(el, val) {
      el.addClass('number').text(val);
    },
    'text': function(el, val) {
      el.addClass('text').text(val);
    },
    'siprefix': function(el, val) {
      el.text(FormatWithSiPrefix(val));
    },
    'percent': function(el, val) {
      el.text(FormatToDecimal(val * 100) + '%');
    },
    'degrees': function(el, val) {
      el.text(FormatToDecimal(val) + '°');
    },
    'hex': function(el, val) {
      el.addClass('number').text(val.toString(16));
    },
    'default': function(el, val) {
      el.text(val);
    },
    'none': function(el, val) {
      el.text('(none)').addClass('empty');
    }
  }

  function StdWidget(parent, label) {    
    var el = $("<input></input>").appendTo(parent).focus();
    if (label) {
      parent.append(document.createTextNode(label));
    }
    return el;
  }

  var widgets = {
    'enum': function(parent, val, scheme) {
      var el = $('<select>').appendTo(parent).focus();
      $('<option>').val('').text('(none)').appendTo(el);
      scheme.enumvals.forEach(function(v) {
        $('<option>').val(v).text(v).appendTo(el);
      });
      el.val(val);
      return function() { return el.val() || null; };
    },
    'text': function(parent, val, scheme) {
      var el = $('<textarea>').text(val).appendTo(parent).focus();
      return function() { return el.val() || null; };
    },
    'int': function(parent, val, scheme) {
      var el = StdWidget(parent, scheme.editsuffix).val(val);
      return function() { return el.val() ? parseInt(el.val()) : null; };
    },
    'hex': function(parent, val, scheme) {
      var el = StdWidget(parent, scheme.editsuffix).val(val);
      return function() { return el.val() ? parseInt(el.val()) : null; };
    },
    'percent': function(parent, val, scheme) {
      var el = StdWidget(
          parent, " × 100% " + (scheme.editsuffix || '')).val(val);
      return function() { return el.val() ? parseFloat(el.val()) : null; };
    },
    'degrees': function(parent, val, scheme) {
      var el = StdWidget(parent, "°" + (scheme.editsuffix || '')).val(val);
      return function() { return el.val() ? parseFloat(el.val()) : null; };
    },
    'default': function(parent, val, scheme) {
      var el = StdWidget(parent, scheme.editsuffix).val(val);
      return function() { return el.val() || null; };
    }
  };

  function CreateWidget(parent, scheme, dict, key) {
    var widget = $('<div class="widget-container">').appendTo(parent);
    if (scheme.editlabel) {
      $('<div class="widget-label">').text(scheme.editlabel).appendTo(widget);      
    }

    var fmt = scheme.format;
    if (!widgets.hasOwnProperty(fmt)) fmt = 'default';
    return widgets[fmt](widget, dict[key], scheme);
  }

  function Format(fmt, el, val) {
    el.empty();
    el.removeClass();
    if (val === null) fmt = 'none';
    else if (!formatters.hasOwnProperty(fmt)) fmt = 'default';
    formatters[fmt](el, val);
  }

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

  function RenderHeader(schema, title, options) {
    options = options || {};
    var cells = new Array2D();
    var curRow = 0;
    var curCol = 0;
    if (options.collapsible) {
      cells.Set(0, 0, '');
      ++curCol;
    }

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
        while (j + 1 < cells.Cols() && cells.IsUndefinedAbove(i, j + 1)) ++j;
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

  function EditField(e, scheme, td, dict, key) {
    td.addClass('editing');
    var el = $('<div class="editbox"></div>').appendTo('body');
    var boundRect = td[0].getBoundingClientRect();
    el.css({
      'position': 'absolute',
      'top': boundRect.bottom + document.documentElement.scrollTop,
      'left': boundRect.left + document.documentElement.scrollLeft
    });
    var header = $('<div class="editbox-header">')
      .text(scheme.title).appendTo(el);

    var getter = CreateWidget(el, scheme, dict, key);

    var okButton = $('<button>OK</button>').click(Ok).appendTo(el);
    var cancelButton = $('<button>Cancel</button>').click(Cancel).appendTo(el);

    function Cancel(e) {
      td.removeClass('editing');
      el.remove();
      document.removeEventListener('click', OnClickOutside, true);      
      document.removeEventListener('keydown', OnKey, true);      
    }

    function Ok(e) {
      dict[key] = getter();
      Format(scheme.format, td, dict[key]);
      td.addClass('modified-value');
      Cancel();
    }

    function OnClickOutside(e) {
      if (el[0].contains(e.target)) return;
      e.stopPropagation();
    }
    function OnKey(e) {
      if (e.keyCode == 27) Cancel();
      if (e.keyCode == 13) Ok();
    }

    e.stopPropagation();
    document.addEventListener('click', OnClickOutside, true);      
    document.addEventListener('keydown', OnKey, true);      
  }

  function RenderDataChunk(schema, item, options) {
    options = options || {};
    var cells = new Array2D();
    var curCol = 0;
    var curRow = 0;
    var xs = [item];

    var element = $('<tbody>');
    if (options.collapsible) {
      var td = $('<td>');
      cells.Set(0, 0, td);
      element.collapser = td;
      ++curCol;
    }

    IterateSchema(
      schema,
      function(item) {
        var td = $('<td>');
        Format(item.format, td, xs[xs.length - 1][item.id]);
        if (options.editable) {
          (function(scheme, dict, key) {
            td.click(function(e) {
              EditField(e, scheme, td, dict, key);
            });
          })(item, xs[xs.length - 1], item.id);
        }
        cells.Set(curRow, curCol, td);
        ++curCol;
      },
      function(item) {
        xs.push(xs[xs.length - 1][item.id]);
      },
      function(item) {
        xs.pop();
      }
    );
    for (var i = 0; i < cells.Rows(); ++i) {
      var tr = $('<tr>');
      for (var j = 0; j < cells.Cols(); ++j) {
        if (cells.Get(i, j) === undefined) continue;
        var td = cells.Get(i, j);
        var col = j;
        var rowspan = 1;
        while (i + rowspan < cells.Rows() &&
          cells.Get(i + rowspan, j) === undefined) {
          ++rowspan;
        }
        while (j + 1 < cells.Cols() && cells.Get(i, j + 1) === undefined) ++j;
        var colspan = j - col + 1;
        td.attr('colspan', colspan)
          .attr('rowspan', rowspan)
          .appendTo(tr);
      }
      tr.appendTo(element);
    }
    element.colspanwidth = cells.Cols();
    return element;
  }

  function RenderFields(schema, data, title, options) {
    options = options || {};
    var el = $('<table>').addClass('deeptable');
    $('<th>').attr('colspan', 2).addClass('deeptable-title')
      .text(title).appendTo($('<tr>').appendTo(el));

    for (var i = 0; i < schema.length; ++i) {
      var tbody = $('<tbody>').addClass(i % 2 ? 'even' : 'odd').appendTo(el);
      var tr = $('<tr>').appendTo(tbody);
      $('<th>').text(schema[i].title).appendTo(tr);
      var td = $('<td>');
      if (schema[i].subfields) {
        var table = $('<table>').addClass('deeptable').appendTo(td);
        RenderHeader(schema[i].subfields).appendTo(table);
        RenderDataChunk(schema[i].subfields, data[schema[i].id], options)
          .addClass('odd')
          .appendTo(table);
      } else {
        Format(schema[i].format, td, data[schema[i].id]);
        if (options.editable) {
          (function(td, scheme, dict, key) {
            td.click(function(e) {
              EditField(e, scheme, td, dict, key);
            });
          })(td, schema[i], data, schema[i].id);
        }
      }
      td.appendTo(tr);
    }

    return el;
  }

  $.widget('ui.deeptable', {
    defaultElement: '<div>',
    options: {
      schema: [],
      data: [],
      onClick: undefined,
    },
    _create: function() {
      var schema = this.options.schema;
      for (var i = 0; i < schema.order.length; ++i) {
        if (this.options.data[schema.order[i]]) {
          this.renderChunk(this.element, {},
            schema,
            schema.order[i],
            this.options.data[schema.order[i]]);
        }
      }
    },
    renderChunk: function(parent, parent_options, root_schema, section, data) {
      var schema = root_schema[section];
      var options = schema.options || {};
      options = $.extend({}, parent_options, options);
      if (schema.type == 'table') {
        var el = $('<table>').addClass('deeptable').appendTo(parent);
        RenderHeader(schema.columns, schema.title, options).appendTo(el);
        for (var i = 0; i < data.length; ++i) {
          var row = RenderDataChunk(schema.columns, data[i], options);
          row.addClass(i % 2 ? 'even' : 'odd').appendTo(el);
          if (schema.subsections) {
            var innerTbody = $('<tbody>').addClass('inner');
            var hasSomething = false;
            for (var j = 0; j < schema.subsections.length; ++j) {
              var subsection = schema.subsections[j];
              var d = data[i][subsection.id];
              if (!d || d.length == 0) continue;
              hasSomething = true;
              var td = $('<td>')
                .addClass('subsection')
                .attr('colspan', row.colspanwidth)
                .appendTo($('<tr>').appendTo(innerTbody));
              this.renderChunk(td, options, root_schema, subsection.schema, d);
            }
            if (hasSomething) {
              innerTbody.appendTo(el);
              if (row.collapser) {
                (function(collapser, tbody) {
                  tbody.hide();
                  var hidden = true;
                  collapser.text('⊞').addClass('clickable').click(function() {
                    if (hidden) {
                      tbody.show('fast');
                      collapser.text('⊟');
                      hidden = false;
                    } else {
                      tbody.hide();
                      collapser.text('⊞');
                      hidden = true;
                    }
                  });
                })(row.collapser, innerTbody);
              }
            }
          }
          if (options.clickable && this.options.onClick) {
            row.addClass('clickable');
            row.click(this.options.onClick.bind(this, data[i]));
          }
        }
      } else if (schema.type == 'fields') {
        RenderFields(schema.fields, data, schema.title, options)
          .appendTo(parent);
      } else {
        $('<div>').text('Uknown schema type! ' + schema.type).appendTo(parent);
      }
    }
  });

})(jQuery);
