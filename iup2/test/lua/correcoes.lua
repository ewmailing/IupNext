--Example IupGLCanvas in Lua 
--  Creates a OpenGL canvas and draws a line in it. This example uses gllua binding of opengl to Lua.

function k_any(self, c)
  if c == K_q then
    return IUP_CLOSE
  else
    return IUP_DEFAULT
  end
end

function redraw(self, x, y)
  print("redraw(self)")
  IupGLMakeCurrent(self)
  -- clear color unnecessary (called once in reshape())
  glClearColor(1,1,1,1)
  glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT)

  glColor(1,0,0)
  glBegin(GL_LINES) 
  glVertex( 0,  0)
  glVertex(10, 10)
  glEnd()
  glFlush()

  IupGLSwapBuffers(self) 
  return IUP_DEFAULT
end

function reshape(self,w,h)
  -- reshape callback being called twice in initialization. the second call comes with erroneous values.
  print("reshape(self,w="..w..",h="..h..")")
  glClearColor(1,1,1,1)
  glMatrixMode(GL_PROJECTION)
  glLoadIdentity()
  glMatrixMode(GL_MODELVIEW)
  glLoadIdentity()
  glViewport(0,0,w,h)
end
 
canvas = iupglcanvas{action=redraw,
                     resize=reshape, 
                     k_any=k_any, 
                     buffer=IUP_DOUBLE,
                     border=IUP_NO, 
                     rastersize = "300x300"}

dg = iupdialog{canvas}
dg.title = "iupglcanvas"
dg:show()
