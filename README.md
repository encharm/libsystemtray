# libsystemtray

Goals of this library:
* Reduce the complexity of creating system tray icons
* Have minimal link dependencies
* Ship both C and C++ API
* Allow all of the features of native platform
* Friendly license, MIT **pull requests are welcome**
* Can be used and/or bundled in commercial software
* Ability to copy and paste code from this repo and make stuff work :)

Temporary limitations:
* KDE at first only. **pull requests are welcome**

Future:
* libappindicator or native gnome/ubuntu dbus
* Windows?
* Mac OSX?
* Common API for all backends

## Installation
Prerequisits:

```
sudo apt install cmake
sudo apt install pkgconf
sudo apt-get install libdbus-1-dev
```

Building:

```
git clone https://github.com/encharm/libsystemtray.git
cd libsystemtray && mkdir build && cd build
cmake ..
make
sudo make install
```

## Example usage for KDE
Note, the examples set icon from name, but you can also use:
```c++
KDETrayIcon::setIconPixmap(int w, int h, const uint8_t* data);
```
or the equivalent in C API 
```c++
void kdetrayicon_set_overlay_icon_pixmap(kdetrayicon* handle, int w, int h, const uint8_t* data);
```
The `data` needs to be a in 32-bit RGBA format.

### C++11 example
As per `tests/iconbyname.cpp`:
```c++
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
```

### C example
```c++
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
```

## License

Copyright (c) 2014 [Code Charm Ltd](http://codecharm.co.uk)

Licensed under the MIT license, see `LICENSE` for details.
