#pragma once

extern "C" {
#include <dbus/dbus.h>
}

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <cassert>
#include <unordered_map>
#include <tuple>
#include <vector>

/// getpid
#include <sys/types.h>
#include <unistd.h>
#include <cstring> // strdup

namespace DBus {
    
    extern bool initialized;
    
//    decltype(::dbus_connection_close) dbus_connection_close;
#define dynamic_c_symbol(name) extern decltype(&(::dbus_ ## name)) name;
    
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
    dynamic_c_symbol(message_iter_get_arg_type)
    dynamic_c_symbol(message_iter_next)
    dynamic_c_symbol(message_iter_init)
    dynamic_c_symbol(message_iter_get_basic)
    dynamic_c_symbol(message_iter_recurse)
    dynamic_c_symbol(message_get_type)
    dynamic_c_symbol(message_get_sender)
    dynamic_c_symbol(message_get_destination)
    dynamic_c_symbol(message_get_args)
    dynamic_c_symbol(message_get_error_name)
    dynamic_c_symbol(bus_request_name)
    dynamic_c_symbol(bus_release_name)
    dynamic_c_symbol(message_new_method_call)
    
    void debug_message(DBusMessage* msg);
    
    template <typename E>
    typename std::underlying_type<E>::type to_underlying(E e) {
        return static_cast<typename std::underlying_type<E>::type>(e);
    }
    
    struct Error : public DBusError {
        Error() {
            DBus::error_init(this);
        }
        ~Error() {
            if(message) {
                fprintf (stderr, "an error occurred: %s\n", message);
            }
            DBus::error_free(this);
        }
    };
  
    enum class HandlerResult {
        Handled = DBUS_HANDLER_RESULT_HANDLED,
        NotYetHandled = DBUS_HANDLER_RESULT_NOT_YET_HANDLED
    };
    
    class Connection {
    public:
        DBusConnection* rawPtr = nullptr;
        
        Connection(DBusConnection* _connection) : rawPtr(_connection) {
            
        }
        ~Connection() {
            DBus::connection_close(rawPtr);
            DBus::connection_unref(rawPtr);
        }
        typedef std::shared_ptr<Connection> Ptr;
        
        static Ptr Get(DBusBusType type = DBUS_BUS_SESSION) {
            DBus::Error error;
            auto connection = Ptr(new Connection(DBus::bus_get_private(type, &error)));
            
            return connection;
        }
        
        typedef std::function<void(DBusMessage*)> ReplyFunc;
        
        bool send(DBusMessage* message) {
            dbus_uint32_t serial = 0;
            auto status = DBus::connection_send(rawPtr, message, &serial);
            DBus::connection_flush(rawPtr);
            DBus::message_unref(message);
            return status;
        }
        
        bool sendWithReply(DBusMessage* message, ReplyFunc func) {
            DBusPendingCall* call;
            bool status = DBus::connection_send_with_reply(rawPtr, message, &call, 1000);
            DBus::message_unref(message);
            
            bool callSetStatus = DBus::pending_call_set_notify(call, [](DBusPendingCall* pending, void* user_data) {
                auto reply = DBus::pending_call_steal_reply(pending);
                (*((ReplyFunc*)user_data))(reply);
                
                DBus::pending_call_unref(pending);
            }, new ReplyFunc(func)     , [](void* user_data) {                
                delete ((ReplyFunc*)user_data);
            });
            
            return callSetStatus;
        }

        typedef std::function<HandlerResult(DBusMessage*)> FilterFunc;
        
        typedef std::pair<DBusHandleMessageFunction, void*> FilterHandle;
        FilterHandle addFilter(std::string path, std::string interface, std::string member, FilterFunc func) {
            return addFilter([=](DBusMessage* msg) {
                if( strcmp(DBus::message_get_path(msg), path.c_str()) == 0
                    && strcmp(DBus::message_get_interface(msg), interface.c_str()) == 0
                    && strcmp(DBus::message_get_member(msg), member.c_str()) == 0
                ) {
                    return func(msg);
                }
                return HandlerResult::NotYetHandled;
            });
        }
        
        FilterHandle addFilter(std::string path, std::string interface, FilterFunc func) {
            return addFilter([=](DBusMessage* msg) {
                if( strcmp(DBus::message_get_path(msg), path.c_str()) == 0
                    && strcmp(DBus::message_get_interface(msg), interface.c_str()) == 0
                ) {
                    return func(msg);
                }
                return HandlerResult::NotYetHandled;
            });
        }
        
        FilterHandle addFilter(std::string path, FilterFunc func) {
            return addFilter([=](DBusMessage* msg) {
                if( strcmp(DBus::message_get_path(msg), path.c_str()) == 0) {
                    return func(msg);
                }
                return HandlerResult::NotYetHandled;
            });
        }
        
        
        
        FilterHandle addFilter(FilterFunc func) {

            auto handleMessageFunc = [](DBusConnection *connection, DBusMessage *message, void* user_data) {
                return (DBusHandlerResult)to_underlying(
                    (*((FilterFunc*)user_data))(message) );
            };
            
            void* userData = new FilterFunc(func);
            DBus::connection_add_filter(rawPtr, handleMessageFunc, userData, [](void* user_data) {                
                delete ((FilterFunc*)user_data);
            });
            return FilterHandle(handleMessageFunc, userData);
        }
        
        void removeFilter(FilterHandle handle) {
            DBus::connection_remove_filter(rawPtr, handle.first, handle.second);
        }
        
