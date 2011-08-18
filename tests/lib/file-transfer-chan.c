/*
 * file-transfer-chan.c - Simple file transfer channel
 *
 * Copyright (C) 2010-2011 Morten Mjelva <morten.mjelva@gmail.com>
 * Copyright (C) 2010-2011 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * Copying and distribution of this file, with or without modification,
 * are permitted in any medium without royalty provided the copyright
 * notice and this notice are preserved.
 */

#include "file-transfer-chan.h"
#include "util.h"
#include "debug.h"

#include <telepathy-glib/telepathy-glib.h>
#include <telepathy-glib/channel-iface.h>
#include <telepathy-glib/svc-channel.h>
#include <telepathy-glib/gnio-util.h>

#include <gio/gunixsocketaddress.h>
#include <gio/gunixconnection.h>

#include <glib/gstdio.h>

static void file_transfer_iface_init (gpointer iface, gpointer data);

G_DEFINE_TYPE_WITH_CODE (TpTestsFileTransferChannel,
    tp_tests_file_transfer_channel,
    TP_TYPE_BASE_CHANNEL,
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CHANNEL_TYPE_FILE_TRANSFER,
      file_transfer_iface_init);
    )

static const char *
tp_tests_file_transfer_channel_interfaces[] = { NULL };

enum /* properties */
{
  PROP_AVAILABLE_SOCKET_TYPES = 1,
  PROP_CONTENT_TYPE,
  PROP_CONTENT_HASH,
  PROP_CONTENT_HASH_TYPE,
  PROP_DATE,
  PROP_DESCRIPTION,
  PROP_FILENAME,
  PROP_INITIAL_OFFSET,
  PROP_SIZE,
  PROP_STATE,
  PROP_TRANSFERRED_BYTES,
  PROP_URI,
  N_PROPS,
};

struct _TpTestsFileTransferChannelPrivate {
    /* Exposed properties */
    gchar *content_type;
    guint64 date;
    gchar *description;
    gchar *filename;
    guint64 size;
    TpFileTransferState state;
    guint64 transferred_bytes;
    gchar *uri;

    /* Hidden properties */
    TpFileHashType content_hash_type;
    gchar *content_hash;
    GHashTable *available_socket_types;
    gint64 initial_offset;

    /* Accepting side */
    GSocketService *service;
    GValue *access_control_param;

    /* Offering side */
    TpSocketAddressType address_type;
    GValue *address;
    gchar *unix_address;
    guint connection_id;
    TpSocketAccessControl access_control;
};

static void
tp_tests_file_transfer_channel_init (TpTestsFileTransferChannel *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE ((self),
      TP_TESTS_TYPE_FILE_TRANSFER_CHANNEL, TpTestsFileTransferChannelPrivate);
}

static void
create_available_socket_types (TpTestsFileTransferChannel *self)
{
  TpSocketAccessControl access_control;
  GArray *unix_tab;

  g_assert (self->priv->available_socket_types == NULL);
  self->priv->available_socket_types = g_hash_table_new_full (NULL, NULL,
      NULL, _tp_destroy_socket_control_list);

  /* SocketAddressTypeUnix */
  unix_tab = g_array_sized_new (FALSE, FALSE, sizeof (TpSocketAccessControl),
      1);
  access_control = TP_SOCKET_ACCESS_CONTROL_LOCALHOST;
  g_array_append_val (unix_tab, access_control);

  g_hash_table_insert (self->priv->available_socket_types,
      GUINT_TO_POINTER (TP_SOCKET_ADDRESS_TYPE_UNIX), unix_tab);
}

static GObject *
constructor (GType type,
    guint n_props,
    GObjectConstructParam *props)
{
  GObject *object =
    G_OBJECT_CLASS (tp_tests_file_transfer_channel_parent_class)->constructor
    (type, n_props, props);
  TpTestsFileTransferChannel *self = TP_TESTS_FILE_TRANSFER_CHANNEL (object);

  self->priv->state = TP_FILE_TRANSFER_STATE_PENDING;

  if (self->priv->available_socket_types == NULL)
    create_available_socket_types (self);

  tp_base_channel_register (TP_BASE_CHANNEL (self));

  return object;
}

