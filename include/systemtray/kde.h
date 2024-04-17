#pragma once

#ifdef __cplusplus
#include <functional>
#include <string>

class KDETrayIcon
{
    class Private;
    Private* d_ptr;

public:
    enum class Status {
        Passive = 0,
        Active = 1,
        NeedsAttention = 2
    };
    
    KDETrayIcon(const std::string& id);
    ~KDETrayIcon();
    void setCategory(const std::string& str);
    void setTitle(const std::string& str);
    
    void setStatus(Status status);
    void setCustomStatus(const std::string& str);
    
    void setWindowId(uint32_t id);
    
    void setIconName(const std::string& str);
    void setIconPixmap(int w, int h, const uint8_t* data);
    void setOverlayIconName(const std::string& str);
    void setOverlayIconPixmap(int w, int h, const uint8_t* data);
    void setAttentionIconPixmap(int w, int h, const uint8_t* data);
    void setAttentionMovieName(const std::string& str);
    
    void onContextMenu(std::function<void(int, int)> handler);
    void onActivate(std::function<void(int, int)> handler);
    void onSecondaryActivate(std::function<void(int, int)> handler);
    void onScroll(std::function<void(int, std::string)> handler);
    
    bool process(int timeout);
};
#else
#include <stdbool.h>
#endif

#include <stdint.h>

typedef struct kdetrayicon kdetrayicon;

typedef enum {
    KDETrayIcon_Passive = 0,
    KDETrayIcon_Active = 1,
    KDETrayIcon_NeedsAttention = 2
} KDETrayIconStatus;


#ifdef __cplusplus
extern "C" {
#endif

kdetrayicon* kdetrayicon_create(const char* id);
void kdetrayicon_destroy(kdetrayicon* handle);
void kdetrayicon_set_category(kdetrayicon* handle, const char* str);
void kdetrayicon_set_title(kdetrayicon* handle, const char* str);

void kdetrayicon_set_status(kdetrayicon* handle, KDETrayIconStatus status);
void kdetrayicon_set_custom_status(kdetrayicon* handle, const char* str);

void kdetrayicon_set_window_id(kdetrayicon* handle, uint32_t id);

void kdetrayicon_set_icon_name(kdetrayicon* handle, const char* str);
void kdetrayicon_set_icon_pixmap(kdetrayicon* handle, int w, int h, const uint8_t* data);

void kdetrayicon_set_overlay_icon_name(kdetrayicon* handle, const char* str);
void kdetrayicon_set_overlay_icon_pixmap(kdetrayicon* handle, int w, int h, const uint8_t* data);

void kdetrayicon_set_attention_icon_pixmap(kdetrayicon* handle, int w, int h, const uint8_t* data);
void kdetrayicon_set_attention_movie_name(kdetrayicon* handle, const char* str);

void kdetrayicon_on_context_menu(kdetrayicon* handle, void(*handler)(int, int, void*), void* user_ptr);
void kdetrayicon_on_activate(kdetrayicon* handle, void(*handler)(int, int, void*), void* user_ptr);
void kdetrayicon_on_secondary_activate(kdetrayicon* handle, void(*handler)(int, int, void*), void* user_ptr);
void kdetrayicon_on_scroll(kdetrayicon* handle, void(*handler)(int, const char*, void*), void* user_ptr);

bool kdetrayicon_process(kdetrayicon* handle, int timeout);

#ifdef __cplusplus
}
#endif
