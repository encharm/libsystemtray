
#include "slimdbus.hpp"

#include <dlfcn.h>
#include <cassert>

namespace DBus {

    bool initialized = false;
    
    static class Initialize {
    public:
        Initialize() {

            
            auto handle = dlopen("libdbus-1.so", RTLD_LAZY);
            if(!handle)
                return;
            #define __libsystemtray_xstr(s) __libsystemtray_str(s)
            #define __libsystemtray_str(s) #s
            
            #undef dynamic_c_symbol
            #define dynamic_c_symbol(name) DBus::name = (decltype(DBus::name))dlsym(handle, "dbus_" __libsystemtray_xstr(name)); if(!DBus::name) return;
            dynamic_c_symbol(error_init)
            dynamic_c_symbol(error_free)
            dynamic_c_symbol(connection_close)
            dynamic_c_symbol(connection_unref)
            dynamic_c_symbol(bus_get_private)
            dynamic_c_symbol(connection_send)
            dynamic_c_symbol(connection_flush)
            dynamic_c_symbol(message_unref)
            dynamic_c_symbol(pending_call_set_notify)
            dynamic_c_symbol(pending_call_steal_reply)
            dynamic_c_symbol(message_get_path)
            dynamic_c_symbol(message_get_interface)
            dynamic_c_symbol(message_get_member)
            dynamic_c_symbol(connection_remove_filter)
            dynamic_c_symbol(message_new_signal)
            dynamic_c_symbol(connection_send_with_reply)
            dynamic_c_symbol(pending_call_unref)
            dynamic_c_symbol(connection_read_write)
            dynamic_c_symbol(connection_get_dispatch_status)
            dynamic_c_symbol(connection_dispatch)
            dynamic_c_symbol(message_iter_init_append)
            dynamic_c_symbol(connection_add_filter)
            dynamic_c_symbol(message_new_method_return)
            dynamic_c_symbol(message_iter_open_container)
            dynamic_c_symbol(message_iter_append_basic)
            dynamic_c_symbol(message_iter_close_container)
            dynamic_c_symbol(message_get_args)
            dynamic_c_symbol(bus_request_name)
            dynamic_c_symbol(bus_release_name)
            dynamic_c_symbol(message_new_method_call)
            DBus::initialized = true;
        }
    } initialize;
};

#undef dynamic_c_symbol
#define dynamic_c_symbol(name) decltype(&(::dbus_ ## name)) DBus:: name;
dynamic_c_symbol(error_init)
dynamic_c_symbol(error_free)
dynamic_c_symbol(connection_close)
dynamic_c_symbol(connection_unref)
dynamic_c_symbol(bus_get_private)
dynamic_c_symbol(connection_send)
dynamic_c_symbol(connection_flush)
dynamic_c_symbol(message_unref)
dynamic_c_symbol(pending_call_set_notify)
dynamic_c_symbol(pending_call_steal_reply)
dynamic_c_symbol(message_get_path)
dynamic_c_symbol(message_get_interface)
dynamic_c_symbol(message_get_member)
dynamic_c_symbol(connection_remove_filter)
dynamic_c_symbol(message_new_signal)
dynamic_c_symbol(connection_send_with_reply)
dynamic_c_symbol(pending_call_unref)
dynamic_c_symbol(connection_read_write)
dynamic_c_symbol(connection_get_dispatch_status)
dynamic_c_symbol(connection_dispatch)
dynamic_c_symbol(message_iter_init_append)
dynamic_c_symbol(connection_add_filter)
dynamic_c_symbol(message_new_method_return)
dynamic_c_symbol(message_iter_open_container)
dynamic_c_symbol(message_iter_append_basic)
dynamic_c_symbol(message_iter_close_container)
dynamic_c_symbol(message_get_args)
dynamic_c_symbol(bus_request_name)
dynamic_c_symbol(bus_release_name)
dynamic_c_symbol(message_new_method_call)