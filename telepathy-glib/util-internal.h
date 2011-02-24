/*
 * util-internal.h - Headers for non-public telepathy-glib utility functions
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
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

#ifndef __TP_UTIL_INTERNAL_H__
#define __TP_UTIL_INTERNAL_H__

#include "config.h"

#include <glib.h>
#include <gio/gio.h>

#include <telepathy-glib/channel-request.h>

GArray *_tp_quark_array_copy (const GQuark *quarks) G_GNUC_WARN_UNUSED_RESULT;
void _tp_quark_array_merge (GArray *array, const GQuark *quarks, gssize n);

#ifdef HAVE_GIO_UNIX
GSocketAddress * _tp_create_temp_unix_socket (GSocketService *service,
    GError **error);
#endif /* HAVE_GIO_UNIX */

GList * _tp_create_channel_request_list (TpDBusDaemon *dbus,
    GHashTable *request_props);

/* Copied from wocky/wocky-utils.h */

#define _tp_implement_finish_void(source, tag) \
    if (g_simple_async_result_propagate_error (\
      G_SIMPLE_ASYNC_RESULT (result), error)) \
      return FALSE; \
    g_return_val_if_fail (g_simple_async_result_is_valid (result, \
            G_OBJECT(source), tag), \
        FALSE); \
    return TRUE;

#define _tp_implement_finish_copy_pointer(source, tag, copy_func, \
    out_param) \
    GSimpleAsyncResult *_simple; \
    _simple = (GSimpleAsyncResult *) result; \
    if (g_simple_async_result_propagate_error (_simple, error)) \
      return FALSE; \
    g_return_val_if_fail (g_simple_async_result_is_valid (result, \
            G_OBJECT (source), tag), \
        FALSE); \
    if (out_param != NULL) \
      *out_param = copy_func ( \
          g_simple_async_result_get_op_res_gpointer (_simple)); \
    return TRUE;

#define _tp_implement_finish_return_copy_pointer(source, tag, copy_func) \
    GSimpleAsyncResult *_simple; \
    _simple = (GSimpleAsyncResult *) result; \
    if (g_simple_async_result_propagate_error (_simple, error)) \
      return NULL; \
    g_return_val_if_fail (g_simple_async_result_is_valid (result, \
            G_OBJECT (source), tag), \
        NULL); \
    return copy_func (g_simple_async_result_get_op_res_gpointer (_simple));

#endif /* __TP_UTIL_INTERNAL_H__ */
