#include "kit.h"

// checkbox toggle callback
static int checkbox_state = 0;
void on_tray_toggle(void *userdata, SDL_TrayEntry *entry) {
    checkbox_state = !checkbox_state;
    SDL_SetTrayEntryChecked(entry, checkbox_state);
}

// quit callback
void on_tray_quit(void *userdata, SDL_TrayEntry *entry) {
    SDL_Event e;
    e.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&e);
}

void main(event ev)
{
    static SDL_Tray *tray = NULL;

    // initialize
    if( ev.init ) {

        // load an icon image
        SDL_Surface *icon = SDL_LoadPNG("demos/art/tray.png");  // simple PNG for portability
        if (!icon) app.quit(-1);

        // create tray
        SDL_Tray *tray = SDL_CreateTray(icon, "Kit Tray Demo");

        // we can free the surface after creating the tray
        if (icon) SDL_DestroySurface(icon);

        // create options tree
        SDL_TrayMenu *menu              = SDL_CreateTrayMenu(tray);
        SDL_TrayEntry *check_entry      = SDL_InsertTrayEntryAt(menu, -1, "Enable feature", SDL_TRAYENTRY_CHECKBOX);
        SDL_TrayEntry *submenu_entry    = SDL_InsertTrayEntryAt(menu, -1, "Options", SDL_TRAYENTRY_SUBMENU);
        SDL_TrayMenu    *submenu        = SDL_CreateTraySubmenu(submenu_entry);
        SDL_TrayEntry   *sub_item1      = SDL_InsertTrayEntryAt(submenu, -1, "Sub option A", SDL_TRAYENTRY_BUTTON);
        SDL_TrayEntry   *sub_item2      = SDL_InsertTrayEntryAt(submenu, -1, "Sub option B", SDL_TRAYENTRY_BUTTON);
        SDL_TrayEntry *quit_entry       = SDL_InsertTrayEntryAt(menu, -1, "Quit", SDL_TRAYENTRY_BUTTON);

        // state
        SDL_SetTrayEntryChecked(check_entry, checkbox_state);
        SDL_SetTrayEntryCallback(check_entry, on_tray_toggle, NULL);

        // mandatory callback
        SDL_SetTrayEntryCallback(quit_entry, on_tray_quit, NULL);
        // secondary callbacks
        SDL_SetTrayEntryCallback(sub_item1, on_tray_toggle, NULL);
        SDL_SetTrayEntryCallback(sub_item2, on_tray_toggle, NULL);

        dialog.alert("A tray icon was placed in the system taskbar");
    }

    // event handler
    if( ev.emit ) {
        if (ev.emit->type == SDL_EVENT_QUIT) {
            app.quit(0);
        }
    }

    // cleanup
    if( ev.quit ) {
        SDL_DestroyTray(tray);
    }
}

const char *hints;
