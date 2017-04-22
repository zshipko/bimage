#include "../bimage-view.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 2){
        puts("Not enough arguments");
        return 1;
    }

    gtk_init(&argc, &argv);

    bimage* im = bimageOpen(argv[1]);
    if (!im){
        puts("Invalid image");
        return 2;
    }

    GtkWidget* window = bimageGtkWindow(im, true);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show(window);
    gtk_main();

    return 0;
}
