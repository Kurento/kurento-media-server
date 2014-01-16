/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
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
#include <string.h>
#include <libsoup/soup.h>
#include "KmsHttpPost.h"

#define OBJECT_NAME "HttpPost"
#define MIME_MULTIPART_FORM_DATA "multipart/form-data"

#define GST_CAT_DEFAULT kms_http_post_debug_category
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define KMS_HTTP_POST_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_HTTP_POST, KmsHttpPostPrivate))

typedef enum {
  MULTIPART_INITIAL_STAGE,
  MULTIPART_SKIP_PREAMBLE,
  MULTIPART_READ_HEADERS,
  MULTIPART_CHECK_HEADERS,
  MULTIPART_READ_CONTENT,
  MULTIPART_IGNORE_CONTENT,
  MULTIPART_FINISHED
} ParseState;

typedef struct _KmsHttpPostMultipart {
  SoupMessageHeaders *headers;
  gchar *boundary;
  ParseState state;
  gchar *tmp_buff;
  guint len;
} KmsHttpPostMultipart;

struct _KmsHttpPostPrivate {
  KmsHttpPostMultipart *multipart;
  SoupMessage *msg;
  gulong chunk_id;
  gulong finish_id;
};

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (KmsHttpPost, kms_http_post,
                         G_TYPE_OBJECT,
                         GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, OBJECT_NAME,
                             0, "debug category for " OBJECT_NAME " element") )
/* properties */
enum {
  PROP_0,

  PROP_MESSAGE,
  N_PROPERTIES
};

#define KMS_HTTP_PORT_DEFAULT_MESSAGE NULL

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/* signals */
enum {
  GOT_DATA,
  FINISHED,
  LAST_SIGNAL
};

static guint obj_signals[LAST_SIGNAL] = { 0 };

static void
kms_http_post_concat_previous_buffer (KmsHttpPost *self, const char **start,
                                      const char **end)
{
  if (self->priv->multipart->tmp_buff == NULL) {
    return;
  }

  if (self->priv->multipart->tmp_buff <= *start &&
      *start <= self->priv->multipart->tmp_buff + self->priv->multipart->len) {
    /* Memory overlap => Do not reallocate more memory in this case */
    return;
  }

  self->priv->multipart->tmp_buff = (char *) g_realloc (
                                      self->priv->multipart->tmp_buff,
                                      self->priv->multipart->len + (*end - *start) );
  memmove (self->priv->multipart->tmp_buff + self->priv->multipart->len,
           *start, *end - *start);
  self->priv->multipart->len += (*end - *start);

  *start = self->priv->multipart->tmp_buff;
  *end = self->priv->multipart->tmp_buff + self->priv->multipart->len;
}

static void
kms_notify_buffer_data (KmsHttpPost *self, const char *start, const char *end)
{
  SoupBuffer *buffer;

  buffer = soup_buffer_new (SOUP_MEMORY_STATIC, start, end - start);

  g_signal_emit (G_OBJECT (self), obj_signals[GOT_DATA], 0, buffer);

  soup_buffer_free (buffer);
}

static void
kms_http_post_skip_preamble (KmsHttpPost *self, const char **start,
                             const char **end)
{
  const char *boundary = self->priv->multipart->boundary;
  int boundary_len = strlen (boundary);
  const char *b;

  if (self->priv->multipart->tmp_buff != NULL) {
    kms_http_post_concat_previous_buffer (self, start, end);
  }

  b = (const char *) memchr (*start, '-', *end - *start);

  if (b == NULL || b != *start) {
    /*first line of the body is not the boundary tag */
    self->priv->multipart->state = MULTIPART_IGNORE_CONTENT;
    return;
  }

  if (b + boundary_len + 4 > *end) {
    /* boundary does not fit in this buffer */
    gchar *mem = NULL;

    if (self->priv->multipart->tmp_buff != NULL) {
      mem = self->priv->multipart->tmp_buff;
    }

    self->priv->multipart->tmp_buff = (gchar *) g_memdup (b, *end - b);
    self->priv->multipart->len = *end - b;

    if (mem != NULL) {
      g_free (mem);
    }

    /* Move start pointer up to the end */
    *start = *end;
    return;
  }

  /* Check for "--boundary" */
  if (b[1] != '-' ||
      memcmp (b + 2, boundary, boundary_len) != 0) {
    /*first line of the body is not the boundary tag */
    self->priv->multipart->state = MULTIPART_IGNORE_CONTENT;
    return;
  }

  /* Check for "--" or "\r\n" after boundary */
  if ( (b[boundary_len + 2] == '-' && b[boundary_len + 3] == '-') ||
       (b[boundary_len + 2] == '\r' && b[boundary_len + 3] == '\n') ) {

    if (b[boundary_len + 2] == '\r' && b[boundary_len + 3] == '\n') {
      /* End of this body part */
      self->priv->multipart->state = MULTIPART_READ_HEADERS;
    } else {
      /* Double hyphens at the end of the boundary marks the end */
      /* of the multipart post requets */
      self->priv->multipart->state = MULTIPART_FINISHED;
    }
  }

  if (*end > b + boundary_len + 4) {
    *start = b + boundary_len + 4;
    return;
  }

  if (self->priv->multipart->tmp_buff != NULL) {
    g_free (self->priv->multipart->tmp_buff);
    self->priv->multipart->tmp_buff = NULL;
  }

  /* Move start pointer up to the end */
  *start = *end;
}

