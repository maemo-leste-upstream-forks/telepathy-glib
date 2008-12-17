#include <glib-object.h>
#include <dbus/dbus-glib.h>
#include <telepathy-glib/dbus-properties-mixin.h>

G_BEGIN_DECLS

/**
 * TestSvcWithProperties:
 *
 * Dummy typedef representing any implementation of this interface.
 */
typedef struct _TestSvcWithProperties TestSvcWithProperties;

/**
 * TestSvcWithPropertiesClass:
 *
 * The class of TestSvcWithProperties.
 */
typedef struct _TestSvcWithPropertiesClass TestSvcWithPropertiesClass;

GType test_svc_with_properties_get_type (void);
#define TEST_TYPE_SVC_WITH_PROPERTIES \
  (test_svc_with_properties_get_type ())
#define TEST_SVC_WITH_PROPERTIES(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), TEST_TYPE_SVC_WITH_PROPERTIES, TestSvcWithProperties))
#define TEST_IS_SVC_WITH_PROPERTIES(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), TEST_TYPE_SVC_WITH_PROPERTIES))
#define TEST_SVC_WITH_PROPERTIES_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE((obj), TEST_TYPE_SVC_WITH_PROPERTIES, TestSvcWithPropertiesClass))




G_END_DECLS
