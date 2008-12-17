#include "_gen/svc.h"


static const DBusGObjectInfo _test_svc_with_properties_object_info;

struct _TestSvcWithPropertiesClass {
    GTypeInterface parent_class;
};

static void test_svc_with_properties_base_init (gpointer klass);

GType
test_svc_with_properties_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (type == 0))
    {
      static const GTypeInfo info = {
        sizeof (TestSvcWithPropertiesClass),
        test_svc_with_properties_base_init, /* base_init */
        NULL, /* base_finalize */
        NULL, /* class_init */
        NULL, /* class_finalize */
        NULL, /* class_data */
        0,
        0, /* n_preallocs */
        NULL /* instance_init */
      };

      type = g_type_register_static (G_TYPE_INTERFACE,
          "TestSvcWithProperties", &info, 0);
    }

  return type;
}

static inline void
test_svc_with_properties_base_init_once (gpointer klass)
{
  static TpDBusPropertiesMixinPropInfo properties[4] = {
      { 0, TP_DBUS_PROPERTIES_MIXIN_FLAG_READ, "u", 0, NULL, NULL }, /* ReadOnly */
      { 0, TP_DBUS_PROPERTIES_MIXIN_FLAG_WRITE, "u", 0, NULL, NULL }, /* WriteOnly */
      { 0, TP_DBUS_PROPERTIES_MIXIN_FLAG_READ | TP_DBUS_PROPERTIES_MIXIN_FLAG_WRITE, "u", 0, NULL, NULL }, /* ReadWrite */
      { 0, 0, NULL, 0, NULL, NULL }
  };
  static TpDBusPropertiesMixinIfaceInfo interface =
      { 0, properties, NULL, NULL };

  interface.dbus_interface = g_quark_from_static_string ("com.example.WithProperties");
  properties[0].name = g_quark_from_static_string ("ReadOnly");
  properties[0].type = G_TYPE_UINT;
  properties[1].name = g_quark_from_static_string ("WriteOnly");
  properties[1].type = G_TYPE_UINT;
  properties[2].name = g_quark_from_static_string ("ReadWrite");
  properties[2].type = G_TYPE_UINT;
  tp_svc_interface_set_dbus_properties_info (TEST_TYPE_SVC_WITH_PROPERTIES, &interface);

  dbus_g_object_type_install_info (test_svc_with_properties_get_type (),
      &_test_svc_with_properties_object_info);
}
static void
test_svc_with_properties_base_init (gpointer klass)
{
  static gboolean initialized = FALSE;

  if (!initialized)
    {
      initialized = TRUE;
      test_svc_with_properties_base_init_once (klass);
    }
}
static const DBusGMethodInfo _test_svc_with_properties_methods[] = {
};

static const DBusGObjectInfo _test_svc_with_properties_object_info = {
  0,
  _test_svc_with_properties_methods,
  0,
"\0",
"\0\0",
"\0\0",
};


