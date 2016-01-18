/* utils.js - Originally created by Ruddick "Vinny" Lawrence
 * ------------------------------------------------------------------
 * Some utility functions that other javascripts might use. Probably
 * requires Prototype. Might require Scriptaculous.
 */

var utils = {
    /* Changes a CSS style given as a string, in px, into a float.
     * For example: utils.px2num("83.9px") returns 83.9.
     */
    px2num: function(str) {
	return parseFloat(str.replace('px',''));
    },

    /* Places and resizes the passed in elem. The corner to place
     * is specified as a two letter string (accepted values are tl,
     * tr, bl, br). Input -1 as xpos and/or ypos in order to center
     * elem in that direction. Input -1 as maxwidth and/or maxheight
     * to make viewport dimensions the maxes. Input 0 to mean no
     * maxes. Function is quirky in that it assumes that the coords
     * (0,0) are the top left corner of a centered div, even when
     * the elem's position is set to absolute (seems to happen when
     * containing div's position is explicitly set to relative).
     * This is because function was created for website that used
     * such a container div. May be some other quirks that make it
     * specific to that page.
     */
    popup_position: function(elem, corner, xpos, ypos, maxwidth, maxheight) {
	elem.setStyle({position: 'absolute', top: 0, left: 0});

	var elem_dims = elem.getDimensions();
	var parent_dims = elem.parentNode.getDimensions();
	var parent_margs = new Array();
	parent_margs['l'] = utils.px2num(elem.parentNode.getStyle('marginLeft'));
	parent_margs['t'] = utils.px2num(elem.parentNode.getStyle('marginTop'));
	var win_dims = document.viewport.getDimensions();
	var win_offsets = document.viewport.getScrollOffsets();
	var padding = new Array();
	padding['l'] = utils.px2num(elem.getStyle('paddingLeft'));
	padding['r'] = utils.px2num(elem.getStyle('paddingRight'));
	padding['t'] = utils.px2num(elem.getStyle('paddingTop'));
	padding['b'] = utils.px2num(elem.getStyle('paddingBottom'));
	var x = 0;
	var y = 0;

	if(maxwidth == -1) maxwidth = win_dims.width; //-1 means viewport width is maxwidth
	if(maxwidth != 0 && elem_dims.width > maxwidth)
	    elem.setStyle({width: (maxwidth - padding['l'] - padding['r']) + 'px'}); //subtract padding pixels because they add to CSS width

	if(maxheight == -1) maxheight = win_dims.height; //-1 means viewport height is maxheight
	if(maxheight != 0 && elem_dims.height > maxheight)
	    elem.setStyle({height: (maxheight - padding['t'] - padding['b']) + 'px'}) ;//subtract padding pixels because they add to CSS height

	elem_dims = elem.getDimensions();

	if(xpos == -1) { //-1 means center elem
	    if(win_offsets[0] == 0 && win_dims.width > parent_dims.width)
		x = parent_margs['l'] + (parent_dims.width - elem_dims.width)/2;
	    else
		x = win_offsets[0] + (win_dims.width - elem_dims.width)/2;
	}
	else {
	    if(corner[1] == 'l')
		x = xpos;
	    else
		x = xpos - elem_dims.width;
	}

	if(ypos == -1) //-1 means center elem
	    y = win_offsets[1] + (win_dims.height - elem_dims.height)/2;
	else{
	    if(corner[0] == 't')
		y = ypos;
	    else
		y = ypos - elem_dims.height;
	}

	elem.setStyle({left: x+'px', top: y+'px'});
    },

    /* Randomly rearranges a passed in array using the Fisher-Yates
     * shuffle. Returns the shuffled array.
     */
    array_shuffle: function(arr) {
	var n = arr.length;
	while(n > 1) {
	    var k = Math.floor(Math.random()*(n+1));
	    n--;
	    var temp = arr[n];
	    arr[n] = arr[k];
	    arr[k] = temp;
	}
    }
};
