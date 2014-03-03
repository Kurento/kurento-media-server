/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "GStreamerFilterImpl.hpp"
#include <generated/MediaPipeline.hpp>
#include <stdio.h>

#define GST_CAT_DEFAULT kurento_gstreamer_filter_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoGStreamerFilterImpl"


namespace kurento
{

GStreamerFilterImpl::GStreamerFilterImpl (
  const std::string &commandLine,
  std::shared_ptr< MediaObjectImpl > parent, int garbagePeriod) :
  FilterImpl (parent, garbagePeriod)
{
  std::string command, rest_token;

  command = commandLine.substr (0, commandLine.find (' ') );

  GST_DEBUG ("Command %s", command.c_str() );

  g_object_set (element, "filter-factory", command.c_str(), NULL);

  g_object_get (G_OBJECT (element), "filter", &filter, NULL);

  if (filter == NULL) {
    // TODO: Improve exception management
    throw  "Media Object type not found";
  }

  g_object_unref (filter);

  rest_token = commandLine.substr (command.length(), commandLine.length() - 1);

  if (rest_token.front() == ' ') {
    rest_token = rest_token.substr (rest_token.find_first_not_of (" "),
                                    rest_token.length() - 1);
  }

  if (rest_token.length() != 0) {
    setCommandProperties (rest_token);
  }
}

std::shared_ptr<MediaObject>
GStreamerFilter::Factory::createObject (
  std::shared_ptr<MediaPipeline> mediaPipeline,
  const std::string &command, int garbagePeriod)
{
  std::shared_ptr<MediaObject> object (new GStreamerFilterImpl (command,
                                       std::dynamic_pointer_cast<MediaObjectImpl> (mediaPipeline),
                                       garbagePeriod) );

  return object;
}

void
GStreamerFilterImpl::setCommandProperties (const std::string &rest_token)
{
  std::vector<std::string>  words;
  std::vector<std::string>::iterator it;
  gchar **elements;
  GRegex *regex;
  gint i;
  GObjectClass *elementClass;

  regex = g_regex_new ("([a-zA-Z0-9\\- ]*=[ ]*[a-zA-Z0-9\\-\\/\",=]+)",
                       G_REGEX_ANCHORED, G_REGEX_MATCH_ANCHORED, NULL);
  elements = g_regex_split (regex, rest_token.c_str(),
                            G_REGEX_MATCH_NOTEMPTY_ATSTART);
  g_regex_unref (regex);
  i = 0;

  while (elements[i] != NULL) {
    if (g_strcmp0 (elements [i], "") == 0) {
      i++;
      continue;
    }

    std::string aux (elements[i]);
    std::string::iterator end_pos = std::remove (aux.begin(), aux.end(), ' ');
    aux.erase (end_pos, aux.end() );
    words.push_back (aux);
    i++;
  }

  g_strfreev (elements);

  elementClass = G_OBJECT_GET_CLASS (this->filter);

  regex = g_regex_new ("^([a-zA-Z0-9\\-]+)",
                       G_REGEX_OPTIMIZE, G_REGEX_MATCH_ANCHORED, NULL);

  for (std::string aux : words) {
    int i;
    GParamSpec *pspec;

    elements = g_regex_split (regex, aux.c_str(), G_REGEX_MATCH_NOTEMPTY_ATSTART);
    i = 0;

    while (elements[i] != NULL) {
      i++;
    }

    if ( i != 3) {
      g_strfreev (elements);
      continue;
    }

    pspec = g_object_class_find_property (elementClass, elements[1]);

    if (pspec == NULL) {
      GST_ERROR ("Property %s not found", elements[1]);
      g_strfreev (elements);
      continue;
    }

    if ( G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_STRING) {
      g_object_set (G_OBJECT (this->filter), elements[1], (elements[2] + 1), NULL);
      GST_DEBUG ("Setting %s = %s as %s", elements[1], elements[2] + 1,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
    } else if ( (G_PARAM_SPEC_VALUE_TYPE (pspec) ) == G_TYPE_BOOLEAN) {
      if (g_ascii_strcasecmp ( (elements[2] + 1), "true") == 0 ) {
        g_object_set (G_OBJECT (this->filter), elements[1], TRUE, NULL);
      }

      if (g_ascii_strcasecmp ( (elements[2] + 1), "false") == 0 ) {
        g_object_set (G_OBJECT (this->filter), elements[1], FALSE, NULL);
      }

      GST_DEBUG ("Setting %s = %s as %s", elements[1], elements[2] + 1,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
    } else if ( G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_INT) {
      gint aux;
      sscanf (elements[2] + 1, "%d", &aux);
      g_object_set (G_OBJECT (this->filter), elements[1], aux, NULL);
      GST_DEBUG ("Setting %s = %s as %s", elements[1], elements[2] + 1,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
    } else if ( G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_UINT) {
      guint aux;
      sscanf (elements[2] + 1, "%d", &aux);
      g_object_set (G_OBJECT (this->filter), elements[1], aux, NULL);
      GST_DEBUG ("Setting %s = %s as %s", elements[1], elements[2] + 1,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
    } else if ( G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_FLOAT) {
      gfloat aux;
      sscanf (elements[2] + 1, "%f", &aux);
      g_object_set (G_OBJECT (this->filter), elements[1], aux, NULL);
      GST_DEBUG ("Setting %s = %s as %s", elements[1], elements[2] + 1,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
    } else if ( G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_DOUBLE) {
      gdouble aux;
      sscanf (elements[2] + 1, "%lf", &aux);
      g_object_set (G_OBJECT (this->filter), elements[1], aux, NULL);
      GST_DEBUG ("Setting %s = %s as %s", elements[1], elements[2] + 1,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
    } else if ( G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_INT64) {
      gint64 aux;
      sscanf (elements[2] + 1, "%" G_GINT64_FORMAT, &aux);
      g_object_set (G_OBJECT (this->filter), elements[1], aux, NULL);
      GST_DEBUG ("Setting %s = %s as %s", elements[1], elements[2] + 1,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
    } else if ( G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_UINT64) {
      guint64 aux;
      sscanf (elements[2] + 1, "%" G_GUINT64_FORMAT, &aux);
      g_object_set (G_OBJECT (this->filter), elements[1], aux, NULL);
      GST_DEBUG ("Setting %s = %s as %s", elements[1], elements[2] + 1,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
    } else if ( G_TYPE_IS_ENUM (G_PARAM_SPEC_VALUE_TYPE (pspec) ) ) {
      GEnumValue *value;
      GEnumClass *enumClass;
      gint aux;

      enumClass = G_ENUM_CLASS (g_type_class_ref (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );

      if (enumClass == NULL) {
        g_strfreev (elements);
        continue;
      }

      //try to get the enum value from its integer value.
      sscanf (elements[2] + 1, "%d", &aux);
      value = g_enum_get_value (enumClass, aux);

      if (value != NULL) {
        g_object_set (G_OBJECT (this->filter), elements[1], value->value, NULL);
        GST_DEBUG ("Setting %s = %s as %s, %s", elements[1], elements[2] + 1,
                   g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ), value->value_nick);
      } else {
        //try to get the enum value from its name.
        value = g_enum_get_value_by_nick (enumClass, elements[2] + 1);

        if (value != NULL) {
          g_object_set (G_OBJECT (this->filter), elements[1], value->value, NULL);
          GST_DEBUG ("Setting %s = %s as %s, %s", elements[1], elements[2] + 1,
                     g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ), value->value_nick);
        } else {
          GST_ERROR ("Enum value not found");
        }
      }
    } else if ( G_PARAM_SPEC_VALUE_TYPE (pspec) == GST_TYPE_CAPS) {
      GstCaps *caps = gst_caps_from_string ( (elements[2] + 1) );

      g_object_set (G_OBJECT (this->filter), elements[1], caps, NULL);
      GST_DEBUG ("Setting %s = %" GST_PTR_FORMAT " as %s", elements[1], caps,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
      gst_caps_unref (caps);
    } else if ( G_PARAM_SPEC_VALUE_TYPE (pspec) == GST_TYPE_STRUCTURE) {
      GstStructure *st = gst_structure_new_from_string ( (elements[2] + 1) );

      g_object_set (G_OBJECT (this->filter), elements[1], st, NULL);
      GST_DEBUG ("Setting %s = %" GST_PTR_FORMAT " as %s", elements[1], st,
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
      gst_structure_free (st);
    } else {
      GST_DEBUG ("Unknown param %s = %s as type %s", elements[1],
                 (elements[2] + 1), g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec) ) );
    }

    g_strfreev (elements);
  }

  g_regex_unref (regex);
  words.clear ();
}

GStreamerFilterImpl::StaticConstructor GStreamerFilterImpl::staticConstructor;

GStreamerFilterImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
