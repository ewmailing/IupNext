--Example IupGLCanvas in Lua 
--  Creates a OpenGL canvas and draws a line in it. This example uses gllua binding of opengl to Lua.

 function k_any(self, c)
   print("k_any(c="..c..")")
   if c == K_q then
     return IUP_CLOSE
   else
     return IUP_DEFAULT
   end
 end
 
 function redraw(self, x, y)
   IupGLMakeCurrent(self)
   print("redraw(x="..x..",y="..y..")")
 
   glClearColor(1.0, 1.0, 1.0, 1.0)
   glClear(GL_COLOR_BUFFER_BIT)
   glClear(GL_DEPTH_BUFFER_BIT)
   glMatrixMode( GL_PROJECTION )
   glViewport(0, 0, 300, 300)
   glLoadIdentity()
 
   glBegin( GL_LINES ) 
   glColor(1.0, 0.0, 0.0)
   glVertex(0.0, 0.0)
   glVertex(10.0, 10.0)
   glEnd()
 
   IupGLSwapBuffers(self) 
   return IUP_DEFAULT
 end

 function reshape(self, w, h)
   print("reshape(w="..w..",h="..h..")")
   return IUP_DEFAULT
 end

 function button(self, b, e, x, y)
   print("mouse(b="..b..",e="..e..",x="..x..",y="..y..")")
   return IUP_DEFAULT
 end
 
 canvas = iupglcanvas{action=redraw,
                      resize=reshape,
                      button=button,
                      k_any=k_any, 
                      buffer=IUP_DOUBLE, 
                      rastersize = "300x300"}
 
 dg = iupdialog{canvas}
 dg.title = "iupglcanvas"
 dg:show()
