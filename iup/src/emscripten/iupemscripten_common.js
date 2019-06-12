

var LibraryIupCommon = {
//	$Common__deps: ['$CommonGlobals'],
	$IupCommonGlobals: {
		idCounter: 1,
		objectIDMap: {},
	},

	$IupCommon: {


		GetObjectForID: function(handle_id) {
			return IupCommonGlobals.objectIDMap[handle_id];
		},
		SetObjectForID: function(the_object, handle_id) {
			IupCommonGlobals.objectIDMap[handle_id] = the_object;
		},
		RegisterNewObject: function(the_object) {
			var current_id = IupCommonGlobals.idCounter;
			IupCommonGlobals.objectIDMap[current_id] = the_object;
			the_object.handleID = current_id;
			IupCommonGlobals.idCounter += 1;
			return current_id;
		},
    InitializeObject: function(the_object) {
      // any other useful properties for all objects should go here
      the_object.style.position = 'absolute';
    },
		DeleteObject: function(handle_id) {
			var the_object = IupCommonGlobals.objectIDMap[handle_id] = null;
			the_object.handleID = null;
			IupCommonGlobals.objectIDMap[handle_id] = null;
			the_object = null;
		},
	},

  emjsCommon_SetPosition: function(handle_id, x, y, width, height)
  {

    // var real_elem = document.getElementById(handle_id);

		var elem = IupCommon.GetObjectForID(handle_id);

    // elem.style.color = 'white';
    elem.style.left = x + 'px';
    console.log(elem.nodeName);
    console.log("left: " + elem.style.left);
    elem.style.top = y + 'px';
    console.log("top: " + elem.style.top);

    // elem.style.width =
    console.log("width: " + width);
    console.log("height: " + height);
    console.log("actual width: " + elem.style.width);
    console.log("actual height: " + elem.style.height);
    // if (width != 0 && height != 0) {
      elem.style.width = width + 'px';
      elem.style.height = height + 'px';
    // }
    console.log("actual width after: " + elem.style.width);
    console.log("actual height after: " + elem.style.height);

  },

	emjsCommon_AddWidgetToDialog: function(parent_id, child_id) {
    console.log("AddWidgetToDialog");
		var parent_dialog = IupCommon.GetObjectForID(parent_id);
		var parent_body = parent_dialog.document.getElementsByTagName("body")[0];
		var child_widget = IupCommon.GetObjectForID(child_id);
		parent_body.appendChild(child_widget);
	},

  // <div> -- inject code -- </div>
  emjsCommon_AddCompoundToDialog: function(parent_id, elem_array, num_elems) {
    var parent_dialog = IupCommon.GetObjectForID(parent_id);
    var parent_body = parent_dialog.document.getElementsByTagName("body")[0];
    var child_widget;

    for (var i = 0; i < num_elems; i++) {
      var child_id = {{{ makeGetValue('elem_array', 'i*4', 'i32') }}};
      child_widget = IupCommon.GetObjectForID(child_id);
      parent_body.appendChild(child_widget);
    }
  },

  emjsCommon_AddCompoundToWidget: function(parent_id, elem_array, num_elems) {
		var parent_widget = IupCommon.GetObjectForID(parent_id);
    var child_widget;

    for (var i = 0; i < num_elems; i++) {
      var child_id = {{{ makeGetValue('elem_array', 'i*4', 'i32') }}};
      child_widget = IupCommon.GetObjectForID(child_id);
      parent_body.appendChild(child_widget);
    }
  },

	emjsCommon_AddWidgetToWidget: function(parent_id, child_id) {
    console.log("AddWidgetToWidget");
		var parent_widget = IupCommon.GetObjectForID(parent_id);
		var child_widget = IupCommon.GetObjectForID(child_id);

		parent_widget.appendChild(child_widget);
	},

  emjsCommon_Alert: function(message) {
    console.log("alert message: ");
    console.log(message);
    alert(Pointer_stringify(message));
  },

  emjsCommon_Log: function(message) {
//    console.log("our log is working");
      console.log(Pointer_stringify(message));
  },

	emjsCommon_IupLog: function(priority, message)
	{
		var stringified_message = Pointer_stringify(message);
		/*	 
		enum IUPLOG_LEVEL
		{
			IUPLOG_LEVEL_LOG = 0,
			IUPLOG_LEVEL_DEBUG,
			IUPLOG_LEVEL_INFO,
			IUPLOG_LEVEL_WARN,
			IUPLOG_LEVEL_ERROR
		};
		*/

		switch(priority)
		{
			case 1:
				console.debug(stringified_message);
				break; 
			case 2:
				console.info(stringified_message);
				break;
			case 3:
				console.warn(stringified_message);
				break;
			case 4:
				console.error(stringified_message);
				break;
			default: 
				console.log(stringified_message);
		}
	},




  emjsCommon_SetFgColor: function(handle_id, r, g, b)
  {
    var current_widget = IupCommon.GetObjectForID(handle_id);
    console.log(current_widget);
    current_widget.style.color = "rgb(" + r + "," + g + "," + b + ")";
  },

  iupEmscriptenSetFgColor: function(handle_id, r, g, b) {
	  var sel_object = IupCommon.GetObjectForID(handle_id);
    sel_object.style.color = "rgb(" + r + "," + g + "," + b + ")";
  }

};

autoAddDeps(LibraryIupCommon, '$IupCommonGlobals');
autoAddDeps(LibraryIupCommon, '$IupCommon');
mergeInto(LibraryManager.library, LibraryIupCommon);
