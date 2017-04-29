#include "bimage-gtk.h"

bimage* bimageDefaultImage = NULL;

bimage* bimageGetDefaultImage(){
    return bimageDefaultImage;
}

void bimageSetDefaultImage(bimage* im, bool free_){
    if (free_){
        bimageRelease(bimageDefaultImage);
    }
    bimageDefaultImage = im;
}

static void release (uint8_t* unused, void* data)
{
    bimageRelease((bimage*)data);
}

GdkPixbuf*
bimagePixbuf(bimage* im, bool owner)
{
    if (bimageTypeChannels(im->type) == BIMAGE_GRAY || bimageTypeDepth(im->type) != BIMAGE_U8){
        return NULL;
    }

    return gdk_pixbuf_new_from_data(
            im->data,
            GDK_COLORSPACE_RGB,
            bimageTypeChannels(im->type) > 3,
            bimageDepthSize(bimageTypeDepth(im->type)),
            im->width, im->height,
            im->width * bimageTypeChannels(im->type),
            owner ? release : NULL, im);
}

GtkWidget*
bimageGtkImage(bimage* im, bool owner)
{
    return gtk_image_new_from_pixbuf(bimagePixbuf(im, owner));
}

GtkWidget*
bimageGtkWindow(bimage* im, bool owner)
{
    GtkWidget* win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget* image = bimageGtkImage(im, owner);
    GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), image);
    gtk_container_add(GTK_CONTAINER(win), scroll);
    gtk_window_set_default_size(GTK_WINDOW(win), 800, 600);
    gtk_widget_show_all(win);
    return win;
}
