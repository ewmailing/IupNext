#include <stdlib.h>
#include <math.h>
#include <iup.h>
#include "iupdraw.h"

enum { CREATE, REMOVE, CHANGE_RADIUS };

typedef struct _Circle{
  int x;
  int y;
  int r;
  int rOriginal;
  int rLast;
  int id;
} Circle;

typedef struct _Operation{
  int type;
  Circle circle;
} Operation;

#define DEF_RADIUS 30
#define MAX 80

static Circle circles[MAX];
static int circlesCount = 0;
static int lastId = 0;
static int highlightedCircle = -1;

static Operation undoRedoList[MAX];
static int operationCount = 0;
static int currentOperationId = 0;

static void addOperation(int type, Circle circle)
{
  if (currentOperationId < operationCount - 1)
    operationCount = currentOperationId + 1;
  undoRedoList[operationCount].type = type;
  undoRedoList[operationCount].circle = circle;
  currentOperationId = operationCount;
  ++operationCount;
}

static void undo()
{
  Operation currentOperation = undoRedoList[currentOperationId];

  if (currentOperationId < 0)
    return;

  switch (currentOperation.type)
  {
    case CREATE:
      remove(currentOperation.circle.id);
      break;
    case REMOVE:
      insert(currentOperation.circle);
      break;
    case CHANGE_RADIUS:
      update(currentOperation.circle.id, currentOperation.circle.rLast);
      break;
  }
  currentOperationId--;
}

static void redo()
{
  Operation currentOperation;

  if (currentOperationId + 1 == operationCount)
    return;

  currentOperation = undoRedoList[currentOperationId + 1];

  switch (currentOperation.type)
  {
    case CREATE:
      insert(currentOperation.circle);
      break;
    case REMOVE:
      remove(currentOperation.circle.id);
      break;
    case CHANGE_RADIUS:
      update(currentOperation.circle.id, currentOperation.circle.r);
      break;
  }
  currentOperationId++;
}

static int undoLeft()
{
  if (currentOperationId < 0)
    return 0;

  return 1;
}

static int redoLeft()
{
  if (currentOperationId + 1 == operationCount)
    return 0;

  return 1;
}

static int getCount()
{
  return circlesCount;
}

static int getLasId()
{
  return lastId;
}

static Circle get(int id)
{
  int i;
  for (i = 0; i < circlesCount; i++)
  {
    if (circles[i].id == id)
      break;
  }
  return circles[i];
}

static int create(int x, int y, int r)
{
  Circle circle;
  if (circlesCount >= 50)
    return 0;
  circles[circlesCount].x = x;
  circles[circlesCount].y = y;
  circles[circlesCount].r = r;
  circles[circlesCount].rOriginal = r;
  circles[circlesCount].rLast = r;
  circles[circlesCount].id = lastId;
  ++circlesCount;
  ++lastId;
  return lastId - 1;
}

static int insert(Circle circle)
{
  int i;
  if (circlesCount >= 50)
    return 0;
  for (i = circlesCount - 1; i >= 0; i--)
  {
    if (circle.id > circles[i].id)
      break;
    circles[i + 1] = circles[i];
  }
  circles[i + 1] = circle;
  ++circlesCount;
  return 1;
}

static int remove(int id)
{
  int i, j;
  for (i = 0; i < circlesCount; i++)
  {
    if (circles[i].id == id)
      break;
  }

  for (j = i; j < circlesCount; j++)
    circles[j] = circles[j + 1];
  --circlesCount;
  return 1;
}

static int update(int id, int r)
{
  int i;
  for (i = 0; i < circlesCount; i++)
  {
    if (circles[i].id == id)
      break;
  }
  circles[i].rLast = circles[i].r;
  circles[i].r = r;
  return 1;
}

static int pick(int x, int y)
{
  int i;
  double dist, dx, dy;
  int selectedCircle = -1;
  for (i = 0; i < circlesCount; i++)
  {
    double dx = circles[i].x - x;
    double dy = circles[i].y - y;
    double d = sqrt((dx*dx) + (dy*dy));
    if (d < circles[i].r)
    {
      if (selectedCircle < 0)
      {
        selectedCircle = circles[i].id;
        dist = d;
      }
      else if (d < dist)
        selectedCircle = circles[i].id;
    }
  }

  return selectedCircle;
}