static void
kms_http_post_read_until_boundary (KmsHttpPost *self, const char **start,
                                   const char **end, gboolean ignore)
{
  const char *boundary = self->priv->multipart->boundary;
  int boundary_len = strlen (boundary);
  const char *b;

  if (self->priv->multipart->tmp_buff != NULL) {
    kms_http_post_concat_previous_buffer (self, start, end);
  }

  for (b = (const char *) memchr (*start, '\r', *end - *start); b != NULL;
       b = (const char *) memchr (b + 1, '\r', *end - (b + 1) ) ) {

    if (b + boundary_len + 6 > *end) {
      /* boundary does not fit in this buffer */
      gchar *mem = NULL;

      if (self->priv->multipart->tmp_buff != NULL) {
        mem = self->priv->multipart->tmp_buff;
      }

      self->priv->multipart->tmp_buff = (gchar *) g_memdup (b, *end - b);
      self->priv->multipart->len = *end - b;

      /* Notify data read so far */
      if (!ignore && *start < b) {
        kms_notify_buffer_data (self, *start, b );
      }

      if (mem != NULL) {
        g_free (mem);
      }

      /* Move start pointer up to the end */
      *start = *end;
      return;
    }

    /* Check for "\r\n--boundary" */
    if (b[1] != '\n' || b[2] != '-' || b[3] != '-' ||
        memcmp (b + 4, boundary, boundary_len) != 0) {
      continue;
    }

    /* Check for "--" or "\r\n" after boundary */
    if ( (b[boundary_len + 4] == '-' && b[boundary_len + 5] == '-') ||
         (b[boundary_len + 4] == '\r' && b[boundary_len + 5] == '\n') ) {

      if (b[boundary_len + 4] == '\r' && b[boundary_len + 5] == '\n') {
        /* End of this body part */
        self->priv->multipart->state = MULTIPART_READ_HEADERS;
      } else {
        /* Double hyphens at the end of the boundary marks the end */
        /* of the multipart post requets */
        self->priv->multipart->state = MULTIPART_FINISHED;
      }

      /* Notify data read so far */
      if (!ignore && *start < b) {
        kms_notify_buffer_data (self, *start, b);
      }

      if (*end <= b + boundary_len + 6) {
        /* Free temporal buffer */
        break;
      }

      *start = b + boundary_len + 6;
      return;
    }
  }

  /* Notify data */
  if (!ignore && b == NULL) {
    kms_notify_buffer_data (self, *start, *end);
  }

  if (self->priv->multipart->tmp_buff != NULL) {
    g_free (self->priv->multipart->tmp_buff);
    self->priv->multipart->tmp_buff = NULL;
  }

  /* Move start pointer up to the end */
  *start = *end;
}

