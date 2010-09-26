#include "hildon-im-plugin.h"
#include "hildon-im-ui.h"

#include <string.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <hildon/hildon.h>

#include <stdarg.h>
#include <stdio.h>

#include "common.h"

#define HILDON_IM_CELLWRITER_TYPE hildon_im_cellwriter_get_type()
#define HILDON_IM_CELLWRITER(obj) GTK_CHECK_CAST(obj, hildon_im_cellwriter_get_type(), HildonIMCellwriter)
#define HILDON_IM_CELLWRITER_CLASS(klass) \
        GTK_CHECK_CLASS_CAST(klass, hildon_im_cellwriter_get_type, \
                             HildonIMCellwriterClass)
#define HILDON_IS_IM_CELLWRITER(obj) \
        GTK_CHECK_TYPE(obj, hildon_im_cellwriter_get_type())
#define HILDON_IM_CELLWRITER_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HILDON_IM_CELLWRITER_TYPE,\
                                      HildonIMCellwriterPrivate))

typedef struct
{
  GtkContainerClass parent;
}
HildonIMCellwriterClass;

typedef struct
{
  GtkContainer parent;

}
HildonIMCellwriter;

typedef struct
{
  HildonIMUI *ui;
  GtkWidget *window;

}
HildonIMCellwriterPrivate;

HildonIMUI * ui = NULL;

static GType hildon_im_cellwriter_type = 0;
static GtkWidgetClass *parent_class = NULL;

GType hildon_im_cellwriter_get_type (void);
GtkWidget *hildon_im_cellwriter_new (HildonIMUI *kbd);

/*
 * HildonIMPlugin interface
 */

static void hildon_im_cellwriter_iface_init (HildonIMPluginIface *iface);

static void hildon_im_cellwriter_enable (HildonIMPlugin *plugin, gboolean init);
static void hildon_im_cellwriter_disable                   (HildonIMPlugin *plugin);
static void hildon_im_cellwriter_surrounding_received      (HildonIMPlugin *plugin,
                                                             const gchar *surrounding,
                                                             gint offset);
/*
 * GObject functions
 */
static void hildon_im_cellwriter_finalize     (GObject *obj);
static void hildon_im_cellwriter_get_property (GObject *object,
                                                guint prop_id,
                                                GValue *value,
                                                GParamSpec *pspec);
static void hildon_im_cellwriter_set_property (GObject *object,
                                                guint prop_id,
                                                const GValue *value,
                                                GParamSpec *pspec);

static void hildon_im_cellwriter_class_init (HildonIMCellwriterClass *klass);
static void hildon_im_cellwriter_init       (HildonIMCellwriter *self);

/*
 * Internal functions
 */
static void populate_window (HildonIMCellwriter *self);
static void cellwriter_init (HildonIMCellwriter *self);

/*
 * Logging functions
 */
#define LOG_FILE "/tmp/hildon-im-cellwriter.log"

int log_indent = 0;

void Log(char *format, ...){
	va_list args;
	static FILE *fp = NULL;

	va_start(args, format);
	
	if(!fp){
		fp = fopen(LOG_FILE, "a");
	}
	
        int i;
        for(i = 0; i < log_indent; i++)
          fprintf(fp, "  ");
	vfprintf(fp, format, args);
	fputc('\n', fp);
	fflush(fp);

	va_end(args);
}


/*
 * Module functions
 */

HildonIMPlugin*
module_create (HildonIMUI *keyboard)
{
  Log("module_create(%x)", keyboard); log_indent ++;
  HildonIMPlugin *plugin = HILDON_IM_PLUGIN (hildon_im_cellwriter_new (keyboard));
  log_indent --;
  Log("module_create = %x", plugin);
  return plugin;
}

void
module_exit(void)
{
  Log("module_exit()");
  /* empty */
}

void
module_init(GTypeModule *module)
{
  Log("module_init(%x)", module); log_indent ++;

  static const GTypeInfo type_info = {
    sizeof(HildonIMCellwriterClass),
    NULL, /* base_init */
    NULL, /* base_finalize */
    (GClassInitFunc) hildon_im_cellwriter_class_init,
    NULL, /* class_finalize */
    NULL, /* class_data */
    sizeof(HildonIMCellwriter),
    0, /* n_preallocs */
    (GInstanceInitFunc) hildon_im_cellwriter_init,
  };

  static const GInterfaceInfo plugin_info = {
    (GInterfaceInitFunc) hildon_im_cellwriter_iface_init,
    NULL, /* interface_finalize */
    NULL, /* interface_data */
  };

  hildon_im_cellwriter_type =
          g_type_module_register_type(module,
                                      GTK_TYPE_CONTAINER, "HildonIMCellwriter",
                                      &type_info,
                                      0);

  g_type_module_add_interface(module,
                              HILDON_IM_CELLWRITER_TYPE,
                              HILDON_IM_TYPE_PLUGIN,
                              &plugin_info);
  log_indent --;
}

/*
 * This is used to know the plugin's information when loading the module
 */
