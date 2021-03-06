#include "config.h"

#include <stdio.h>

#include <glib.h>

#include <telepathy-glib/util.h>

void test_strv_contains (void);

void
test_strv_contains (void)
{
  const gchar * const strv[] = {
      "Pah",
      "Pah",
      "Pah-pah-pah",
      "Patrick!",
      NULL
  };

  g_assert (tp_strv_contains (strv, "Patrick!"));
  g_assert (!tp_strv_contains (strv, "Snakes!"));
}

static void
test_value_array_build (void)
{
  GValueArray *arr;
  const gchar *host = "badger.snakes";
  guint port = 128;
  gchar *host_out = NULL;
  guint port_out = 1234;

  arr = tp_value_array_build (2,
    G_TYPE_STRING, host,
    G_TYPE_UINT, port,
    G_TYPE_INVALID);

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  g_assert_cmpstr (g_value_get_string (g_value_array_get_nth (arr, 0)), ==,
      host);
  g_assert_cmpuint (g_value_get_uint (g_value_array_get_nth (arr, 1)), ==,
      port);
  G_GNUC_END_IGNORE_DEPRECATIONS

  tp_value_array_unpack (arr, 2,
      &host_out,
      &port_out);

  g_assert_cmpstr (host_out, ==, host);
  g_assert_cmpuint (port_out, ==, port);

  tp_value_array_free (arr);
}

