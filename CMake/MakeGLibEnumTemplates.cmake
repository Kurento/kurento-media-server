message(STATUS ${headers})
file(WRITE ${htemplate} "/*** BEGIN file-header ***/
#ifndef __${includeguard}_ENUM_TYPES_H__
#define __${includeguard}_ENUM_TYPES_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*** END file-header ***/
/*** BEGIN file-production ***/

/* enumerations from \"@filename@\" */

/*** END file-production ***/
/*** BEGIN value-header ***/
GType @enum_name@_get_type (void);
#define GST_TYPE_@ENUMSHORT@ (@enum_name@_get_type())

/*** END value-header ***/
/*** BEGIN file-tail ***/
G_END_DECLS

#endif /* __${includeguard}_ENUM_TYPES_H__ */
/*** END file-tail ***/
")

# Write the .c template
file(WRITE ${ctemplate} "/*** BEGIN file-header ***/
#include \"${name}.h\"\n")
foreach(header ${headers})
  file(APPEND ${ctemplate} "#include \"${header}\"\n")
endforeach(header)
file(APPEND ${ctemplate} "
/*** END file-header ***/
/*** BEGIN file-production ***/

/* enumerations from \"@filename@\" */
/*** END file-production ***/
/*** BEGIN value-header ***/
GType
@enum_name@_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;
  if (g_once_init_enter (&g_define_type_id__volatile)) {
    static const G@Type@Value values[] = {
/*** END value-header ***/
/*** BEGIN value-production ***/
{ @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" },
/*** END value-production ***/
/*** BEGIN value-tail ***/
{ 0, NULL, NULL }
    };
    GType g_define_type_id = g_@type@_register_static (\"@EnumName@\", values);
    g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
  }
  return g_define_type_id__volatile;
}

/*** END value-tail ***/
")