        bool readWriteDispatch(int timeout = -1) {
            bool readWriteStatus = DBus::connection_read_write(rawPtr, timeout);
            for(DBusDispatchStatus status = DBus::connection_get_dispatch_status(rawPtr);
                status == DBUS_DISPATCH_DATA_REMAINS; status = DBus::connection_dispatch(rawPtr)) {
            }
            return readWriteStatus;
        }
        
    };
        
    class Interface {
        typedef std::function<void(DBusMessage*, DBusMessageIter&)> GetterFunc;
        typedef std::function<void(DBusMessage*)> SetterFunc;
        typedef std::tuple<std::string, GetterFunc, SetterFunc> GetterSetterEntry;
        
        std::unordered_map<std::string, GetterSetterEntry> properties;
        std::unordered_map<std::string, GetterFunc> methods;
        
        std::vector<Connection::FilterHandle> filterHandles;
                
    protected:
        const std::string path;
        const std::string interface;
        Connection::Ptr connection;
        
    public:
        void emitSignal(std::string name, std::function<void(DBusMessageIter&)> argsFunc = nullptr) {
            DBusMessageIter args;
            auto signal = DBus::message_new_signal(path.c_str(), interface.c_str(), name.c_str());
            DBus::message_iter_init_append(signal, &args);
            if(argsFunc) {
                argsFunc(args);
            }
            connection->send(signal);
        }
        
        Interface(std::string _path, std::string _interface, std::string baseXml, std::string xml, Connection::Ptr& _connection) : path(_path), interface(_interface), connection(_connection) {
            filterHandles.push_back(connection->addFilter(path, interface, [=](DBusMessage* msg) {                
                auto method = DBus::message_get_member(msg);
                auto methodIter = methods.find(method);
                if(methodIter == methods.end()) {
                    return DBus::HandlerResult::NotYetHandled;
                }
                {
                    auto message = DBus::message_new_method_return(msg);
                    DBusMessageIter args;
                    DBus::message_iter_init_append(message, &args);
                    methodIter->second(msg, args);
                    connection->send(message);
                }                
                
                return DBus::HandlerResult::Handled;
            }));
            
            
            filterHandles.push_back(connection->addFilter(path, "org.freedesktop.DBus.Properties", "GetAll",
            [=](DBusMessage* msg) {
                auto message = DBus::message_new_method_return(msg);
                DBusMessageIter args;
                DBus::message_iter_init_append(message, &args);
                        
                DBusMessageIter dict;
                DBus::message_iter_open_container(&args, DBUS_TYPE_ARRAY,
                    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                    DBUS_TYPE_STRING_AS_STRING
                    DBUS_TYPE_VARIANT_AS_STRING
                    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
                    &dict);
                
                
                for(auto& propertyEntry: properties) {
                    DBusMessageIter dictEntry;
                    DBus::message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &dictEntry);
                    const char* key = propertyEntry.first.c_str();
                    DBus::message_iter_append_basic(&dictEntry, DBUS_TYPE_STRING, &key);
                    {
                        DBusMessageIter variant;
                        auto& dbusType = std::get<0>(propertyEntry.second);
                        DBus::message_iter_open_container(&dictEntry, DBUS_TYPE_VARIANT, dbusType.c_str(), &variant);
                        auto& f = std::get<1>(propertyEntry.second);
                        f(msg, variant);
                        DBus::message_iter_close_container(&dictEntry, &variant);
                        
                    }
                    DBus::message_iter_close_container(&dict, &dictEntry);
                }
                DBus::message_iter_close_container(&args, &dict);

                connection->send(message);
                                
                return DBus::HandlerResult::Handled;
            }));
            
            
            filterHandles.push_back(connection->addFilter(path, "org.freedesktop.DBus.Properties", "Get", [=](DBusMessage* msg) {
                DBus::Error error;
                const char* interfaceGet;
                const char* property;
                bool status = DBus::message_get_args(msg, &error, DBUS_TYPE_STRING, &interfaceGet, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);
                if(!status) {
                    return DBus::HandlerResult::NotYetHandled;
                }
                printf("Get %s %s\n", interfaceGet, property);
                if(strcmp(interfaceGet, interface.c_str()) != 0) {
                    return DBus::HandlerResult::NotYetHandled;
                }
                auto propertyIter = properties.find(property);
                if(propertyIter == properties.end()) {
                   return DBus::HandlerResult::NotYetHandled;
                }
                
                {
                    auto message = DBus::message_new_method_return(msg);
                    DBusMessageIter args;
                    DBus::message_iter_init_append(message, &args);
                    std::get<1>(propertyIter->second)(message, args);
                    connection->send(message);
                }
                
                    
                return DBus::HandlerResult::Handled;
            }));
            
            auto introspectionFilter = [=](const std::string& path, const std::string& xml) {
                return connection->addFilter(path, "org.freedesktop.DBus.Introspectable", "Introspect", [=](DBusMessage* msg) {
                    auto message = DBus::message_new_method_return(msg);
                    DBusMessageIter args;
                    DBus::message_iter_init_append(message, &args);
                    const char* str = xml.c_str();
                    DBus::message_iter_append_basic(&args, DBUS_TYPE_STRING, &str);
                    connection->send(message);
                    
                    return DBus::HandlerResult::Handled;
                });
            };
            
            filterHandles.push_back(introspectionFilter("/", baseXml));
            filterHandles.push_back(introspectionFilter(path, xml));
        }
        virtual ~Interface() {
            for(auto& filterHandle: filterHandles) {
                connection->removeFilter(filterHandle);
            }
        }
        void addProperty(std::string dbusType, std::string name, GetterFunc getter, SetterFunc setter = SetterFunc()) {
            properties[name] = GetterSetterEntry(dbusType, getter, setter);
        }
        void addMethod(std::string name, GetterFunc getter) {
            methods[name] = getter;
        }
    };
    
};