#include <systemtray/kde.h>
#include <cstdio>

int main(void)
{
    KDETrayIcon icon("test");
    icon.setIconName("mail-client");
    icon.setTitle("Mail Client Icon Test");
    
    // left click
    icon.onActivate([=](int x, int y) {
        printf("Activate %i %i\n", x, y);
    });
    // middle click
    icon.onSecondaryActivate([=](int x, int y) {
        printf("Secondary activate %i %i\n", x, y);
    });
    // right click
    icon.onContextMenu([=](int x, int y) {
        printf("Context menu %i %i\n", x, y);
    });
    // scroll over icon
    icon.onScroll([=](int offset, std::string orientation) {
        printf("Scroll %i %s\n", offset, orientation.c_str());
    });
    while(icon.process(10));
    
    return 0;
}