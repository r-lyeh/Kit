#ifdef __cplusplus
extern "C" {
#endif

void igTextWithHoverColor(ImU32 col, ImVec2 indents_offon, const char* text_begin) {
    ImGuiContext* g = igGetCurrentContext(); //*GImGui;
    ImGuiWindow* window = igGetCurrentWindow();
    if (window->SkipItems)
        return;

    const char *text_end = text_begin + strlen(text_begin);

    // Layout
    const ImVec2 text_pos = ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
    const ImVec2 text_size = igCalcTextSize(text_begin, text_end, false, -1);
    ImRect bb = ImRect(text_pos.x, text_pos.y, text_pos.x + text_size.x, text_pos.y + text_size.y);
    igItemSize_Vec2(text_size, 0.0f);
    if (!igItemAdd(bb, 0, NULL, 0))
        return;

    // Render
    bool hovered = igIsItemHovered(0);
    if (hovered) igPushStyleColor_U32(ImGuiCol_Text, col);
    igRenderText(ImVec2(bb.Min.x + (hovered ? indents_offon.y : indents_offon.x), bb.Min.y + 0), text_begin, text_end, false);
    if (hovered) igPopStyleColor(1);
}

int igCurrentWindowStackSize(void) {
    return igGetCurrentContext()->CurrentWindowStack.Size;
}

int igIsAnyWindowHovered(void) {
    return igIsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

float igGetWindowPosX(void) { return igGetWindowPos().x; }

float igGetWindowPosY(void) { return igGetWindowPos().y; }

int igRightAlign(const char* str_id) {
    if(igBeginTable(str_id, 2, ImGuiTableFlags_SizingFixedFit, ImVec2(-1,0), 0)) {
        igTableSetupColumn("a", ImGuiTableColumnFlags_WidthStretch, 0,0);

        igTableNextColumn();
        igTableNextColumn();
        return true;
    }
    return false;
}

int igRightAlignEnd() {
    return igEndTable(), false;
}

/*
int igMouseLastCursor() {
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    for( int i = ImGuiMouseCursor_Arrow; i < ImGuiMouseCursor_COUNT; ++i ) 
        if( bd->MouseLastCursor == bd->MouseCursors[i] ) return i;
    return ImGuiMouseCursor_Arrow;
}
*/

int igGetHoveredWindowID(void) {
    ImGuiWindow *win = igGetCurrentContext()->HoveredWindow;
    return win ? win->ID : 0;
}

#ifdef __cplusplus
}
#endif
