// Author: Damian "Rush" Kaczmarek, Code Charm Ltd
// License: MIT

#include "slimdbus.hpp"

#include "systemtray/kde.h"

#define WATCHER_NAME        "org.kde.StatusNotifierWatcher"
#define WATCHER_OBJECT      "/StatusNotifierWatcher"
#define WATCHER_INTERFACE   "org.kde.StatusNotifierWatcher"

#define ITEM_NAME           "org.kde.StatusNotifierItem"
#define ITEM_OBJECT         "/StatusNotifierItem"
#define ITEM_INTERFACE      "org.kde.StatusNotifierItem"


static const char* watcherXml =
    "<node>"
    "   <interface name='org.kde.StatusNotifierWatcher'>"
    "       <property name='IsStatusNotifierHostRegistered' type='b' access='read' />"
    "       <method name='RegisterStatusNotifierItem'>"
    "           <arg name='service' type='s' direction='in' />"
    "       </method>"
    "       <signal name='StatusNotifierHostRegistered' />"
    "       <signal name='StatusNotifierHostUnregistered' />"
    "   </interface>"
    "</node>";

static const char* itemXml = 
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"" \
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">" \
"<node>" \
"  <interface name=\"org.freedesktop.DBus.Introspectable\">" \
"    <method name=\"Introspect\">" \
"      <arg name=\"xml_data\" type=\"s\" direction=\"out\"/>" \
"    </method>" \
"  </interface>" \
"  <interface name=\"org.freedesktop.DBus.Peer\">" \
"    <method name=\"Ping\"/>" \
"    <method name=\"GetMachineId\">" \
"      <arg name=\"machine_uuid\" type=\"s\" direction=\"out\"/>" \
"    </method>" \
"  </interface>" \
"  <node name=\"MenuBar\"/>" \
"  <node name=\"StatusNotifierItem\"/>" \
"</node>" \
"";

static const char* itemXml2 =
    "<node>"
    "   <interface name='org.kde.StatusNotifierItem'>"
    "       <property name='Id' type='s' access='read' />"
    "       <property name='Category' type='s' access='read' />"
    "       <property name='Title' type='s' access='read' />"
    "       <property name='Status' type='s' access='read' />"
    "       <property name='WindowId' type='i' access='read' />"
    "       <property name='IconName' type='s' access='read' />"
    "       <property name='IconPixmap' type='(iiay)' access='read' />"
    "       <property name='OverlayIconName' type='s' access='read' />"
    "       <property name='OverlayIconPixmap' type='(iiay)' access='read' />"
    "       <property name='AttentionIconName' type='s' access='read' />"
    "       <property name='AttentionIconPixmap' type='(iiay)' access='read' />"
    "       <property name='AttentionMovieName' type='s' access='read' />"
    "       <property name='ToolTip' type='(s(iiay)ss)' access='read' />"
    "       <method name='ContextMenu'>"
    "           <arg name='x' type='i' direction='in' />"
    "           <arg name='y' type='i' direction='in' />"
    "       </method>"
    "       <method name='Activate'>"
    "           <arg name='x' type='i' direction='in' />"
    "           <arg name='y' type='i' direction='in' />"
    "       </method>"
    "       <method name='SecondaryActivate'>"
    "           <arg name='x' type='i' direction='in' />"
    "           <arg name='y' type='i' direction='in' />"
    "       </method>"
    "       <method name='Scroll'>"
    "           <arg name='delta' type='i' direction='in' />"
    "           <arg name='orientation' type='s' direction='in' />"
    "       </method>"
    "       <signal name='NewTitle' />"
    "       <signal name='NewIcon' />"
    "       <signal name='NewAttentionIcon' />"
    "       <signal name='NewOverlayIcon' />"
    "       <signal name='NewToolTip' />"
    "       <signal name='NewStatus'>"
    "           <arg name='status' type='s' />"
    "       </signal>"
    "   </interface>"
    "  <interface name='org.freedesktop.DBus.Properties'>"
    "    <method name='Get'>"
    "      <arg name='interface_name' type='s' direction='in'/>"
    "      <arg name='property_name' type='s' direction='in'/>"
    "      <arg name='value' type='v' direction='out'/>"
    "    </method>"
    "    <method name='Set'>"
    "      <arg name='interface_name' type='s' direction='in'/>"
    "      <arg name='property_name' type='s' direction='in'/>"
    "      <arg name='value' type='v' direction='in'/>"
    "    </method>"
    "    <method name='GetAll'>"
    "      <arg name='interface_name' type='s' direction='in'/>"
    "      <arg name='values' type='a{sv}' direction='out'/>"
    "      <annotation name='org.qtproject.QtDBus.QtTypeName.Out0' value='QVariantMap'/>"
    "    </method>"
    "    <signal name='PropertiesChanged'>"
    "      <arg name='interface_name' type='s' direction='out'/>"
    "      <arg name='changed_properties' type='a{sv}' direction='out'/>"
    "      <annotation name='org.qtproject.QtDBus.QtTypeName.Out1' value='QVariantMap'/>"
    "      <arg name='invalidated_properties' type='as' direction='out'/>"
    "    </signal>"
    "  </interface>"
    "  <interface name='org.freedesktop.DBus.Introspectable'>"
    "    <method name='Introspect'>"
    "      <arg name='xml_data' type='s' direction='out'/>"
    "    </method>"
    "  </interface>"
    "  <interface name='org.freedesktop.DBus.Peer'>"
    "    <method name='Ping'/>"
    "    <method name='GetMachineId'>"
    "      <arg name='machine_uuid' type='s' direction='out'/>"
    "    </method>"
    "  </interface>"
    "</node>";