static void
kms_http_post_parse_header (KmsHttpPost *self, const char *start,
                            const char *end)
{
  const char *name_start, *name_end, *value_start, *value_end, *eol;
  char *name, *value;

  name_start = start;
  name_end = strchr (start, ':');

  /* Reject if there is no ':', or the header name is
   * empty, or it contains whitespace.
   */
  if (name_end == NULL || name_end == name_start ||
      name_start + strcspn (name_start, " \t\r\n") < name_end) {
    /* Ignore this line. */
    return;
  }

  /* Find the end of the value; ie, an end-of-line that
   * isn't followed by a continuation line.
   */
  value_start = name_end + 1;
  value_end = strchr (name_start, '\n');

  if (value_end == NULL) {
    return;
  }

  /* Skip leading whitespace */
  while (value_start < value_end &&
         (*value_start == ' ' || *value_start == '\t' ||
          *value_start == '\r' || *value_start == '\n') ) {
    value_start++;
  }

  /* clip trailing whitespace */
  eol = value_end;

  while (eol > value_start &&
         (eol[-1] == ' ' || eol[-1] == '\t' || eol[-1] == '\r') ) {
    eol--;
  }

  name = g_strndup (name_start, name_end - name_start );
  value = g_strndup (value_start, eol - value_start );

  /* Add new header */
  soup_message_headers_append (self->priv->multipart->headers, name, value);

  g_free (name);
  g_free (value);
}

static void
kms_http_post_read_headers (KmsHttpPost *self, const char **start,
                            const char **end)
{
  const char *b;

  if (self->priv->multipart->tmp_buff != NULL) {
    kms_http_post_concat_previous_buffer (self, start, end);
  }

  b = *start;

  while (b <= *end) {
    const char *newline = (const char *) memchr (b, '\n', *end - b);

    if (newline == NULL) {
      /* header does not fit in this buffer */
      gchar *mem = NULL;

      if (self->priv->multipart->tmp_buff != NULL) {
        mem = self->priv->multipart->tmp_buff;
      }

      self->priv->multipart->tmp_buff = (gchar *) g_memdup (b, *end - b);
      self->priv->multipart->len = *end - b;

      if (mem != NULL) {
        g_free (mem);
      }

      /* Move start pointer up to the end */
      *start = *end;
      return;
    }

    /* Check if this is a blank line */
    if (newline - b == 1 && b[0] == '\r' && b[1] == '\n') {
      self->priv->multipart->state = MULTIPART_CHECK_HEADERS;

      if (*end <= newline + 1) {
        /* Free temporal buffer */
        break;
      }

      *start = newline + 1;
      return;
    }

    kms_http_post_parse_header (self, b, newline);

    /* Search next header starting from the next character following '\n' */
    b = newline + 1;
  }

  if (self->priv->multipart->tmp_buff != NULL) {
    g_free (self->priv->multipart->tmp_buff);
    self->priv->multipart->tmp_buff = NULL;
  }

  /* Move start pointer up to the end */
  *start = *end;
}

static void
kms_http_post_check_headers (KmsHttpPost *self)
{
  GHashTable *params = NULL;
  gchar *disposition = NULL;

  if (self->priv->multipart->headers == NULL) {
    return;
  }

  if (!soup_message_headers_get_content_disposition (
        self->priv->multipart->headers, &disposition, &params) ) {
    goto end;
  }

  /* We are only interested in filename param */
  if (g_hash_table_contains (params, "filename") ) {
    self->priv->multipart->state = MULTIPART_READ_CONTENT;
  } else {
    self->priv->multipart->state = MULTIPART_IGNORE_CONTENT;
  }

end:

  if (disposition != NULL) {
    g_free (disposition);
  }

  if (params != NULL) {
    g_hash_table_destroy (params);
  }
}

static void
kms_http_post_check_preamble (KmsHttpPost *self, const char **start,
                              const char **end)
{
  if (*end - *start < 1) {
    return;
  }

  if (*start[0] == '\r') {
    /* Multipart without preamble */
    self->priv->multipart->state = MULTIPART_IGNORE_CONTENT;
    return;
  }

  /* RFC 1341 7.2.1 Multipart: The common syntax */

  /* Note that the encapsulation boundary must occur at the beginning of a line,
   * i.e., following a CRLF, and that initial CRLF is considered to be part of
   * the encapsulation boundary rather than part of the preceding part.
   * The requirement that the encapsulation boundary begins with a CRLF implies
   * that the body of a multipart entity must itself begin with a CRLF before
   * the first encapsulation line -- that is, if the "preamble" area is not
   * used, the entity headers must be followed by TWO CRLFs. This is indeed how
   * such entities should be composed. A tolerant mail reading program, however,
   * may interpret a body of type multipart that begins with an encapsulation
   * line NOT initiated by a CRLF as also being an encapsulation boundary
   */

  self->priv->multipart->state = MULTIPART_SKIP_PREAMBLE;
}