const HildonIMPluginInfo *
hildon_im_plugin_get_info(void)
{
  Log("hildon_im_plugin_get_info()"); log_indent ++;
  static const HildonIMPluginInfo info =
  {
    "HIM Cellwriter",                   /* description */
    "him_cellwriter",                   /* name */
    NULL,                               /* menu title */
    NULL,                               /* gettext domain */
    TRUE,                               /* visible in menu */
    FALSE,                              /* cached TODO make it TRUE */
    HILDON_IM_TYPE_FULLSCREEN,          /* UI type */
    HILDON_IM_GROUP_LATIN,              /* group */
    HILDON_IM_DEFAULT_PLUGIN_PRIORITY,  /* priority */
    NULL,                               /* special character plugin */
    NULL,                               /* help page */
    TRUE,                               /* disable common UI buttons */
    0,                                  /* plugin height */
    HILDON_IM_TRIGGER_FINGER            /* trigger */
  };

  log_indent --;

  return &info;
}

/*
 * This function returns the list of available languages supported
 * by the plugin.
 */
gchar **
hildon_im_plugin_get_available_languages (gboolean *free)
{
  Log("hildon_im_plugin_get_available_langauges()");
  static gchar *list[] = { "en_GB", NULL };
  *free = FALSE;

  return list;
}

GType
hildon_im_cellwriter_get_type (void)
{
  // Log("hildon_im_plugin_fkb_get_type()");
  return hildon_im_cellwriter_type;
}

/*
 * Implement the interface.
 */
static void
hildon_im_cellwriter_iface_init (HildonIMPluginIface *iface)
{
  Log("hildon_im_cellwriter_iface_init()");
  iface->enable = hildon_im_cellwriter_enable;
  iface->disable = hildon_im_cellwriter_disable;
  iface->surrounding_received = hildon_im_cellwriter_surrounding_received;
  // preedit_commited ?
}

static void
hildon_im_cellwriter_class_init (HildonIMCellwriterClass *klass)
{
  Log("hildon_im_cellwriter_class_init(%x)", klass); log_indent ++;
  GObjectClass *object_class;
  GtkObjectClass *gtk_object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  parent_class = g_type_class_peek_parent(klass);
  g_type_class_add_private(klass, sizeof(HildonIMCellwriterPrivate));

  object_class = G_OBJECT_CLASS(klass);
  gtk_object_class = GTK_OBJECT_CLASS(klass);
  widget_class = GTK_WIDGET_CLASS(klass);
  container_class = GTK_CONTAINER_CLASS(klass);

  object_class->set_property  = hildon_im_cellwriter_set_property;
  object_class->get_property  = hildon_im_cellwriter_get_property;
  object_class->finalize      = hildon_im_cellwriter_finalize;

  /* install properties and signals as needed */

  g_object_class_install_property(object_class, HILDON_IM_PROP_UI,
                                  g_param_spec_object (HILDON_IM_PROP_UI_DESCRIPTION,
                                                       HILDON_IM_PROP_UI_DESCRIPTION,
                                                       "UI that uses plugin",
                                                       HILDON_IM_TYPE_UI,
                                                       G_PARAM_READWRITE
                                                       | G_PARAM_CONSTRUCT_ONLY));

  log_indent --;
}

static void
hildon_im_cellwriter_init (HildonIMCellwriter *self)
{
  Log("hildon_im_cellwriter_init(%x)", self);

  HildonIMCellwriterPrivate *priv;
 
  g_return_if_fail (HILDON_IS_IM_CELLWRITER (self));
  priv = HILDON_IM_CELLWRITER_GET_PRIVATE(self);

  priv->window = NULL;
}

static void
hildon_im_cellwriter_finalize(GObject *obj)
{
  Log("hildon_im_cellwriter_finalize(%x)", obj);
  if (G_OBJECT_CLASS(parent_class)->finalize)
  {
    G_OBJECT_CLASS(parent_class)->finalize(obj);
  }
}

GtkWidget *
hildon_im_cellwriter_new (HildonIMUI *kbd)
{
  Log("hildon_im_cellwriter_new(%x)", kbd); log_indent ++;
  GtkWidget *widget =  g_object_new (HILDON_IM_CELLWRITER_TYPE,
                       HILDON_IM_PROP_UI_DESCRIPTION, kbd, NULL);
  log_indent --;
  return widget;
}