int bt_undo_cb(Ihandle *self)
{
  Ihandle *canvas = IupGetDialogChild(self, "CANVAS");
  Ihandle *undoButton = IupGetDialogChild(self, "UNDO");
  Ihandle *redoButton = IupGetDialogChild(self, "REDO");

  undo();

  if (undoLeft())
    IupSetAttribute(undoButton, "ACTIVE", "YES");
  else
    IupSetAttribute(undoButton, "ACTIVE", "NO");


  if (redoLeft())
    IupSetAttribute(redoButton, "ACTIVE", "YES");
  else
    IupSetAttribute(redoButton, "ACTIVE", "NO");

  IupUpdate(canvas);

  return IUP_DEFAULT;
}

int bt_redo_cb(Ihandle *self)
{
  Ihandle *canvas = IupGetDialogChild(self, "CANVAS");
  Ihandle *undoButton = IupGetDialogChild(self, "UNDO");
  Ihandle *redoButton = IupGetDialogChild(self, "REDO");

  redo();

  if (undoLeft())
    IupSetAttribute(undoButton, "ACTIVE", "YES");
  else
    IupSetAttribute(undoButton, "ACTIVE", "NO");


  if (redoLeft())
    IupSetAttribute(redoButton, "ACTIVE", "YES");
  else
    IupSetAttribute(redoButton, "ACTIVE", "NO");

  IupUpdate(canvas);

  return IUP_DEFAULT;
}
static int motion_cb(Ihandle *ih, int x, int y, char* status)
{
  highlightedCircle = pick(x, y);
  IupUpdate(ih);
  return IUP_DEFAULT;
}

static int button_cb(Ihandle *ih, int but, int pressed, int x, int y, char* status)
{
  Ihandle *dial = IupGetDialog(ih);

  if (but == IUP_BUTTON1 && pressed)
  {
    Ihandle *undoButton = IupGetDialogChild(ih, "UNDO");
    Ihandle *redoButton = IupGetDialogChild(ih, "REDO");
    int id = create(x, y, DEF_RADIUS);
    addOperation(CREATE, get(id));
    /***********************************************************************************/
    for (int i = 0; i < operationCount; i++)
    {
      printf("list[%d] = %d\n", i, undoRedoList[i].circle.id);
    }
    printf("Count = %d\n", operationCount);
    printf("Operation Ind = %d\n", operationCount);
    /***********************************************************************************/

    if (undoLeft())
      IupSetAttribute(undoButton, "ACTIVE", "YES");
    else
      IupSetAttribute(undoButton, "ACTIVE", "NO");

    if (redoLeft())
      IupSetAttribute(redoButton, "ACTIVE", "YES");
    else
      IupSetAttribute(redoButton, "ACTIVE", "NO");
  }
  else if (but == IUP_BUTTON3 && pressed)
  {
    int id = pick(x, y);
    if (id >= 0)
    {
      Ihandle *configDial = (Ihandle *)IupGetAttribute(dial, "CONFIGDIAL");
      IupSetInt(configDial, "CIRCLEID", id);
      IupPopup(IupGetAttribute(dial, "CONFIGDIAL"), IUP_CENTER, IUP_CENTER);
      highlightedCircle = -1;
    }
  }
  IupUpdate(ih);

  return IUP_DEFAULT;
}

static int action(Ihandle *ih, float posx, float posy)
{
  int i, w, h;

  IupDrawBegin(ih);

  IupDrawGetSize(ih, &w, &h);

  IupSetAttribute(ih, "DRAWCOLOR", "255 255 255");
  IupSetAttribute(ih, "DRAWSTYLE", "FILL");
  IupDrawRectangle(ih, 0, 0, w - 1, h - 1);

  IupSetAttribute(ih, "DRAWCOLOR", "0 0 0");
  IupSetAttribute(ih, "DRAWSTYLE", "STROKE");

  for (i = 0; i < getCount(); i++)
  {
    Circle circle = circles[i];

    if (highlightedCircle == circle.id)
    {
      IupSetAttribute(ih, "DRAWCOLOR", "128 128 128");
      IupSetAttribute(ih, "DRAWSTYLE", "FILL");
    }
    else
    {
      IupSetAttribute(ih, "DRAWCOLOR", "0 0 0");
      IupSetAttribute(ih, "DRAWSTYLE", "STROKE");
    }

    IupDrawArc(ih, circles[i].x - circles[i].r, circles[i].y - circles[i].r,
               circles[i].x + circles[i].r, circles[i].y + circles[i].r, 0., 360.);
  }

  IupDrawEnd(ih);
  return IUP_DEFAULT;
}

