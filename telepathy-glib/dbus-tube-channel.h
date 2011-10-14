/*
 * dbus-tube-channel.h - high level API for DBusTube channels
 *
 * Copyright (C) 2011 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __TP_DBUS_TUBE_CHANNEL_H__
#define __TP_DBUS_TUBE_CHANNEL_H__

#include <telepathy-glib/channel.h>

G_BEGIN_DECLS

#define TP_TYPE_DBUS_TUBE_CHANNEL (tp_dbus_tube_channel_get_type ())
#define TP_DBUS_TUBE_CHANNEL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TP_TYPE_DBUS_TUBE_CHANNEL, TpDBusTubeChannel))
#define TP_DBUS_TUBE_CHANNEL_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), TP_TYPE_DBUS_TUBE_CHANNEL, TpDBusTubeChannelClass))
#define TP_IS_DBUS_TUBE_CHANNEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TP_TYPE_DBUS_TUBE_CHANNEL))
#define TP_IS_DBUS_TUBE_CHANNEL_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), TP_TYPE_DBUS_TUBE_CHANNEL))
#define TP_DBUS_TUBE_CHANNEL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TP_TYPE_DBUS_TUBE_CHANNEL, TpDBusTubeChannelClass))

typedef struct _TpDBusTubeChannel TpDBusTubeChannel;
typedef struct _TpDBusTubeChannelClass TpDBusTubeChannelClass;
typedef struct _TpDBusTubeChannelPrivate TpDBusTubeChannelPrivate;

struct _TpDBusTubeChannel
{
  /*<private>*/
  TpChannel parent;
  TpDBusTubeChannelPrivate *priv;
};

struct _TpDBusTubeChannelClass
{
  /*<private>*/
  TpChannelClass parent_class;
  GCallback _padding[7];
};

GType tp_dbus_tube_channel_get_type (void);

const gchar * tp_dbus_tube_channel_get_service_name (TpDBusTubeChannel *self);

GHashTable * tp_dbus_tube_channel_get_parameters (TpDBusTubeChannel *self);

G_END_DECLS

#endif
