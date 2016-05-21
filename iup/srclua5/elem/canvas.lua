------------------------------------------------------------------------------
-- Canvas class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "canvas",
  parent = iup.WIDGET,
  subdir = "elem",
  creation = "-",
  callback = {
    action = "ff",
    button_cb = "nnnns",
    enterwindow_cb = "",
    leavewindow_cb = "",
    motion_cb = "nns",
    resize_cb = "nn",
    scroll_cb = "nff",
    keypress_cb = "nn",
    wom_cb = "n",
    wheel_cb = "fnns",
    mdiactivate_cb = "",
    touch_cb = "nnns",
    focus_cb = "n",
  }
}

function ctrl.createElement(class, param)
   return iup.Canvas()
end

function ctrl.DrawBegin(ih)
  iup.DrawBegin(ih)
end

function ctrl.DrawEnd(ih)
  iup.DrawEnd(ih)
end

function ctrl.DrawParentBackground(ih)
  iup.DrawParentBackground(ih)
end

function ctrl.DrawLine(ih, x1, y1, x2, y2)
  iup.DrawLine(ih, x1, y1, x2, y2)
end

function ctrl.DrawRectangle(ih, x1, y1, x2, y2)
  iup.DrawRectangle(ih, x1, y1, x2, y2)
end

function ctrl.DrawArc(ih, x1, y1, x2, y2, a1, a2)
  iup.DrawArc(ih, x1, y1, x2, y2, a1, a2)
end

function ctrl.DrawPolygon(ih, points)
  iup.DrawPolygon(ih, points)
end

function ctrl.DrawText(ih, text, x, y)
  iup.DrawText(ih, text, x, y)
end

function ctrl.DrawImage(ih, name, make_inactive, x, y)
  iup.DrawImage(ih, name, make_inactive, x, y)
end

function ctrl.DrawSetClipRect(ih, x1, y1, x2, y2)
  iup.DrawSetClipRect(ih, x1, y1, x2, y2)
end

function ctrl.DrawResetClip(ih)
  iup.DrawResetClip(ih)
end

function ctrl.DrawSelectRect(ih, x, y, w, h)
  iup.DrawSelectRect(ih, x, y, w, h)
end

function ctrl.DrawFocusRect(ih, x, y, w, h)
  iup.DrawFocusRect(ih, x, y, w, h)
end

function ctrl.DrawGetSize(ih)
  return iup.DrawGetSize(ih)
end

function ctrl.DrawGetTextSize(ih, str)
  return iup.DrawGetTextSize(ih, str)
end

function ctrl.DrawGetImageInfo(ih, name)
  return iup.DrawGetImageInfo(name)
end


iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iupWidget")
