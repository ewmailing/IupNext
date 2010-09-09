#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iup.h>

#include "../src/iup_class.h"
#include "../src/iup_register.h"

#define MAX_ITEMS 200

static char* callback_str[] = {"idle", "", "Ii", "ccc", "d", "iiiic",
                        "ff", "fiis", "i", "iff", "ii", "iic", 
                        "iiIII", "iii", "iiii", "iiiiii", "iiiiiis",
                        "iiiis", "iinsii", "iis", "is", "nii",
                        "nn", "s", "sii", "siii", "ss", "i=s", "ii=s",
                        "iiiiiiv"};

static char currentClassName[256];

static int compare(const void *a, const void *b)
{
  return strcmp( * ( char** ) a, * ( char** ) b );
}

/* Update callback labels */
static int callbacksList_ActionCB (Ihandle *ih, char *callName, int pos, int state)
{
  char* temp;
  char params[256], copy[2];
  int i = 0;
  int returns = 0;

  if(!callName)
    return 0;
  
  temp = iupClassGetDefaultAttribute(currentClassName, callName);

  copy[1] = '\0';
  sprintf(params, "%s", "Ihandle*");

  IupSetAttribute(IupGetHandle("labelCallback"), "TITLE", callName);

  while(i < (int)strlen(temp))
  {
    strncpy(copy, temp+i, 1);

    if(!strcmp(copy, "i"))
      sprintf(params, "%s, %s", params, "int");
    else if(!strcmp(copy, "c"))
      sprintf(params, "%s, %s", params, "unsigned char");
    else if(!strcmp(copy, "s"))
      sprintf(params, "%s, %s", params, "char*");
    else if(!strcmp(copy, "f"))
      sprintf(params, "%s, %s", params, "float");
    else if(!strcmp(copy, "d"))
      sprintf(params, "%s, %s", params, "double");
    else if(!strcmp(copy, "I"))
      sprintf(params, "%s, %s", params, "int*");
    else if(!strcmp(copy, "n"))
      sprintf(params, "%s, %s", params, "Ihandle*");
    else if(!strcmp(copy, "v"))
      sprintf(params, "%s, %s", params, "cdCanvas*");
    else if(!strcmp(copy, "="))
    {
      returns = 1;
      break;
    }

    i++;
  }

  if(returns)
    sprintf(params, "%s (= char*)", params);
  else if(!strcmp(callName, "HELP_CB"))
    sprintf(params, "%s (= void)", params);
  else
    sprintf(params, "%s (= int)", params);

  if(!strcmp(temp, "idle"))
    IupSetAttribute(IupGetHandle("labelParams"), "TITLE", "void");
  else
    IupSetAttribute(IupGetHandle("labelParams"), "TITLE", params);

  (void)ih;
  (void)pos;
  (void)state;

  return IUP_DEFAULT;
}

/* Update attribute labels */
static int attributesList_ActionCB (Ihandle *ih, char *attribName, int pos, int state)
{
  char* temp;

  if(!attribName)
    return 0;

  temp = iupClassGetDefaultAttribute(currentClassName, attribName);

  IupSetAttribute(IupGetHandle("labelAttribute"), "TITLE", attribName);

  if(!temp)
    IupSetAttribute(IupGetHandle("labelDefault"), "TITLE", "NULL");
  else
    IupSetAttribute(IupGetHandle("labelDefault"), "TITLE", temp);

  (void)ih;
  (void)pos;
  (void)state;

  return IUP_DEFAULT;
}

static int classesList_ActionCB (Ihandle *ih, char *className, int pos, int state)
{
  int i, j, n;
  char*  temp;
  char **names = (char **) malloc(MAX_ITEMS * sizeof(char *));
  int call_str = sizeof(callback_str) / sizeof(char*);
  
  n = IupGetClassAttributes(className, names, MAX_ITEMS);

  /* Sort names */
  qsort(names, n, sizeof(char*), compare);

  /* Clear lists */
  IupSetAttribute(IupGetHandle("listAttributes"), "REMOVEITEM", NULL);
  IupSetAttribute(IupGetHandle("listCallbacks"),  "REMOVEITEM", NULL);

  /* Populate attribute and callback lists */
  for (i = 0; i < n; i++)
  {
    temp = iupClassGetDefaultAttribute(className, names[i]);
    
    if(!temp)
    {
      /* No default value */
      IupSetAttribute(IupGetHandle("listAttributes"), "APPENDITEM", names[i]);
    }
    else
    {
      for(j = 0; j < call_str; j++)
      {
        /* callback attribute */
        if(!strcmp(temp, callback_str[j]))
        {
          IupSetAttribute(IupGetHandle("listCallbacks"), "APPENDITEM", names[i]);
          break;
        }
      }

      if(j == call_str)
      {
        /* attribute w/ default value */
        IupSetAttribute(IupGetHandle("listAttributes"), "APPENDITEM", names[i]);
      }
    }
  }

  /* Update labels (values) */
  IupSetAttribute(IupGetHandle("labelClass"), "TITLE", className);
  strcpy(currentClassName, className);

  IupSetAttribute(IupGetHandle("labelAttribute"), "TITLE", NULL);
  IupSetAttribute(IupGetHandle("labelDefault"),   "TITLE", NULL);
  IupSetAttribute(IupGetHandle("labelCallback"),  "TITLE", NULL);
  IupSetAttribute(IupGetHandle("labelParams"),    "TITLE", NULL);


  (void)ih;
  (void)pos;
  (void)state;

  return IUP_DEFAULT;
}

