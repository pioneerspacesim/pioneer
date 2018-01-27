var SYSTEMTAB_MODULE = (function() {
  'use strict';

  var self = {};

  self.createTab = function(parent, entry) {
    this.tabName = entry.name;
    var el = $('<div>').appendTo(parent);

    function ResetTabContents() {
      el.empty();
      $.getJSON('/json/systems/get/', {
        'file': entry.filename,
        'system': entry.name
      }, function(data) {

        var buttonBar = $('<div>').addClass('button-bar').appendTo(el);
        $('<button>').text('Save').addClass('enable-on-change')
            .attr('disabled', 'disabled').appendTo(buttonBar)
            .click(function(e) {
              $.ajax({
                url: '/json/systems/store/',
                type: 'POST',
                data: {
                  id: JSON.stringify(entry),
                  data: JSON.stringify(data.data)
                },
                cache: false,
                dataType: 'json'
              }).done(function() {
                entry.filename = data.data.system.filename;
                ResetTabContents();
              }).fail(function(xhr, textstatus) {
                $(e.target).text("Error: " + textstatus);
              });
            });

        $('<button>').text('Reset').addClass('enable-on-change')
            .attr('disabled', 'disabled').appendTo(buttonBar).click(function() {
              if (confirm("Reset all fields and lose changes?")) {
                ResetTabContents();
              }
            });

        if (entry.filename && entry.system != 'New System') {
          $('<button>').text('Delete').appendTo(buttonBar).click(function() {
            if (confirm("Delete this system?")) {
              $.ajax({
                url: '/json/systems/delete/',
                type: 'POST',
                data: {
                  id: JSON.stringify(entry),
                },
                cache: false,
                dataType: 'json'
              }).done(function() {
                el.empty();
                el.text('This system has been deleted');
              }).fail(function(xhr, textstatus) {
                $(e.target).text("Error: " + textstatus);
              });

            }
          });
        }


        var deepTable = $('<div>').appendTo(el)
        deepTable.deeptable({
          data: [data.data],
          schema: data.schema,
        });
      });
    }

    ResetTabContents();

    return this;
  }
  return self;
})();
