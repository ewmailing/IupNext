/** \file
 * \brief Macros for the Ihandle structure.
 * We should avoid using this in the future.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IMACROS_H 
#define __IMACROS_H

/* control access macros */
#define foreachchild(c,n) for (c=child(n); c; c=brother(c))
#define child(_)         ((_)->firstchild)
#define parent(_)        ((_)->parent)
#define brother(_)       ((_)->brother)

/* Ihandle structure access */
#define type(n)          ((n)->type)
#define hclass(n)        ((n)->iclass)
#define env(n)           ((n)->enviroment)
#define number(n)        ((n)->serialnumber)
#define handle(n)        ((n)->handle)
#define usersize(n)      ((n)->usersize)
#define posx(n)          ((n)->posx)
#define posy(n)          ((n)->posy)
#define naturalsize(n)   ((n)->natural)
#define naturalwidth(n)  ((n)->natural.width)
#define naturalheight(n) ((n)->natural.height)
#define currentwidth(n)  ((n)->current.width)
#define currentheight(n) ((n)->current.height)

#define image_data(_)    ((_)->attrib.data)
#define button_data(_)   ((_)->attrib.data)
#define canvas_data(_)   ((_)->attrib.data)
#define text_data(_)     ((_)->attrib.data)
#define matrix_data(_)   ((_)->attrib.data)
#define dialog_data(_)   ((_)->attrib.data)
#define keyaction_list(_) ((_)->attrib.k.list)
#define ddlist_data(_)   ((_)->attrib.lst.dddata)
#define list_data(_)     ((_)->attrib.lst.data)

#endif