class StatusNotifierItem : public DBus::Interface
{
    struct Icon {
        int w, h;
        std::vector<uint8_t> bytes;
    };
    std::string m_id = "", m_category = "ApplicationStatus", m_title = "Title", m_status = "Active", m_iconName, m_overlayIconName, m_attentionIconName, m_attentionMovieName;
    uint32_t m_windowId = 0;
    Icon m_iconPixmap = {0, 0}, m_overlayIconPixmap = {0, 0}, m_attentionIconPixmap = {0, 0};
    
    bool registered = false;
    std::string name;
public:;
    enum class Status {
        Passive = 0,
        Active = 1,
        NeedsAttention = 2
    };

    void setId(const std::string& str) {
        m_id = str;
    }
    void setCategory(const std::string& str) {
        m_category = str;
    }
    void setTitle(const std::string& str) {
        m_title = str;
        if(registered) {
            emitSignal("NewTitle");
        }
    }
    
    void setCustomStatus(const std::string str) {
        m_status = str;
        if(registered) {
            emitSignal("NewStatus", [=](DBusMessageIter &args) {
                const char* str = m_status.c_str();
                printf("Emit signal %s\n", str);
                DBus::message_iter_append_basic(&args, DBUS_TYPE_STRING, &str);
            });
        }        
    }
    
    void setStatus(Status status) {
        const char* str;
        switch(status) {
            case Status::Passive: str = "Passive"; break;
            case Status::Active: str = "Active"; break;
            case Status::NeedsAttention: str = "NeedsAttention"; break;
            default:
                str = "";
        }
        setCustomStatus(str);
    }
    void setIconName(const std::string& str) {
        m_iconName = str;
        if(registered) { emitSignal("NewIcon"); }
    }
    void setOverlayIconName(const std::string& str) {
        m_overlayIconName = str;
        if(registered) { emitSignal("NewOverlayIcon"); }
    }
    void setAttentionIconName(const std::string& str) {
        m_attentionIconName = str;
        if(registered) { emitSignal("NewAttentionIcon"); }
    }
    void setAttentionIconPixmap(int w, int h, const uint8_t* bytes) {
        m_attentionIconPixmap = Icon { .w = w, .h = h, .bytes = std::vector<uint8_t>(bytes, bytes + w*h*4)};
        if(registered) { emitSignal("NewAttentionIcon"); }
    }
    void setAttentionMovieName(const std::string& str) {
        m_attentionMovieName = str;
        if(registered) { emitSignal("NewAttentionIcon"); }
    }
    void setWindowId(uint32_t id) {
        m_windowId = id;
    }
    void setIconPixmap(int w, int h, const uint8_t* bytes) {
        m_iconPixmap = Icon { .w = w, .h = h, .bytes = std::vector<uint8_t>(bytes, bytes + w*h*4)};
        if(registered) { emitSignal("NewIcon"); }
    }
    void setOverlayIconPixmap(int w, int h, const uint8_t* bytes) {
        m_overlayIconPixmap = Icon { .w = w, .h = h, .bytes = std::vector<uint8_t>(bytes, bytes + w*h*4)};
        if(registered) { emitSignal("NewOverlayIcon"); }
    }
    
