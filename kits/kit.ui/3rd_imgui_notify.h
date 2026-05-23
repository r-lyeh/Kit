/**
 * @file ImGuiNotify.hpp
 * @brief A header-only library for creating toast notifications with ImGui.
 * 
 * Based on imgui-notify by patrickcjk
 * https://github.com/patrickcjk/imgui-notify
 * 
 * @version 0.0.33 by r-lyeh: C impl
 * @version 0.0.32 by r-lyeh: multi-viewports no longer needed. font-awesome no longer needed. animations.
 *                            C api. paused notifications if mouse is hovering them. simplified implementation.
 * @version 0.0.3  by TyomaVader
 * @date 07.07.2024
 */

#ifdef __cplusplus
extern "C" {
#endif

void igNotify(char type, double timeout_ss, const char *title, const char *body);
void igNotifyEx(char type, double timeout_ss, const char *title, const char *body, void (*fn)(void));
void igNotifyUpdate(void);
void igNotifyDemo(void);

#define igNotify(type,timeout_ss,title,...) igNotify(type,timeout_ss,title,"" __VA_ARGS__)

#if 1 // KIT_CODE
#pragma once

//< @r-lyeh include both imgui headers beforehand
//#include "imgui.h"
//#include "imgui_internal.h"

//< @r-lyeh optionally, include FA header beforehand
//#include "IconsFontAwesome6.h"
#ifndef ICON_FA_XMARK
#define ICON_FA_XMARK "X" //< @r-lyeh
#define ICON_FA_CIRCLE_INFO "(i)" //< @r-lyeh
#define ICON_FA_CIRCLE_CHECK "(v/)" //< @r-lyeh
#define ICON_FA_CIRCLE_EXCLAMATION "(!)" //< @r-lyeh
#define ICON_FA_TRIANGLE_EXCLAMATION "/!\\" //< @r-lyeh
#endif
 
/**
 * CONFIGURATION SECTION Start
*/

#define NOTIFY_STRDUP       SDL_strdup
#define NOTIFY_FREE         SDL_free
#define NOTIFY_TIME_MS()    elapsed.ms()

#define NOTIFY_PADDING_X                    20.f        // Bottom-left X padding
#define NOTIFY_PADDING_Y                    20.f        // Bottom-left Y padding
#define NOTIFY_PADDING_MESSAGE_Y            10.f        // Padding Y between each message
#define NOTIFY_FADE_IN_OUT_TIME             150         // Fade in and out duration
#define NOTIFY_DEFAULT_DISMISS              3000        // Auto dismiss after X ms. 0 to stick forever
#define NOTIFY_OPACITY                      0.8f        // 0-1 Toast opacity
#define NOTIFY_USE_SEPARATOR                false       // If true, a separator will be rendered between the title and the content
#define NOTIFY_USE_DISMISS_BUTTON           true        // If true, a dismiss button will be rendered in the top right corner of the toast
#define NOTIFY_RENDER_LIMIT                 5           // Max number of toasts rendered at the same time. Set to 0 for unlimited
#define NOTIFY_DEFAULT_BUTTON_TEXT          "+"

/**
 * CONFIGURATION SECTION End
*/

static const ImGuiWindowFlags NOTIFY_DEFAULT_TOAST_FLAGS = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;

typedef enum igNotifyType {
    igNotifyType_None,
    igNotifyType_Success,
    igNotifyType_Warning,
    igNotifyType_Error,
    igNotifyType_Info,
    igNotifyType_COUNT
} igNotifyType;

typedef enum igNotifyPhase {
    igNotifyPhase_FadeIn,
    igNotifyPhase_Wait,
    igNotifyPhase_FadeOut,
    igNotifyPhase_Expired,
    igNotifyPhase_COUNT
} igNotifyPhase;

typedef enum igNotifyPos {
    igNotifyPos_TopLeft,
    igNotifyPos_TopCenter,
    igNotifyPos_TopRight,
    igNotifyPos_BottomLeft,
    igNotifyPos_BottomCenter,
    igNotifyPos_BottomRight,
    igNotifyPos_Center,
    igNotifyPos_COUNT
} igNotifyPos;

/**
 * @brief A class for creating toast notifications with ImGui.
 */
typedef struct igNotifyToast {
    igNotifyType type;
    char *title;
    char *content;
    int dismissTime;
    ImGuiWindowFlags flags;
    uint64_t creationTime;
    void (*onButtonPress)();
    const char *buttonLabel;
    int id_;
} igNotifyToast;

    /**
     * @return const char* The default title of the toast notification, or generic one based on its type.
     */
    const char* igNotify_getTitle(const igNotifyToast *g) {
        if (g->title)  return g->title;
        switch (g->type) {
            default:
            case igNotifyType_None: return NULL;
            case igNotifyType_Success: return "Success";
            case igNotifyType_Warning: return "Warning";
            case igNotifyType_Error: return "Error";
            case igNotifyType_Info: return "Info";
        }
    };

    /**
     * @return ImVec4 The color of the toast notification, based on its type.
     */
    ImVec4 igNotify_getColor(const igNotifyToast *g) {
        switch (g->type) {
            default:
            case igNotifyType_None:      return ImVec4(255, 255, 255, 255); // White
            case igNotifyType_Success:   return ImVec4(0, 255, 0, 255); // Green
            case igNotifyType_Warning:   return ImVec4(255, 255, 0, 255); // Yellow
            case igNotifyType_Error:     return ImVec4(255, 0, 0, 255); // Error
            case igNotifyType_Info:      return ImVec4(0, 157, 255, 255); // Blue
        }
    }

    /**
     * @return const char* The icon of the toast notification, based on its type.
     */
    const char* igNotify_getIcon(const igNotifyToast *g) {
        switch (g->type) {
            default:
            case igNotifyType_None:    return NULL;
            case igNotifyType_Success: return ICON_FA_CIRCLE_CHECK;
            case igNotifyType_Warning: return ICON_FA_TRIANGLE_EXCLAMATION;
            case igNotifyType_Error:   return ICON_FA_CIRCLE_EXCLAMATION;
            case igNotifyType_Info:    return ICON_FA_CIRCLE_INFO;
        }
    }

    /**
     * @brief Get the elapsed time in milliseconds since the creation of the object.
     * 
     * @return int64_t The elapsed time in milliseconds.
     * @throws An exception with the message "Unsupported platform" if the platform is not supported.
     */
    int64_t igNotify_getElapsedTime(const igNotifyToast *g) {
        return NOTIFY_TIME_MS() - g->creationTime;
    }

    /**
     * @return igNotifyPhase The current phase of the toast notification, based on the elapsed time since its creation.
     *         - igNotifyPhase_FadeIn: The notification is fading in.
     *         - igNotifyPhase_Wait: The notification is waiting to be dismissed.
     *         - igNotifyPhase_FadeOut: The notification is fading out.
     *         - igNotifyPhase_Expired: The notification has expired and should be removed.
     */
    igNotifyPhase igNotify_getPhase(const igNotifyToast *g) {
        const int64_t elapsed = igNotify_getElapsedTime(g);

        /**/ if (elapsed > NOTIFY_FADE_IN_OUT_TIME + g->dismissTime + NOTIFY_FADE_IN_OUT_TIME) return g->dismissTime <= 0 ? igNotifyPhase_Wait : igNotifyPhase_Expired; //< @r-lyeh: Wait
        else if (elapsed > NOTIFY_FADE_IN_OUT_TIME + g->dismissTime) return igNotifyPhase_FadeOut;
        else if (elapsed > NOTIFY_FADE_IN_OUT_TIME) return igNotifyPhase_Wait;
        else return igNotifyPhase_FadeIn;
    }

    /**
     * @return The percentage of fade for the notification.
     */
    float igNotify_getFadePercent(const igNotifyToast *g) {
        const int64_t elapsed = igNotify_getElapsedTime(g);
        const igNotifyPhase phase = igNotify_getPhase(g);

        if (phase == igNotifyPhase_FadeIn)  return ((float)elapsed / (float)NOTIFY_FADE_IN_OUT_TIME) * NOTIFY_OPACITY;
        if (phase == igNotifyPhase_FadeOut) return (1.f - (((float)elapsed - (float)NOTIFY_FADE_IN_OUT_TIME - (float)g->dismissTime) / (float)NOTIFY_FADE_IN_OUT_TIME)) * NOTIFY_OPACITY;
        return 1.f * NOTIFY_OPACITY;
    }


    static array_(igNotifyToast) notifications;

    void igNotifyRemove(int i) {
        int count = array_count(notifications);
        if( i >= 0 && i < count ) {
            if( notifications[i].title ) NOTIFY_FREE(notifications[i].title);
            if( notifications[i].content ) NOTIFY_FREE(notifications[i].content);
            array_delswap(notifications, i); // @fixme: do a memmove+array_pop() instead
        }
    }

    /**
     * Renders all notifications in the notifications vector.
     * Each notification is rendered as a toast window with a title, content and an optional icon.
     * If a notification is expired, it is removed from the vector.
     */
    void igNotifyUpdate(void) {

        float height = 0.f;
        const ImVec2 mainWindowSize = igGetMainViewport()->Size;

        bool paused = 0;
        void (*clicked)() = 0;

        for (size_t i = 0; i < array_count(notifications); ++i) {
            igNotifyToast* toast = &notifications[i];

            #if NOTIFY_RENDER_LIMIT > 0
                if (i > NOTIFY_RENDER_LIMIT)
                {
                    continue;
                }
            #endif

            // Get icon, title and other data
            const char* icon = igNotify_getIcon(toast);
            const char* title = toast->title;
            const char* content = toast->content;
            const char* defaultTitle = igNotify_getTitle(toast);
            const float opacity = igNotify_getFadePercent(toast); // Get opacity based of the current phase
            ImVec4 textColor = igNotify_getColor(toast); textColor.w = opacity;

            // Generate new unique name for this toast
            char windowName[50];
            snprintf(windowName, 50, "##TOAST%d", toast->id_);

            //PushStyleColor(ImGuiCol_Text, textColor);
            igSetNextWindowBgAlpha(opacity);

#if IMGUI_HAS_DOCK
            int NOTIFY_RENDER_OUTSIDE_MAIN_WINDOW = igGetIO_Nil()->ConfigFlags & ImGuiConfigFlags_ViewportsEnable;
            if( NOTIFY_RENDER_OUTSIDE_MAIN_WINDOW ) {
                short mainMonitorId = ImGuiViewportP_ImGuiViewportP()->PlatformMonitor;
                ImGuiPlatformIO* platformIO = igGetPlatformIO_Nil();
                ImGuiPlatformMonitor* monitor = &platformIO->Monitors.Data[mainMonitorId];

                // Set notification window position to bottom right corner of the monitor
                igSetNextWindowPos(ImVec2(monitor->WorkPos.x + monitor->WorkSize.x - NOTIFY_PADDING_X, monitor->WorkPos.y + monitor->WorkSize.y - NOTIFY_PADDING_Y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
            }
            else
#endif
            {
                // Set notification window position to bottom right corner of the main window, considering the main window size and location in relation to the display
                ImVec2 mainWindowPos = igGetMainViewport()->Pos;
                igSetNextWindowPos(ImVec2(mainWindowPos.x + mainWindowSize.x - NOTIFY_PADDING_X, mainWindowPos.y + mainWindowSize.y - NOTIFY_PADDING_Y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
            }
            igSetNextWindowSize(ImVec2(0,0), ImGuiCond_Always);

            // Set notification window flags
            if (!NOTIFY_USE_DISMISS_BUTTON && toast->onButtonPress == NULL) {
                toast->flags |= ImGuiWindowFlags_NoInputs;
            }

            igBegin(windowName, NULL, toast->flags);

            // Render over all other windows
            igBringWindowToDisplayFront(igGetCurrentWindow());

            // Here we render the toast content
            {
                igPushTextWrapPos(mainWindowSize.x / 3.f); // We want to support multi-line text, this will wrap the text after 1/3 of the screen width

                bool wasTitleRendered = false;

                // If an icon is set
                if ( icon && icon[0] ) {
                    //Text(icon); // Render icon text
                    igTextColored(textColor, "%s", icon);
                    wasTitleRendered = true;
                }

                // If a title is set
                if ( title && title[0] ) {
                    // If a title and an icon is set, we want to render on same line
                    if (icon && icon[0])
                        igSameLine(0,-1);

                    igText("%s", title); // Render title text
                    wasTitleRendered = true;
                }
                else 
                if (defaultTitle && defaultTitle[0]) {
                    if (icon && icon[0])
                        igSameLine(0,-1);

                    igText("%s", defaultTitle); // Render default title text (igNotifyType_Success -> "Success", etc...)
                    wasTitleRendered = true;
                }

                // If a dismiss button is enabled
                if (NOTIFY_USE_DISMISS_BUTTON) {
                    // If a title or content is set, we want to render the button on the same line
                    if (wasTitleRendered || (content && content[0])) {
                        igSameLine(0,-1);
                    }

                    // Render the dismiss button on the top right corner
                    // NEEDS TO BE REWORKED
                    float scale = 0.8f;

                    if (igCalcTextSize(content,0,0,-1).x > igGetContentRegionAvail().x) {
                        scale = 0.8f;
                    }

                    igSetCursorPosX(igGetCursorPosX() + (igGetWindowSize().x - igGetCursorPosX()) * scale);

                    // If the button is pressed, we want to remove the notification
                    if (igButton(ICON_FA_XMARK,ImVec2(0,0))) {
                        igNotifyRemove(i); // notifications.erase(notifications.begin()+i);
                        --i;
                    }
                }

                // In case ANYTHING was rendered in the top, we want to add a small padding so the text (or icon) looks centered vertically
                if (wasTitleRendered && (content && content[0])) {
                    igSetCursorPosY(igGetCursorPosY() + 5.f); // Must be a better way to do this!!!!
                }

                // If a content is set
                if (content && content[0]) {
                    if (wasTitleRendered) {
                        #if NOTIFY_USE_SEPARATOR
                            igSeparator();
                        #endif
                    }

                    igText("%s", content); // Render content text
                }

                // If a button is set
                if (toast->onButtonPress != NULL) {
                    // If the button is pressed, we want to execute the lambda function
                    if (igButton(toast->buttonLabel, ImVec2(0,0))) {
                        clicked = toast->onButtonPress;
                    }
                }

                igPopTextWrapPos();
            }

            // Save height for next toasts
            height += igGetWindowHeight() + NOTIFY_PADDING_MESSAGE_Y;

            paused |= !!igIsWindowHovered(0);

            // End
            igEnd();
        }

        // Remove expired toasts
        for (size_t i = 0; i < array_count(notifications); ++i) {
            igNotifyToast* toast = &notifications[i];

            if (paused) toast->dismissTime += 1000/60;

            else

            if (igNotify_getPhase(toast) == igNotifyPhase_Expired) {
                igNotifyRemove(i); // notifications.erase(notifications.begin()+i);
                --i;
                continue;
            }
        }

        if(clicked) clicked();
    }

//> @r-lyeh
void igNotifyEx(char type, double timeout_ss, const char *head, const char *body, void (*fn)(void)) {
    // (n)one, o(k), (w)arn, (i)nfo, (e)rror

    igNotifyToast t = {0};
    t.type = igNotifyType_None;
    t.title = head ? NOTIFY_STRDUP(head) : NULL;
    t.content = body ? NOTIFY_STRDUP(body) : NULL;
    t.dismissTime = timeout_ss ? (int)(timeout_ss * 1000) : NOTIFY_DEFAULT_DISMISS;
    t.flags = NOTIFY_DEFAULT_TOAST_FLAGS;
    t.creationTime = NOTIFY_TIME_MS();
    t.onButtonPress = fn ? fn : NULL;
    t.buttonLabel = NOTIFY_DEFAULT_BUTTON_TEXT;

    if(type != igNotifyType_None)
        t.type =
            type == 'w' || type == 'W' ? igNotifyType_Warning :
            type == 'e' || type == 'E' ? igNotifyType_Error :
            type == 'i' || type == 'I' ? igNotifyType_Info : igNotifyType_Success;

    static int counter = 0;
    t.id_ = counter++;
    array_push(notifications, t);
}
void (igNotify)(char type, double timeout_ss, const char *title, const char *message) {
    igNotifyEx(type, timeout_ss, title, message, NULL);
}

void igNotifyDemoThanks(void) {
    igNotify('K', 3.0, "Thanks for clicking!");
}

void igNotifyDemo(void) {
    igSetNextWindowPos(ImVec2(igGetIO_Nil()->DisplaySize.x / 2, igGetIO_Nil()->DisplaySize.y / 2), ImGuiCond_Once, ImVec2(0,0));
    igSetNextWindowSize(ImVec2(550, 550), ImGuiCond_Once);
    igBegin("ImGui Notify Test Window", NULL, 0);

    if (igCollapsingHeader_TreeNodeFlags("Examples without title", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (igButton("Success",ImVec2(0,0))) igNotify('K', 5.0, "That is a success!");
        igSameLine(0,-1); if(igButton("Warning",ImVec2(0,0))) igNotify('W', 5.0, "This is a warning!");
        igSameLine(0,-1); if(igButton("Error",ImVec2(0,0))) igNotify('E', 5.0, "Segmentation fault");
        igSameLine(0,-1); if(igButton("Info",ImVec2(0,0))) igNotify('I', 5.0, "Info about ImGui...");
        igSameLine(0,-1); if(igButton("Info long",ImVec2(0,0))) igNotify('I', 5.0, "Hi, I'm a long notification. I'm here to show you that you can write a lot of text in me. I'm also here to show you that I can wrap text, so you don't have to worry about that.");
        igSameLine(0,-1); if(igButton("Notify with button",ImVec2(0,0))) igNotifyEx('E', 5.0, "Click me!", "Notification content", igNotifyDemoThanks );
    }

    if (igCollapsingHeader_TreeNodeFlags("Do it yourself", ImGuiTreeNodeFlags_DefaultOpen)) {
        static char title[4096] = "Hello there!";
        igInputTextMultiline("Title", title, sizeof(title), ImVec2(0,0),0,NULL,NULL);

        static char content[4096] = "General Kenobi! \n- Grevious";
        igInputTextMultiline("Content", content, sizeof(content), ImVec2(0,0),0,NULL,NULL);

        static int duration = 5.0; // 5 seconds
        igInputInt("Duration (ss)", &duration, 15,100,0);
        if (duration < 0) duration = 0; // Shouldn't be negative

        static int type = 1;
        static const char* type_str[] = { "None", "Success", "Warning", "Error", "Info" };

        if (igBeginCombo("Type", type_str[type], 0)) {
            for (auto n = 0; n < COUNTOF(type_str); n++) {
                const bool isSelected = ((uint8_t)type == n);

                if (igSelectable_Bool(type_str[n], isSelected, false, ImVec2(0,0)))
                    type = n;

                if (isSelected)
                    igSetItemDefaultFocus();
            }

            igEndCombo();
        }

        static bool enable_title = true, enable_content = true;
        igCheckbox("Enable title", &enable_title);
        igSameLine(0,-1);
        igCheckbox("Enable content", &enable_content);

        if (igButton("Show",ImVec2(0,0)))
            igNotifyEx(type_str[type][0], duration, enable_title ? title : NULL, enable_content ? content : NULL, NULL);
    }

    igEnd();
}

#endif // KIT_CODE

#ifdef __cplusplus
}
#endif