static void
kms_http_post_parse_multipart_data (KmsHttpPost *self, const char *start,
                                    const char *end)
{
  while (start != end) {
    switch (self->priv->multipart->state) {
    case MULTIPART_INITIAL_STAGE:
      kms_http_post_check_preamble (self, &start, &end);
      break;

    case MULTIPART_SKIP_PREAMBLE:
      kms_http_post_skip_preamble (self, &start, &end);
      break;

    case MULTIPART_READ_HEADERS:
      kms_http_post_read_headers (self, &start, &end);
      break;

    case MULTIPART_CHECK_HEADERS:
      kms_http_post_check_headers (self);
      break;

    case MULTIPART_IGNORE_CONTENT:
      kms_http_post_read_until_boundary (self, &start, &end, TRUE);

      if (self->priv->multipart->state != MULTIPART_IGNORE_CONTENT) {
        soup_message_headers_clear (self->priv->multipart->headers);
      }

      break;

    case MULTIPART_READ_CONTENT:
      kms_http_post_read_until_boundary (self, &start, &end, FALSE);

      if (self->priv->multipart->state != MULTIPART_READ_CONTENT) {
        /* Do not process anything else */
        self->priv->multipart->state = MULTIPART_FINISHED;
      }

      break;

    case MULTIPART_FINISHED:
      /* Ignore next data if there is any */
      return;
    }
  }
}

static void
got_chunk_cb (SoupMessage *msg, SoupBuffer *chunk, gpointer data)
{
  KmsHttpPost *self = KMS_HTTP_POST (data);

  if (self->priv->multipart != NULL) {
    /* Extract data from body parts */
    kms_http_post_parse_multipart_data (self, chunk->data,
                                        chunk->data + chunk->length);
  } else {
    /* Data received in a non multipart POST request is */
    /* provided as it is without any further processing */
    kms_notify_buffer_data (self, chunk->data, chunk->data + chunk->length);
  }
}

static void
kms_http_post_destroy_multipart (KmsHttpPost *self)
{
  if (self->priv->multipart == NULL) {
    return;
  }

  g_free (self->priv->multipart->boundary);

  if (self->priv->multipart->headers != NULL) {
    soup_message_headers_free (self->priv->multipart->headers);
  }

  g_free (self->priv->multipart->tmp_buff);

  g_slice_free (KmsHttpPostMultipart, self->priv->multipart);
  self->priv->multipart = NULL;
}

static void
kms_http_post_init_multipart (KmsHttpPost *self)
{
  if (self->priv->multipart != NULL) {
    GST_WARNING ("Multipart data is already initialized");
    kms_http_post_destroy_multipart (self);
  }

  self->priv->multipart = g_slice_new0 (KmsHttpPostMultipart);
  self->priv->multipart->headers =
    soup_message_headers_new (SOUP_MESSAGE_HEADERS_MULTIPART);
}

static void
kms_http_post_release_message (KmsHttpPost *self)
{
  if (self->priv->msg == NULL) {
    return;
  }

  if (self->priv->chunk_id != 0L) {
    g_signal_handler_disconnect (self->priv->msg, self->priv->chunk_id);
    self->priv->chunk_id = 0L;
  }

  if (self->priv->finish_id != 0L) {
    g_signal_handler_disconnect (self->priv->msg, self->priv->finish_id);
    self->priv->finish_id = 0L;
  }

  g_clear_object (&self->priv->msg);
}

static void
finished_cb (SoupMessage *msg, gpointer data)
{
  KmsHttpPost *self = KMS_HTTP_POST (data);

  kms_http_post_release_message (self);
  kms_http_post_destroy_multipart (self);

  g_signal_emit (G_OBJECT (self), obj_signals[FINISHED], 0, NULL);
}

