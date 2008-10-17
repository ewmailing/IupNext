local MainDialog = nil;

local function Hide(dlg)
  if dlg then
    dlg.placement = "MINIMIZED";
    dlg:show();

    dlg.TrayMenuToggle.title = "Show";
    dlg.hidetaskbar = "YES";
  end
end

local function Show(dlg)
  if dlg then
    dlg.TrayMenuToggle.title = "Hide";
    dlg.hidetaskbar = "NO";

--    dlg.placement = "NORMAL";
    dlg:show();
  end
end

local TrayMenuToggle = iup.item {
  title = "Hide";
  action = function(self)
    local text = self.title;
    if text == "Hide" then Hide(MainDialog); else Show(MainDialog); end
  end;
};

local TrayMenu = iup.menu {
  TrayMenuToggle;
  iup.item {title = "Exit", action = function() return iup.CLOSE; end};
};

MainDialog = iup.dialog {
  title   = "MyApp";
  icon  = "MyIcon";
  maxbox  = "NO";
  resize  = "NO";
  tray  = "YES";
  traytip   = "Hello World";
--  trayimage   = "MyIcon";
  TrayMenuToggle = TrayMenuToggle;

  iup.label{title = "    Hello World    "};

  trayclick_cb = function()
    TrayMenu:popup(iup.MOUSEPOS, iup.MOUSEPOS);
  end;

  show_cb = function(self, mode)
--    if 1 == mode then   -- Restore
--      TrayMenuToggle.title = "Hide";
--      self.hidetaskbar = "NO";
    if 2 == mode then -- Minimize
      TrayMenuToggle.title = "Show";
      self.hidetaskbar = "YES";
--    else      -- Show dialog (mode == 0)
--      --iup.Message("Just checking...", tostring(mode));
    end
  end;


  close_cb = function(self)
    self.tray = "NO";
--    return iup.CLOSE;
  end;
};

MainDialog:show();
iup.MainLoop();
