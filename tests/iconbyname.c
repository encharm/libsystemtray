#include <systemtray/kde.h>
#include <stdio.h>

static void handler(int x, int y, void* label) {
    printf("%s %i %i\n", (char*)label, x, y);
}

static void scroll_handler(int scroll, const char* orientation, void* label) {
    printf("%s %i %s\n", (char*)label, scroll, orientation);
}


int main(void)
{
    kdetrayicon* handle = kdetrayicon_create("test");
    kdetrayicon_set_icon_name(handle, "mail-client");
    kdetrayicon_set_title(handle, "Mail Client Icon Test");
    
    // left click
    kdetrayicon_on_activate(handle, handler, "Activate");

    // middle click
    kdetrayicon_on_secondary_activate(handle, handler, "Secondary activate");
    
    // right click
    kdetrayicon_on_context_menu(handle, handler, "Secondary activate");
    
    // scroll over icon
    kdetrayicon_on_scroll(handle, scroll_handler, "Scroll");
    
    while(kdetrayicon_process(handle, 10));
    
    return 0;
}