static void
kms_http_post_configure_msg (KmsHttpPost *self)
{
  const gchar *content_type;
  GHashTable *params = NULL;

  content_type =
    soup_message_headers_get_content_type (self->priv->msg->request_headers,
        &params);

  if (content_type == NULL) {
    GST_WARNING ("Content-type header is not present in request");
    soup_message_set_status (self->priv->msg, SOUP_STATUS_NOT_ACCEPTABLE);
    goto end;
  }

  if (strncmp (content_type, "multipart/", 10) == 0) {
    if (strlen (content_type) >= 19 &&
        strncmp (content_type + 11, "form-data", 9) ) {
      /* Content-Type: multipart/form-data */
      kms_http_post_init_multipart (self);
      self->priv->multipart->boundary =
        g_strdup ( (gchar *) g_hash_table_lookup (params, "boundary") );

      if (self->priv->multipart->boundary == NULL) {
        GST_WARNING ("Malformed multipart POST request");
        kms_http_post_destroy_multipart (self);
        soup_message_set_status (self->priv->msg, SOUP_STATUS_NOT_ACCEPTABLE);
        goto end;
      }
    } else {
      GST_WARNING ("Unsupported multipart format: %s", content_type);
      soup_message_set_status (self->priv->msg, SOUP_STATUS_NOT_ACCEPTABLE);
      goto end;
    }
  }

  soup_message_set_status (self->priv->msg, SOUP_STATUS_OK);

  /* Get chunks without filling-in body's data field after */
  /* the body is fully sent/received */
  soup_message_body_set_accumulate (self->priv->msg->request_body, FALSE);

  self->priv->chunk_id = g_signal_connect (self->priv->msg, "got-chunk",
                         G_CALLBACK (got_chunk_cb), self);
  self->priv->finish_id = g_signal_connect (self->priv->msg, "finished",
                          G_CALLBACK (finished_cb), self);
end:

  if (params != NULL) {
    g_hash_table_destroy (params);
  }
}

static void
kms_http_post_set_property (GObject *obj, guint prop_id,
                            const GValue *value, GParamSpec *pspec)
{
  KmsHttpPost *self = KMS_HTTP_POST (obj);

  switch (prop_id) {
  case PROP_MESSAGE:
    kms_http_post_release_message (self);
    kms_http_post_destroy_multipart (self);

    if (SOUP_IS_MESSAGE (g_value_get_object (value) ) ) {
      self->priv->msg = SOUP_MESSAGE (g_object_ref (
                                        g_value_get_object (value) ) );
      kms_http_post_configure_msg (self);
    }

    break;

  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    break;
  }
}

static void
kms_http_post_get_property (GObject *obj, guint prop_id, GValue *value,
                            GParamSpec *pspec)
{
  KmsHttpPost *self = KMS_HTTP_POST (obj);

  switch (prop_id) {
  case PROP_MESSAGE:
    g_value_set_object (value, self->priv->msg);
    break;

  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    break;
  }
}

static void
kms_http_post_dispose (GObject *obj)
{
  KmsHttpPost *self = KMS_HTTP_POST (obj);

  kms_http_post_release_message (self);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (kms_http_post_parent_class)->dispose (obj);
}

static void
kms_http_post_finalize (GObject *obj)
{
  KmsHttpPost *self = KMS_HTTP_POST (obj);

  kms_http_post_destroy_multipart (self);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (kms_http_post_parent_class)->finalize (obj);
}

static void
kms_http_post_class_init (KmsHttpPostClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = kms_http_post_set_property;
  gobject_class->get_property = kms_http_post_get_property;
  gobject_class->dispose = kms_http_post_dispose;
  gobject_class->finalize = kms_http_post_finalize;

  obj_properties[PROP_MESSAGE] =
    g_param_spec_object ("soup-message",
                         "Soup message object",
                         "Message to get data from",
                         SOUP_TYPE_MESSAGE,
                         (GParamFlags) (G_PARAM_READWRITE) );

  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);

  obj_signals[GOT_DATA] =
    g_signal_new ("got-data",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (KmsHttpPostClass, got_data), NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
                  SOUP_TYPE_BUFFER);

  obj_signals[FINISHED] =
    g_signal_new ("finished",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (KmsHttpPostClass, finished), NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);

  /* Registers a private structure for an instantiatable type */
  g_type_class_add_private (klass, sizeof (KmsHttpPostPrivate) );
}

static void
kms_http_post_init (KmsHttpPost *self)
{
  self->priv = KMS_HTTP_POST_GET_PRIVATE (self);
}

KmsHttpPost *
kms_http_post_new ()
{
  KmsHttpPost *obj;

  obj = KMS_HTTP_POST (g_object_new (KMS_TYPE_HTTP_POST, NULL) );

  return obj;
}