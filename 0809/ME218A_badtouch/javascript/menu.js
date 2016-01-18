/* menu.js - Originally Created by Ruddick "Vinny" Lawrence
 * -------------------------------------------------------------
 * Controls dropdown menus. Modified from script at http://javascript-array.com/scripts/simple_drop_down_menu/. Requires Prototype.
 */

var menu = {
    init: function() {
	menu.timeout = 300;
	menu.close_timer = null;
	menu.menuitem = null;

	Event.observe(document, 'click', menu.close_menu);
	
        var headers = $('menu').select('.menuheader');
        for(var i = 0; i < headers.length; i++) {
	    Event.observe(headers[i], 'mouseover', menu.open_menu);
	    Event.observe(headers[i], 'mouseout', menu.start_close_timer);
	    var menulists = headers[i].parentNode.select('.menulist');
	    if(menulists.length != 0) {
		var menulist = menulists[0];
		Event.observe(menulist, 'mouseover', menu.cancel_close_timer);
		Event.observe(menulist, 'mouseout', menu.start_close_timer);
	    }
        }
    },

    open_menu: function(e) {
	var header = Event.element(e);
	if(header.tagName != "LI") header = header.parentNode;

	// cancel close timer
	menu.cancel_close_timer();
	// close old menu
	if(menu.menuitem) menu.close_menu();

	// show new menu
	var selects = header.select('div.menulist')
	if(selects == 0) return;
	menu.menuitem = selects[0];
	menu.menuitem.setStyle({visibility: 'visible'});
    },

    close_menu: function() {
	if(menu.menuitem) menu.menuitem.setStyle({visibility: 'hidden'});
	menu.menuitem = null;
	menu.close_timer = null;
    },

    start_close_timer: function() {
	menu.close_timer = window.setTimeout(menu.close_menu, menu.timeout);
    },

    cancel_close_timer: function() {
	if(menu.close_timer != null) {
	    window.clearTimeout(menu.close_timer);
	    menu.close_timer = null;
	}
    }
};

Event.observe(window, 'load', menu.init);
