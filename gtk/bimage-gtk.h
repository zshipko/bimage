#ifndef __BIMAGE_GTK_HEADER_GUARD
#define __BIMAGE_GTK_HEADER_GUARD

#include <bimage.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <stdbool.h>

bimage* bimageGetDefaultImage();
void bimageSetDefaultImage(bimage*, bool);

GdkPixbuf*
bimagePixbuf(bimage* im, bool owner);

GtkWidget*
bimageGtkImage(bimage* im, bool owner);

GtkWidget*
bimageGtkWindow(bimage* im, bool owner);

#endif