int val_valuechanged_cb(Ihandle *self)
{
  Ihandle* dial = IupGetDialog(self);
  Ihandle* canvas = (Ihandle *)IupGetAttribute(self, "CANVAS");
  Ihandle *undoButton = (Ihandle *)IupGetDialogChild(self, "UNDO");
  Ihandle *redoButton = (Ihandle *)IupGetDialogChild(self, "REDO");
  int circleId = IupGetInt(dial, "CIRCLEID");
  double value = IupGetDouble(self, "VALUE");
  Circle circle = get(circleId);
  int rOriginal = circle.rOriginal;

  update(circleId, rOriginal*value);

  addOperation(CHANGE_RADIUS, get(circleId));

  if (undoLeft())
    IupSetAttribute(undoButton, "ACTIVE", "YES");
  else
    IupSetAttribute(undoButton, "ACTIVE", "NO");

  if (redoLeft)
    IupSetAttribute(redoButton, "ACTIVE", "YES");
  else
    IupSetAttribute(redoButton, "ACTIVE", "NO");

  IupUpdate(canvas);

  return IUP_DEFAULT;
}

Ihandle *createCircleConfigDial()
{
  Ihandle *dial, *val, *vbox, *box;

  val = IupVal("HORIZONTAL");
  IupSetAttribute(val, "EXPAND", "HORIZONTAL");
  IupSetAttribute(val, "MIN", "0.5");
  IupSetAttribute(val, "MAX", "1.5");
  IupSetAttribute(val, "VALUE", "1");

  box = IupVbox(IupFill(), val, IupFill(), NULL);
  IupSetAttribute(box, "ALIGMENT", "ACENTER");

  IupSetAttribute(box, "NMARGIN", "10x10");
  dial = IupDialog(box);
  IupSetAttribute(dial, "SIZE", "100x60");

  IupSetCallback(val, "VALUECHANGED_CB", (Icallback)val_valuechanged_cb);

  return dial;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *hbox, *vbox, *configDial;
  Ihandle *undoButton, *redoButton, *canvas;

  IupOpen(&argc, &argv);

  undoButton = IupButton("Undo", NULL);
  IupSetAttribute(undoButton, "NAME", "UNDO");
  IupSetAttribute(undoButton, "SIZE", "60");
  redoButton = IupButton("Redo", NULL);
  IupSetAttribute(redoButton, "NAME", "REDO");
  IupSetAttribute(redoButton, "SIZE", "60");

  IupSetAttribute(redoButton, "ACTIVE", "NO");
  IupSetAttribute(undoButton, "ACTIVE", "NO");

  canvas = IupCanvas(NULL);
  IupSetAttribute(canvas, "NAME", "CANVAS");
  IupSetAttribute(canvas, "EXPAND", "YES");

  hbox = IupHbox(IupFill(), undoButton, redoButton, IupFill(), NULL);

  vbox = IupVbox(hbox, canvas, NULL);

  IupSetAttribute(vbox, "NMARGIN", "10x10");
  IupSetAttribute(vbox, "GAP", "10");
  dlg = IupDialog(vbox);
  IupSetAttribute(dlg, "TITLE", "Circle Drawer");
  IupSetAttribute(dlg, "SIZE", "300x150");

  /* Registers callbacks */
  IupSetCallback(undoButton, "ACTION", (Icallback)bt_undo_cb);
  IupSetCallback(redoButton, "ACTION", (Icallback)bt_redo_cb);
  IupSetCallback(canvas, "ACTION", (Icallback)action);
  IupSetCallback(canvas, "BUTTON_CB", (Icallback)button_cb);
  IupSetCallback(canvas, "MOTION_CB", (Icallback)motion_cb);

  configDial = createCircleConfigDial();

  IupSetAttribute(configDial, "CANVAS", (char *)canvas);
  IupSetAttribute(configDial, "UNDO", (char *)undo);
  IupSetAttribute(configDial, "REDO", (char *)redo);

  IupSetAttribute(dlg, "CONFIGDIAL", (char *)configDial);

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();

  IupClose();
  return EXIT_SUCCESS;
}