static void
dispose (GObject *object)
{
  TpTestsFileTransferChannel *self = TP_TESTS_FILE_TRANSFER_CHANNEL (object);

  g_free (self->priv->content_hash);
  g_free (self->priv->content_type);
  g_free (self->priv->description);
  g_free (self->priv->filename);
  g_free (self->priv->uri);

  tp_clear_pointer (&self->priv->address, tp_g_value_slice_free);
  tp_clear_pointer (&self->priv->available_socket_types, g_hash_table_unref);
  tp_clear_pointer (&self->priv->access_control_param, tp_g_value_slice_free);

  if (self->priv->unix_address != NULL)
    g_unlink (self->priv->unix_address);

  tp_clear_pointer (&self->priv->unix_address, g_free);

  ((GObjectClass *) tp_tests_file_transfer_channel_parent_class)->dispose (
      object);
}

static void
get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  TpTestsFileTransferChannel *self = (TpTestsFileTransferChannel *) object;

  switch (property_id)
    {
      case PROP_AVAILABLE_SOCKET_TYPES:
        g_value_set_boxed (value, self->priv->available_socket_types);
        break;

      case PROP_CONTENT_HASH:
        g_value_set_string (value, self->priv->content_hash);

      case PROP_CONTENT_HASH_TYPE:
        g_value_set_uint (value, self->priv->content_hash_type);
        break;

      case PROP_CONTENT_TYPE:
        g_value_set_string (value, self->priv->content_type);
        break;

      case PROP_DATE:
        g_value_set_uint64 (value, self->priv->date);
        break;

      case PROP_DESCRIPTION:
        g_value_set_string (value, self->priv->description);
        break;

      case PROP_FILENAME:
        g_value_set_string (value, self->priv->filename);
        break;

      case PROP_INITIAL_OFFSET:
        g_value_set_uint64 (value, self->priv->initial_offset);
        break;

      case PROP_SIZE:
        g_value_set_uint64 (value, self->priv->size);
        break;

      case PROP_STATE:
        g_value_set_uint (value, self->priv->state);
        break;

      case PROP_TRANSFERRED_BYTES:
        g_value_set_uint64 (value, self->priv->transferred_bytes);
        break;

      case PROP_URI:
        g_value_set_string (value, self->priv->uri);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  TpTestsFileTransferChannel *self = (TpTestsFileTransferChannel *) object;

  switch (property_id)
    {
      case PROP_AVAILABLE_SOCKET_TYPES:
        self->priv->available_socket_types = g_value_dup_boxed (value);
        break;

      case PROP_CONTENT_HASH:
        self->priv->content_hash = g_value_dup_string (value);
        break;

      case PROP_CONTENT_HASH_TYPE:
        break;

      case PROP_CONTENT_TYPE:
        self->priv->content_type = g_value_dup_string (value);
        break;

      case PROP_DATE:
        self->priv->date = g_value_get_uint64 (value);
        break;

      case PROP_DESCRIPTION:
        self->priv->description = g_value_dup_string (value);
        break;

      case PROP_FILENAME:
        self->priv->filename = g_value_dup_string (value);
        break;

      case PROP_INITIAL_OFFSET:
        self->priv->initial_offset = g_value_get_uint64 (value);
        break;

      case PROP_SIZE:
        self->priv->size = g_value_get_uint64 (value);
        break;

      case PROP_STATE:
        self->priv->state = g_value_get_uint (value);
        break;

      case PROP_TRANSFERRED_BYTES:
        self->priv->transferred_bytes = g_value_get_uint64 (value);
        break;

      case PROP_URI:
        self->priv->uri = g_value_dup_string (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
channel_close (TpBaseChannel *self)
{
  g_print ("entered channel_close");
  tp_base_channel_destroyed (self);
}

static void
fill_immutable_properties (TpBaseChannel *self,
    GHashTable *properties)
{
  TpBaseChannelClass *klass = TP_BASE_CHANNEL_CLASS (
      tp_tests_file_transfer_channel_parent_class);

  klass->fill_immutable_properties (self, properties);

  tp_dbus_properties_mixin_fill_properties_hash (
      G_OBJECT (self), properties,
      TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER, "AvailableSocketTypes",
      TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER, "ContentType",
      TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER, "Filename",
      TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER, "Size",
      TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER, "Description",
      TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER, "Date",
      NULL);

  /* URI is immutable only for outgoing transfers */
  if (tp_base_channel_is_requested (self))
    {
      tp_dbus_properties_mixin_fill_properties_hash (G_OBJECT (self),
          properties,
          TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER, "URI", NULL);
    }
}

static void
tp_tests_file_transfer_channel_class_init (
    TpTestsFileTransferChannelClass *klass)
{
  GObjectClass *object_class = (GObjectClass *) klass;
  TpBaseChannelClass *base_class = TP_BASE_CHANNEL_CLASS (klass);
  GParamSpec *param_spec;

  static TpDBusPropertiesMixinPropImpl file_transfer_props[] = {
      { "AvailableSocketTypes", "available-socket-types", NULL },
      { "ContentType", "content-type", NULL },
      { "Date", "date", NULL },
      { "Description", "description", NULL },
      { "Filename", "filename", NULL },
      { "Size", "size", NULL },
      { "State", "state", NULL },
      { "TransferredBytes", "transferred-bytes", NULL },
      { "URI", "uri", NULL },
      { NULL }
  };

  object_class->constructor = constructor;
  object_class->get_property = get_property;
  object_class->set_property = set_property;
  object_class->dispose = dispose;

  base_class->channel_type = TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER;
  base_class->target_handle_type = TP_HANDLE_TYPE_CONTACT;
  base_class->interfaces = tp_tests_file_transfer_channel_interfaces;

  base_class->close = channel_close;
  base_class->fill_immutable_properties = fill_immutable_properties;

  tp_text_mixin_class_init (object_class,
      G_STRUCT_OFFSET (TpTestsFileTransferChannelClass, text_class));

  param_spec = g_param_spec_boxed ("available-socket-types",
      "AvailableSocketTypes",
      "The AvailableSocketTypes property of this channel",
      TP_HASH_TYPE_SUPPORTED_SOCKET_MAP,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_AVAILABLE_SOCKET_TYPES,
      param_spec);

  param_spec = g_param_spec_string ("content-type",
      "ContentType",
      "The ContentType property of this channel",
      NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_CONTENT_TYPE,
      param_spec);

  param_spec = g_param_spec_uint64 ("date",
      "Date",
      "The Date property of this channel",
      0, G_MAXUINT64, 0,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_DATE,
      param_spec);

  param_spec = g_param_spec_string ("description",
      "Description",
      "The Description property of this channel",
      NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_DESCRIPTION,
      param_spec);

  param_spec = g_param_spec_string ("filename",
      "Filename",
      "The Filename property of this channel",
      NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_FILENAME,
      param_spec);

  param_spec = g_param_spec_uint64 ("initial-offset",
      "InitialOffset",
      "The InitialOffset property of this channel",
      0, G_MAXUINT64, 0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_INITIAL_OFFSET,
      param_spec);

  param_spec = g_param_spec_uint64 ("size",
      "Size",
      "The Size property of this channel",
      0, G_MAXUINT64, 0,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_SIZE,
      param_spec);

  param_spec = g_param_spec_uint ("state",
      "State",
      "The State property of this channel",
      0, NUM_TP_FILE_TRANSFER_STATES, TP_FILE_TRANSFER_STATE_NONE,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_STATE,
      param_spec);

  param_spec = g_param_spec_uint64 ("transferred-bytes",
      "TransferredBytes",
      "The TransferredBytes property of this channel",
      0, G_MAXUINT64, 0,
      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_TRANSFERRED_BYTES,
      param_spec);

  param_spec = g_param_spec_string ("uri",
      "URI",
      "The URI property of this channel",
      NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_URI,
      param_spec);

  tp_dbus_properties_mixin_implement_interface (object_class,
      TP_IFACE_QUARK_CHANNEL_TYPE_FILE_TRANSFER,
      tp_dbus_properties_mixin_getter_gobject_properties, NULL,
      file_transfer_props);

  g_type_class_add_private (object_class,
      sizeof (TpTestsFileTransferChannelPrivate));
}

static void
file_transfer_iface_init (gpointer iface, gpointer data)
{
#if 0
  TpSvcChannelTypeFileTransferClass *klass = iface;

#define IMPLEMENT(x) tp_svc_channel_type_file_transfer_implement_##x (klass, \
    file_transfer_##x)
#undef IMPLEMENT
#endif
}

/* Return the address of the file transfer's socket */
GSocketAddress *
tp_tests_file_transfer_channel_get_server_address (
    TpTestsFileTransferChannel *self)
{
  GSocketAddress *address;
  GError *error = NULL;

  g_assert (self->priv->address != NULL);

  address = tp_g_socket_address_from_variant (self->priv->address_type,
      self->priv->address, &error);

  if (error != NULL)
    {
      g_printf ("%s\n", error->message);
    }

  return address;
}