static void
test_utf8_make_valid (void)
{
  gchar *tmp;
  gint i;
  struct {
      const gchar *source;
      const gchar *target;
  } test[] = {
    /* lifted directly from https://bugzilla.gnome.org/show_bug.cgi?id=610969
     * examples from http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt */

    /* greek 'kosme' */
    { "\xce\xba\xe1\xbd\xb9\xcf\x83\xce\xbc\xce\xb5", "\xce\xba\xe1\xbd\xb9\xcf\x83\xce\xbc\xce\xb5" },
    /* first sequence of each length */
    { "\x00", "\x00" },
    { "\xc2\x80", "\xc2\x80" },
    { "\xe0\xa0\x80", "\xe0\xa0\x80" },
    { "\xf0\x90\x80\x80", "\xf0\x90\x80\x80" },
    { "\xf8\x88\x80\x80\x80", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\xfc\x84\x80\x80\x80\x80", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    /* last sequence of each length */
    { "\x7f", "\x7f" },
    { "\xdf\xbf", "\xdf\xbf" },
#if !GLIB_CHECK_VERSION (2, 36, 0)
    { "\xef\xbf\xbf", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
    { "\xf7\xbf\xbf\xbf", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\xfb\xbf\xbf\xbf\xbf", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\xfd\xbf\xbf\xbf\xbf\xbf", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    /* other boundary conditions */
    { "\xed\x9f\xbf", "\xed\x9f\xbf" },
    { "\xee\x80\x80", "\xee\x80\x80" },
    { "\xef\xbf\xbd", "\xef\xbf\xbd" },
#if !GLIB_CHECK_VERSION (2, 36, 0)
    { "\xf4\x8f\xbf\xbf", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
    { "\xf4\x90\x80\x80", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    /* malformed sequences */
    /* continuation bytes */
    { "\x80", "\xef\xbf\xbd" },
    { "\xbf", "\xef\xbf\xbd" },
    { "\x80\xbf", "\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x80\xbf\x80", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x80\xbf\x80\xbf", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x80\xbf\x80\xbf\x80", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x80\xbf\x80\xbf\x80\xbf", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x80\xbf\x80\xbf\x80\xbf\x80", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    /* all possible continuation byte */
    { "\x80", "\xef\xbf\xbd" },
    { "\x81", "\xef\xbf\xbd" },
    { "\x82", "\xef\xbf\xbd" },
    { "\x83", "\xef\xbf\xbd" },
    { "\x84", "\xef\xbf\xbd" },
    { "\x85", "\xef\xbf\xbd" },
    { "\x86", "\xef\xbf\xbd" },
    { "\x87", "\xef\xbf\xbd" },
    { "\x88", "\xef\xbf\xbd" },
    { "\x89", "\xef\xbf\xbd" },
    { "\x8a", "\xef\xbf\xbd" },
    { "\x8b", "\xef\xbf\xbd" },
    { "\x8c", "\xef\xbf\xbd" },
    { "\x8d", "\xef\xbf\xbd" },
    { "\x8e", "\xef\xbf\xbd" },
    { "\x8f", "\xef\xbf\xbd" },
    { "\x90", "\xef\xbf\xbd" },
    { "\x91", "\xef\xbf\xbd" },
    { "\x92", "\xef\xbf\xbd" },
    { "\x93", "\xef\xbf\xbd" },
    { "\x94", "\xef\xbf\xbd" },
    { "\x95", "\xef\xbf\xbd" },
    { "\x96", "\xef\xbf\xbd" },
    { "\x97", "\xef\xbf\xbd" },
    { "\x98", "\xef\xbf\xbd" },
    { "\x99", "\xef\xbf\xbd" },
    { "\x9a", "\xef\xbf\xbd" },
    { "\x9b", "\xef\xbf\xbd" },
    { "\x9c", "\xef\xbf\xbd" },
    { "\x9d", "\xef\xbf\xbd" },
    { "\x9e", "\xef\xbf\xbd" },
    { "\x9f", "\xef\xbf\xbd" },
    { "\xa0", "\xef\xbf\xbd" },
    { "\xa1", "\xef\xbf\xbd" },
    { "\xa2", "\xef\xbf\xbd" },
    { "\xa3", "\xef\xbf\xbd" },
    { "\xa4", "\xef\xbf\xbd" },
    { "\xa5", "\xef\xbf\xbd" },
    { "\xa6", "\xef\xbf\xbd" },
    { "\xa7", "\xef\xbf\xbd" },
    { "\xa8", "\xef\xbf\xbd" },
    { "\xa9", "\xef\xbf\xbd" },
    { "\xaa", "\xef\xbf\xbd" },
    { "\xab", "\xef\xbf\xbd" },
    { "\xac", "\xef\xbf\xbd" },
    { "\xad", "\xef\xbf\xbd" },
    { "\xae", "\xef\xbf\xbd" },
    { "\xaf", "\xef\xbf\xbd" },
    { "\xb0", "\xef\xbf\xbd" },
    { "\xb1", "\xef\xbf\xbd" },
    { "\xb2", "\xef\xbf\xbd" },
    { "\xb3", "\xef\xbf\xbd" },
    { "\xb4", "\xef\xbf\xbd" },
    { "\xb5", "\xef\xbf\xbd" },
    { "\xb6", "\xef\xbf\xbd" },
    { "\xb7", "\xef\xbf\xbd" },
    { "\xb8", "\xef\xbf\xbd" },
    { "\xb9", "\xef\xbf\xbd" },
    { "\xba", "\xef\xbf\xbd" },
    { "\xbb", "\xef\xbf\xbd" },
    { "\xbc", "\xef\xbf\xbd" },
    { "\xbd", "\xef\xbf\xbd" },
    { "\xbe", "\xef\xbf\xbd" },
    { "\xbf", "\xef\xbf\xbd" },
    /* lone start characters */
    { "\xc0\x20", "\xef\xbf\xbd\x20" },
    { "\xc1\x20", "\xef\xbf\xbd\x20" },
    { "\xc2\x20", "\xef\xbf\xbd\x20" },
    { "\xc3\x20", "\xef\xbf\xbd\x20" },
    { "\xc4\x20", "\xef\xbf\xbd\x20" },
    { "\xc5\x20", "\xef\xbf\xbd\x20" },
    { "\xc6\x20", "\xef\xbf\xbd\x20" },
    { "\xc7\x20", "\xef\xbf\xbd\x20" },
    { "\xc8\x20", "\xef\xbf\xbd\x20" },
    { "\xc9\x20", "\xef\xbf\xbd\x20" },
    { "\xca\x20", "\xef\xbf\xbd\x20" },
    { "\xcb\x20", "\xef\xbf\xbd\x20" },
    { "\xcc\x20", "\xef\xbf\xbd\x20" },
    { "\xcd\x20", "\xef\xbf\xbd\x20" },
    { "\xce\x20", "\xef\xbf\xbd\x20" },
    { "\xcf\x20", "\xef\xbf\xbd\x20" },
    { "\xd0\x20", "\xef\xbf\xbd\x20" },
    { "\xd1\x20", "\xef\xbf\xbd\x20" },
    { "\xd2\x20", "\xef\xbf\xbd\x20" },
    { "\xd3\x20", "\xef\xbf\xbd\x20" },
    { "\xd4\x20", "\xef\xbf\xbd\x20" },
    { "\xd5\x20", "\xef\xbf\xbd\x20" },
    { "\xd6\x20", "\xef\xbf\xbd\x20" },
    { "\xd7\x20", "\xef\xbf\xbd\x20" },
    { "\xd8\x20", "\xef\xbf\xbd\x20" },
    { "\xd9\x20", "\xef\xbf\xbd\x20" },
    { "\xda\x20", "\xef\xbf\xbd\x20" },
    { "\xdb\x20", "\xef\xbf\xbd\x20" },
    { "\xdc\x20", "\xef\xbf\xbd\x20" },
    { "\xdd\x20", "\xef\xbf\xbd\x20" },
    { "\xde\x20", "\xef\xbf\xbd\x20" },
    { "\xdf\x20", "\xef\xbf\xbd\x20" },
    { "\xe0\x20", "\xef\xbf\xbd\x20" },
    { "\xe1\x20", "\xef\xbf\xbd\x20" },
    { "\xe2\x20", "\xef\xbf\xbd\x20" },
    { "\xe3\x20", "\xef\xbf\xbd\x20" },
    { "\xe4\x20", "\xef\xbf\xbd\x20" },
    { "\xe5\x20", "\xef\xbf\xbd\x20" },
    { "\xe6\x20", "\xef\xbf\xbd\x20" },
    { "\xe7\x20", "\xef\xbf\xbd\x20" },
    { "\xe8\x20", "\xef\xbf\xbd\x20" },
    { "\xe9\x20", "\xef\xbf\xbd\x20" },
    { "\xea\x20", "\xef\xbf\xbd\x20" },
    { "\xeb\x20", "\xef\xbf\xbd\x20" },
    { "\xec\x20", "\xef\xbf\xbd\x20" },
    { "\xed\x20", "\xef\xbf\xbd\x20" },
    { "\xee\x20", "\xef\xbf\xbd\x20" },
    { "\xef\x20", "\xef\xbf\xbd\x20" },
    { "\xf0\x20", "\xef\xbf\xbd\x20" },
    { "\xf1\x20", "\xef\xbf\xbd\x20" },
    { "\xf2\x20", "\xef\xbf\xbd\x20" },
    { "\xf3\x20", "\xef\xbf\xbd\x20" },
    { "\xf4\x20", "\xef\xbf\xbd\x20" },
    { "\xf5\x20", "\xef\xbf\xbd\x20" },
    { "\xf6\x20", "\xef\xbf\xbd\x20" },
    { "\xf7\x20", "\xef\xbf\xbd\x20" },
    { "\xf8\x20", "\xef\xbf\xbd\x20" },
    { "\xf9\x20", "\xef\xbf\xbd\x20" },
    { "\xfa\x20", "\xef\xbf\xbd\x20" },
    { "\xfb\x20", "\xef\xbf\xbd\x20" },
    { "\xfc\x20", "\xef\xbf\xbd\x20" },
    { "\xfd\x20", "\xef\xbf\xbd\x20" },
    /* missing continuation bytes */
    { "\x20\xc0", "\x20\xef\xbf\xbd" },
    { "\x20\xe0\x80", "\x20\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x20\xf0\x80\x80", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x20\xf8\x80\x80\x80", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x20\xfc\x80\x80\x80\x80", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x20\xdf", "\x20\xef\xbf\xbd" },
    { "\x20\xef\xbf", "\x20\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x20\xf7\xbf\xbf", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x20\xfb\xbf\xbf\xbf", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    { "\x20\xfd\xbf\xbf\xbf\xbf", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
    /* impossible bytes */
    { "\x20\xfe\x20", "\x20\xef\xbf\xbd\x20" },
    { "\x20\xff\x20", "\x20\xef\xbf\xbd\x20" },
    /* overlong sequences */
    { "\x20\xc0\xaf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xe0\x80\xaf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xf0\x80\x80\xaf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xf8\x80\x80\x80\xaf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xfc\x80\x80\x80\x80\xaf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xc1\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xe0\x9f\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xf0\x8f\xbf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xf8\x87\xbf\xbf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xfc\x83\xbf\xbf\xbf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xc0\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xe0\x80\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xf0\x80\x80\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xf8\x80\x80\x80\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xfc\x80\x80\x80\x80\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    /* illegal code positions */
    { "\x20\xed\xa0\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xad\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xae\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xaf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xb0\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xbe\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xbf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xa0\x80\xed\xb0\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xa0\x80\xed\xbf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xad\xbf\xed\xb0\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xad\xbf\xed\xbf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xae\x80\xed\xb0\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xae\x80\xed\xbf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xaf\xbf\xed\xb0\x80\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xed\xaf\xbf\xed\xbf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
#if !GLIB_CHECK_VERSION (2, 36, 0)
    { "\x20\xef\xbf\xbe\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
    { "\x20\xef\xbf\xbf\x20", "\x20\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\x20" },
#endif

    { NULL, NULL }
  };

  for (i = 0; test[i].source != NULL; i++)
    {
      tmp = tp_utf8_make_valid (test[i].source);
      g_assert (!tp_strdiff (tmp, test[i].target));
      g_free (tmp);
    }
}

int main (int argc, char **argv)
{
  GPtrArray *ptrarray;
  gchar *string;

  g_assert (!tp_strdiff (NULL, NULL));
  g_assert (tp_strdiff ("badger", NULL));
  g_assert (tp_strdiff (NULL, "badger"));
  g_assert (!tp_strdiff ("badger", "badger"));
  g_assert (tp_strdiff ("badger", "mushroom"));

  ptrarray = g_ptr_array_new ();
  g_ptr_array_add (ptrarray, GINT_TO_POINTER (23));
  g_ptr_array_add (ptrarray, GINT_TO_POINTER (42));
  g_assert (tp_g_ptr_array_contains (ptrarray, GINT_TO_POINTER (23)));
  g_assert (tp_g_ptr_array_contains (ptrarray, GINT_TO_POINTER (42)));
  g_assert (!tp_g_ptr_array_contains (ptrarray, GINT_TO_POINTER (666)));
  g_ptr_array_unref (ptrarray);

  string = tp_escape_as_identifier ("");
  g_assert (!tp_strdiff (string, "_"));
  g_free (string);

  string = tp_escape_as_identifier ("badger");
  g_assert (!tp_strdiff (string, "badger"));
  g_free (string);

  string = tp_escape_as_identifier ("0123abc_xyz\x01\xff");
  g_assert (!tp_strdiff (string, "_30123abc_5fxyz_01_ff"));
  g_free (string);

  string = tp_escape_as_identifier ("??");
  g_assert (!tp_strdiff (string,  "_c2_a9"));
  g_free (string);

  test_strv_contains ();

  test_value_array_build ();

  test_utf8_make_valid ();

  return 0;
}
