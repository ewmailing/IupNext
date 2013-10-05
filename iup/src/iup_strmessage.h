/** \file
 * \brief Language Dependent String Messages 
 *
 * See Copyright Notice in "iup.h"
 */

 
#ifndef __IUP_STRMESSAGE_H 
#define __IUP_STRMESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif


/** \defgroup strmessage Language Dependent String Messages
 * \par
 * String database that is dependend of the selected language.
 * \par
 * See \ref iup_strmessage.h
 * \ingroup util */

/** Returns a common string from a registered coded message.
 * The returned string depends on the global LANGUAGE attribute.
 * \ingroup strmessage */
char* iupStrMessageGet(const char* message);

/* Called from iup_global */
void iupStrMessageUpdateLanguage(const char* language);

/* called only in IupOpen and IupClose */
void iupStrMessageInit(void);
void iupStrMessageFinish(void);


#ifdef __cplusplus
}
#endif

#endif