    ~StatusNotifierItem() {
        DBus::Error error;
        DBus::bus_release_name(connection->rawPtr, name.c_str(), &error);
    }
    
    StatusNotifierItem(DBus::Connection::Ptr connection)
    : DBus::Interface("/StatusNotifierItem", "org.kde.StatusNotifierItem", itemXml2, connection)
    {
        auto stringHandler = [](std::string& value) {
            return [=,&value] (DBusMessage* msg, DBusMessageIter& iter) {
                const char* str = value.c_str();
                DBus::message_iter_append_basic(&iter, DBUS_TYPE_STRING, &str);
            };
        };

        auto iconHandler = [](Icon& icon) {
            return [=,&icon](DBusMessage* msg, DBusMessageIter& iterBase) {
                
                
                DBusMessageIter subIter;
                
                DBusMessageIter iter;
                DBus::message_iter_open_container(&iterBase, DBUS_TYPE_ARRAY, "(iiay)", &iter);
                
                if(icon.w && icon.h) {
                
                    DBus::message_iter_open_container(&iter, DBUS_TYPE_STRUCT, nullptr, &subIter);
                    
                    int w = icon.w, h = icon.h;
                    DBus::message_iter_append_basic(&subIter, DBUS_TYPE_INT32, &w);
                    DBus::message_iter_append_basic(&subIter, DBUS_TYPE_INT32, &h);
                    
                    DBusMessageIter subSubIter;
                    DBus::message_iter_open_container(&subIter, DBUS_TYPE_ARRAY, "y", &subSubIter);
                    for(auto byte: icon.bytes) {
                        DBus::message_iter_append_basic(&subSubIter, DBUS_TYPE_BYTE, &byte);
                    }
                    DBus::message_iter_close_container(&subIter, &subSubIter);
                    
                    DBus::message_iter_close_container(&iter, &subIter);
                }
                
                DBus::message_iter_close_container(&iterBase, &iter);
            };
        };
        
        addProperty("i", "WindowId", [=] (DBusMessage* msg, DBusMessageIter& iter) {
            DBus::message_iter_append_basic(&iter, DBUS_TYPE_INT32, &m_windowId);
        });
        
        addProperty("s", "AttentionIconName", stringHandler(m_attentionIconName));
        addProperty("s", "AttentionMovieName", stringHandler(m_attentionMovieName));
        addProperty("s", "Category", stringHandler(m_category));
        addProperty("s", "Id", stringHandler(m_id));
        addProperty("s", "Title", stringHandler(m_title));
        addProperty("s", "Status", stringHandler(m_status));
        addProperty("s", "IconName", stringHandler(m_iconName));
        addProperty("a(iiay)", "IconPixmap",iconHandler(m_iconPixmap));
        addProperty("a(iiay)", "OverlayIconPixmap",iconHandler(m_overlayIconPixmap));
        addProperty("a(iiay)", "AttentionIconPixmap",iconHandler(m_attentionIconPixmap));
        
    }
    void registerWithHost(std::function<void(bool)> callback = std::function<void(bool)>()) {
        auto message = DBus::message_new_method_call(WATCHER_NAME, WATCHER_OBJECT, "org.freedesktop.DBus.Properties", "Get");
        {
            DBusMessageIter args;
            DBus::message_iter_init_append(message, &args);
            const char* interface = WATCHER_INTERFACE;
            const char* property = "IsStatusNotifierHostRegistered";
            DBus::message_iter_append_basic(&args, DBUS_TYPE_STRING, &interface);
            DBus::message_iter_append_basic(&args, DBUS_TYPE_STRING, &property);
        }
        
        connection->sendWithReply(message, [=](DBusMessage* reply) {            
            static int id = 0;
            
            {
                static char name_buf[1024];
                snprintf(name_buf, sizeof(name_buf), "org.kde.StatusNotifierItem-%u-%u", getpid(), id++);
                name = name_buf;
            }
            
            DBus::Error error;
            int code = DBus::bus_request_name(connection->rawPtr, name.c_str(), 0, &error);
            assert(code == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

            auto message = DBus::message_new_method_call(WATCHER_NAME, WATCHER_OBJECT, WATCHER_INTERFACE, "RegisterStatusNotifierItem");
            {
                DBusMessageIter args;
                DBus::message_iter_init_append(message, &args);
                const char* str = name.c_str();
                DBus::message_iter_append_basic(&args, DBUS_TYPE_STRING, &str);
            }
            connection->sendWithReply(message, [=](DBusMessage* reply) {            
                // TODO check reply
                registered = true;
            });
        });
    }
};

class KDETrayIcon::Private : public StatusNotifierItem {
public:
    std::function<void(int, int)> handlerContextMenu;
    std::function<void(int, int)> handlerActivate;
    std::function<void(int, int)> handlerSecondaryActivate;
    std::function<void(int, std::string)> handlerScroll;
    bool processed = false;
    Private() : StatusNotifierItem(DBus::Connection::Get()) {
        
    }
    friend class KDETrayIcon;
};

KDETrayIcon::KDETrayIcon(const std::string& id) : d_ptr(new KDETrayIcon::Private) {
    d_ptr->setId(id);
    d_ptr->addMethod("ContextMenu", [=](DBusMessage* msg, DBusMessageIter& iter) {
        int x, y;
        DBus::Error err;
        if(!DBus::message_get_args(msg, &err, DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y, DBUS_TYPE_INVALID))
            return;
        if(d_ptr->handlerContextMenu)
            d_ptr->handlerContextMenu(x, y);
    });
    d_ptr->addMethod("Activate", [=](DBusMessage* msg, DBusMessageIter& iter) {
        int x, y;
        DBus::Error err;
        if(!DBus::message_get_args(msg, &err, DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y, DBUS_TYPE_INVALID))
            return;
        if(d_ptr->handlerActivate) {
            d_ptr->handlerActivate(x, y);
        }
    });
    d_ptr->addMethod("SecondaryActivate", [=](DBusMessage* msg, DBusMessageIter& iter) {
        int x, y;
        DBus::Error err;
        if(!DBus::message_get_args(msg, &err, DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y, DBUS_TYPE_INVALID))
            return;
        if(d_ptr->handlerSecondaryActivate)
            d_ptr->handlerSecondaryActivate(x, y);                
    });
    d_ptr->addMethod("Scroll", [=](DBusMessage* msg, DBusMessageIter& iter) {
        int distance;
        const char* orientation = "";
        DBus::Error err;
        if(!DBus::message_get_args(msg, &err, DBUS_TYPE_INT32, &distance, DBUS_TYPE_STRING, &orientation, DBUS_TYPE_INVALID))
            return;
        if(d_ptr->handlerScroll)
            d_ptr->handlerScroll(distance, orientation);
    });
}

void KDETrayIcon::setCategory(const std::string& str) {
    d_ptr->setCategory(str);
}
void KDETrayIcon::setTitle(const std::string& str) {
    d_ptr->setTitle(str);
}

void KDETrayIcon::setStatus(KDETrayIcon::Status status) {
    d_ptr->setStatus((KDETrayIcon::Private::Status)DBus::to_underlying(status) );
}
void KDETrayIcon::setCustomStatus(const std::string& str) {
    d_ptr->setCustomStatus(str);
}

void KDETrayIcon::setWindowId(uint32_t id) {
    d_ptr->setWindowId(id);
}

void KDETrayIcon::setIconName(const std::string& str) {
    d_ptr->setIconName(str);
}
void KDETrayIcon::setIconPixmap(int w, int h, const uint8_t* data) {
    d_ptr->setIconPixmap(w, h, data);
}
void KDETrayIcon::setOverlayIconName(const std::string& str) {
    d_ptr->setOverlayIconName(str);
}
void KDETrayIcon::setOverlayIconPixmap(int w, int h, const uint8_t* data) {
    d_ptr->setOverlayIconPixmap(w, h, data);
}
void KDETrayIcon::setAttentionIconPixmap(int w, int h, const uint8_t* data) {
    d_ptr->setAttentionIconPixmap(w, h, data);
}

void KDETrayIcon::setAttentionMovieName(const std::string& str) {
    d_ptr->setAttentionMovieName(str);
}

void KDETrayIcon::onContextMenu(std::function<void(int, int)> handler) {
    d_ptr->handlerContextMenu = handler;
}
void KDETrayIcon::onActivate(std::function<void(int, int)> handler) {
    d_ptr->handlerActivate = handler;
}
void KDETrayIcon::onSecondaryActivate(std::function<void(int, int)> handler) {
    d_ptr->handlerSecondaryActivate = handler;
}
void KDETrayIcon::onScroll(std::function<void(int, std::string)> handler) {
    d_ptr->handlerScroll = handler;
}

bool KDETrayIcon::process(int timeout) {
    if(!d_ptr->processed) {
        d_ptr->registerWithHost();
        d_ptr->processed = true;
    }
    return d_ptr->connection->readWriteDispatch(timeout);
}

KDETrayIcon::~KDETrayIcon() {
    delete d_ptr;
}


kdetrayicon* kdetrayicon_create(const char* id) {
    return (kdetrayicon*) (new KDETrayIcon(id));
}

void kdetrayicon_destroy(kdetrayicon* handle) {
    delete (KDETrayIcon*)handle;
}

void kdetrayicon_set_category(kdetrayicon* handle, const char* str) {
    ((KDETrayIcon*)handle)->setCategory(str);
}

void kdetrayicon_set_title(kdetrayicon* handle, const char* str) {
    ((KDETrayIcon*)handle)->setTitle(str);
}

void kdetrayicon_set_status(kdetrayicon* handle, KDETrayIconStatus status) {
    ((KDETrayIcon*)handle)->setStatus((KDETrayIcon::Status)status);
}

void kdetrayicon_set_custom_status(kdetrayicon* handle, const char* str) {
    ((KDETrayIcon*)handle)->setCustomStatus(str);
}

void kdetrayicon_set_window_id(kdetrayicon* handle, uint32_t id) {
    ((KDETrayIcon*)handle)->setWindowId(id);
}

void kdetrayicon_set_icon_name(kdetrayicon* handle, const char* str) {
    ((KDETrayIcon*)handle)->setIconName(str);
}

void kdetrayicon_set_icon_pixmap(kdetrayicon* handle, int w, int h, const uint8_t* data) {
    ((KDETrayIcon*)handle)->setIconPixmap(w, h, data);
}

void kdetrayicon_set_overlay_icon_name(kdetrayicon* handle, const char* str) {
    ((KDETrayIcon*)handle)->setOverlayIconName(str);
}

void kdetrayicon_set_attention_movie_name(kdetrayicon* handle, const char* str) {
    ((KDETrayIcon*)handle)->setAttentionMovieName(str);
}

void kdetrayicon_set_overlay_icon_pixmap(kdetrayicon* handle, int w, int h, const uint8_t* data) {
    ((KDETrayIcon*)handle)->setOverlayIconPixmap(w, h, data);
}

void kdetrayicon_set_attention_icon_pixmap(kdetrayicon* handle, int w, int h, const uint8_t* data) {
    ((KDETrayIcon*)handle)->setAttentionIconPixmap(w, h, data);
}

void kdetrayicon_on_context_menu(kdetrayicon* handle, void(*handler)(int, int, void*), void* user_ptr) {
    ((KDETrayIcon*)handle)->onContextMenu([=](int x, int y) {
        handler(x, y, user_ptr);
    });
}

void kdetrayicon_on_activate(kdetrayicon* handle, void(*handler)(int, int, void*), void* user_ptr) {
    ((KDETrayIcon*)handle)->onActivate([=](int x, int y) {
        handler(x, y, user_ptr);
    });
}

void kdetrayicon_on_secondary_activate(kdetrayicon* handle, void(*handler)(int, int, void*), void* user_ptr) {
    ((KDETrayIcon*)handle)->onSecondaryActivate([=](int x, int y) {
        handler(x, y, user_ptr);
    });
}

void kdetrayicon_on_scroll(kdetrayicon* handle, void(*handler)(int, const char*, void*), void* user_ptr) {
    ((KDETrayIcon*)handle)->onScroll([=](int offset, std::string orientation) {
        handler(offset, orientation.c_str(), user_ptr);
    });
}

bool kdetrayicon_process(kdetrayicon* handle, int timeout) {
    ((KDETrayIcon*)handle)->process(timeout);
}