static void
hildon_im_cellwriter_get_property (GObject *object,
                                    guint prop_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  Log("hildon_im_cellwriter_get_property(%x, %d, %x, %x)", object, prop_id, value, pspec); log_indent ++;
  HildonIMCellwriterPrivate *priv;

  g_return_if_fail (HILDON_IS_IM_CELLWRITER(object));
  priv = HILDON_IM_CELLWRITER_GET_PRIVATE(object);

  switch (prop_id)
  {
    case HILDON_IM_PROP_UI:
      g_value_set_object(value, priv->ui);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
  log_indent ++;
}

static void
hildon_im_cellwriter_set_property (GObject *object,
                                    guint prop_id,
                                    const GValue *value,
                                    GParamSpec *pspec)
{
  Log("hildon_im_cellwriter_set_property(%x, %d, %x, %x)", object, prop_id, value, pspec); log_indent ++;
  HildonIMCellwriterPrivate *priv;

  g_return_if_fail (HILDON_IS_IM_CELLWRITER (object));
  priv = HILDON_IM_CELLWRITER_GET_PRIVATE(object);

  switch (prop_id)
  {
    case HILDON_IM_PROP_UI:
      ui = priv->ui = g_value_get_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
  log_indent --;
}

static void
hildon_im_cellwriter_enable (HildonIMPlugin *plugin, gboolean init)
{
  Log("hildon_im_cellwriter_enable(%x, %s)", plugin, init ? "true" : "false" ); log_indent ++;

  HildonIMCellwriter *self;
  HildonIMCellwriterPrivate *priv;
  gboolean window_is_new;

  g_return_if_fail (HILDON_IS_IM_CELLWRITER (plugin));
  self = HILDON_IM_CELLWRITER(plugin);
  priv = HILDON_IM_CELLWRITER_GET_PRIVATE(self);

  window_is_new = (priv->window == NULL);

  if (window_is_new)
  {
    priv->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  }

  gtk_window_set_type_hint(GTK_WINDOW(priv->window), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_decorated(GTK_WINDOW(priv->window), FALSE);
  // hildon_gtk_window_set_portrait_flags (GTK_WINDOW(priv->window), HILDON_PORTRAIT_MODE_SUPPORT);

  hildon_im_ui_send_communication_message(priv->ui, HILDON_IM_CONTEXT_REQUEST_SURROUNDING);

  gtk_window_fullscreen(GTK_WINDOW(priv->window));
  gtk_widget_show_all(priv->window);

  gdk_window_set_transient_for(GTK_WIDGET(priv->window)->window, gtk_widget_get_root_window(GTK_WIDGET(priv->window)));

  if (window_is_new)
  {
    cellwriter_init(self);
    populate_window(self);
    gtk_widget_show_all(priv->window);
  }

  window_show();

  log_indent --;
}

static void
hildon_im_cellwriter_disable (HildonIMPlugin *plugin)
{
  Log("hildon_im_cellwriter_disable(%x)", plugin); log_indent ++;

  HildonIMCellwriter *self;
  HildonIMCellwriterPrivate *priv;

  g_return_if_fail (HILDON_IS_IM_CELLWRITER(plugin));
  self = HILDON_IM_CELLWRITER(plugin);
  priv = HILDON_IM_CELLWRITER_GET_PRIVATE(self);

  if (GTK_WIDGET_VISIBLE(GTK_WIDGET(priv->window)))
  {
    gtk_widget_hide(GTK_WIDGET(priv->window));
  }

  log_indent --;
}

void cell_widget_load_string(const gchar *str);

static void
hildon_im_cellwriter_surrounding_received(HildonIMPlugin *plugin,
                                           const gchar *surrounding,
                                           gint offset)
{
  Log("hildon_im_cellwriter_surrounding_received(%x, '%s', %d)", plugin, surrounding, offset); log_indent ++;

  HildonIMCellwriter *self;
  HildonIMCellwriterPrivate *priv;

  if(hildon_im_ui_get_commit_mode(ui) == HILDON_IM_COMMIT_REDIRECT)
    cell_widget_load_string(surrounding);

  log_indent --;
}

static void
close_fkb (GtkButton *button, gpointer user_data)
{
  HildonIMCellwriter *self;
  HildonIMCellwriterPrivate *priv;

  g_return_if_fail (HILDON_IS_IM_CELLWRITER (user_data));
  self = HILDON_IM_CELLWRITER(user_data);
  priv = HILDON_IM_CELLWRITER_GET_PRIVATE(self);

  hildon_im_ui_restore_previous_mode(priv->ui);
}

void read_profile();

static void cellwriter_init(HildonIMCellwriter *self){

  HildonIMCellwriterPrivate *priv;
  priv = HILDON_IM_CELLWRITER_GET_PRIVATE(self);

  key_event_init(priv->ui);
  recognize_init();
  read_profile();
  update_enabled_samples();
  
}

static void
populate_window (HildonIMCellwriter *self)
{
  Log("populate_window(%x)", self); log_indent ++;

  HildonIMCellwriterPrivate *priv;
  GtkWidget *parea, *hbox;
  gint screen_width = gdk_screen_width();

  priv = HILDON_IM_CELLWRITER_GET_PRIVATE(self);

  window_create(priv->window);

  log_indent --;
}


/*
typedef enum
{
  HILDON_IM_COMMIT_DIRECT,
  HILDON_IM_COMMIT_REDIRECT,
  HILDON_IM_COMMIT_SURROUNDING,
  HILDON_IM_COMMIT_BUFFERED,
  HILDON_IM_COMMIT_PREEDIT
} HildonIMCommitMode;

hildon_im_ui_Send_event(ui, Window window, XEvent event)

*/