void PopulateListOfClasses(void)
{
  int i, n;
  char **list = (char **) malloc(MAX_ITEMS * sizeof(char *));

  n = IupGetAllClasses(list, MAX_ITEMS);
  
  qsort(list, n, sizeof(char*), compare);

  for(i = 0; i < n; i++)
    IupSetAttribute(IupGetHandle("listClasses"), "APPENDITEM", list[i]);
}

void ConfTest(void)
{
  Ihandle *dialog, *box, *lists, *listClasses, *listAttributes, *listCallbacks;
  Ihandle *labelAttribute, *labelDefault, *labelCallback, *labelParams, *labelClass;
  
  listClasses    = IupList(NULL);  /* list of registered classes */
  listAttributes = IupList(NULL);  /* list of attributes of the selected class */
  listCallbacks  = IupList(NULL);  /* list of  callbacks of the selected class */

  labelClass = IupLabel(NULL);

  labelAttribute = IupLabel(NULL);
  labelDefault   = IupLabel(NULL);
  labelCallback  = IupLabel(NULL);
  labelParams    = IupLabel(NULL);

  IupSetAttributes(listClasses,    "EDITBOX=NO, DROPDOWN=NO, NAME=listClasses,    SIZE= 70x85");
  IupSetAttributes(listAttributes, "EDITBOX=NO, DROPDOWN=NO, NAME=listAttributes, SIZE=120x85");
  IupSetAttributes(listCallbacks,  "EDITBOX=NO, DROPDOWN=NO, NAME=listCallbacks,  SIZE=120x85");

  IupSetAttribute(labelClass, "FONTSTYLE", "Bold");
  IupSetAttribute(labelClass, "FGCOLOR", "255 0 0");

  IupSetAttribute(labelClass,     "SIZE", "100x");
  IupSetAttribute(labelAttribute, "SIZE", "100x");
  IupSetAttribute(labelDefault,   "SIZE", "100x");
  IupSetAttribute(labelCallback,  "SIZE", "100x");
  IupSetAttribute(labelParams,    "SIZE", "280x");

  IupSetCallback(listClasses,    "ACTION", (Icallback)    classesList_ActionCB);
  IupSetCallback(listAttributes, "ACTION", (Icallback) attributesList_ActionCB);
  IupSetCallback(listCallbacks,  "ACTION", (Icallback)  callbacksList_ActionCB);

  IupSetHandle("listClasses",    listClasses);
  IupSetHandle("listAttributes", listAttributes);
  IupSetHandle("listCallbacks",  listCallbacks);

  IupSetHandle("labelClass",     labelClass);  
  IupSetHandle("labelAttribute", labelAttribute);
  IupSetHandle("labelDefault",   labelDefault);  
  IupSetHandle("labelCallback",  labelCallback);
  IupSetHandle("labelParams",    labelParams);  

  lists = IupVbox(
            IupHbox(
              IupSetAttributes(IupFrame(IupVbox(listClasses,    NULL)), "TITLE=CLASSES"),
              IupSetAttributes(IupFrame(IupVbox(listAttributes, NULL)), "TITLE=ATTRIBUTES"),
              IupSetAttributes(IupFrame(IupVbox(listCallbacks,  NULL)), "TITLE=CALLBACKS"),
              NULL),
            IupHbox(
              IupSetAttributes(IupFrame(IupHbox(IupVbox(IupLabel("Class: "), IupLabel("Attribute: "),
                                                        IupLabel("Default: "), IupLabel("Callback: "),  
                                                        IupLabel("Parameters: "), NULL),
                                                IupVbox(labelClass, labelAttribute,
                                                        labelDefault, labelCallback,
                                                        labelParams, NULL),
                                                        NULL)), "TITLE=DETAILS"),
              NULL),
            NULL);

  box = IupHbox(lists, NULL);

	IupSetAttributes(lists,"MARGIN=10x10, GAP=10");

  dialog = IupDialog(box);

	IupSetAttribute(dialog, "TITLE", "Class Conference and Documentation");

  IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

  PopulateListOfClasses();
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  ConfTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
