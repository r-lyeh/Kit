int igThemeKit(int hue07, int alt07, int nav07, int lit01, int border01, int compact01, int shape0123, int showdemo);

#if KIT_CODE
#pragma once

ImVec4 igThemeKit_dim(ImVec4 hi, int lit01) {
    float h,s,v; igColorConvertRGBtoHSV(hi.x,hi.y,hi.z, &h,&s,&v);
    ImVec4 dim = ImColor_HSV(h,s,lit01 ? v*0.65:v*0.65, hi.w).Value;
    if( hi.z > hi.x && hi.z > hi.y ) return ImVec4(dim.x,dim.y,hi.z,dim.w);
    return dim;
}

int igThemeKit(int hue07, int alt07, int nav07, int lit01, int border01, int compact01, int shape0123, int showdemo) {

    {
    static int hue, alt, nav, shape; ONCE hue = hue07, alt = alt07, nav = nav07, shape = shape0123;
    static bool light, borders, compact; ONCE light = lit01, borders = border01, compact = compact01;
    if( showdemo ) {
        if( igBegin("THEME",0,0) ) {
            int theme = 0;
            theme |= igSliderInt("Hue", &hue, 0, 7, "%d", 0);
            theme |= igSliderInt("Alt", &alt, 0, 7, "%d", 0);
            theme |= igSliderInt("Nav", &nav, 0, 7, "%d", 0);
            theme |= igSliderInt("Shape", &shape, 0, 3, "%d", 0);
            theme |= igCheckbox("Light", &light);
            theme |= igCheckbox("Compact", &compact);
            theme |= igCheckbox("Borders", &borders);
            if(theme) igThemeKit(hue["CRYOLAMP"], alt["CRYOLAMP"], nav["CRYOLAMP"], light, borders, compact, shape, 0);
        }
        igEnd();
        return 0;
    }
    }

    bool rounded = shape0123 == 2;

    const float _8 = compact01 ? 4 : 8;
    const float _4 = compact01 ? 1 : 4;
    const float _2 = compact01 ? 0.5 : 1;

    ImGuiIO *io = igGetIO_Nil();
    ImGuiStyle *style = igGetStyle();

    style->Alpha = 1.0;
    style->DisabledAlpha = 0.3;

    style->WindowPadding = ImVec2(4, _8);
    style->FramePadding = ImVec2(4, _4);
    style->ItemSpacing = ImVec2(_8, _2 + _2);
    style->ItemInnerSpacing = ImVec2(4, 4);
    style->IndentSpacing = 16;
    style->ScrollbarSize = compact01 ? 12 : 18;
    style->GrabMinSize = compact01 ? 16 : 20;

    style->WindowBorderSize = border01 ? 1 : 0;
    style->ChildBorderSize = border01 ? 1 : 0;
    style->PopupBorderSize = border01 ? 1 : 0;
    style->FrameBorderSize = border01 ? 1 : 0;

    style->WindowRounding = 4;
    style->ChildRounding = 6;
    style->FrameRounding = shape0123 == 0 ? 0 : shape0123 == 1 ? 4 : 12;
    style->PopupRounding = 4;
    style->ScrollbarRounding = rounded * 8 + 4;
    style->GrabRounding = style->FrameRounding;

    style->TabBorderSize = 0;
    style->TabBarBorderSize = 2;
    style->TabBarOverlineSize = 2;
    style->TabCloseButtonMinWidthSelected = -1; // -1:always visible, 0:visible when hovered, >0:visible when hovered if minimum width
    style->TabCloseButtonMinWidthUnselected = -1;
    style->TabRounding = rounded;

    style->CellPadding = ImVec2(8, 3);

    style->WindowTitleAlign = ImVec2(0.5, 0.5);
    style->WindowMenuButtonPosition = ImGuiDir_Right;

    style->ColorButtonPosition = ImGuiDir_Right;
    style->ButtonTextAlign = ImVec2(0.5, 0.5);
    style->SelectableTextAlign = ImVec2(0.5, 0.5);
    style->SeparatorTextAlign.x = 1.00; // 0.1<left 1.0>right
    style->SeparatorTextBorderSize = compact01 ? 2 : 3;
    style->SeparatorTextPadding = ImVec2(0, 0);

    style->WindowMinSize = ImVec2(32, 16);
    style->ColumnsMinSpacing = 6;

    // diamond sliders
    style->CircleTessellationMaxError = shape0123 == 3 ? 4.00f : 0.30f;

    const ImVec4 cyan    = ImVec4(000/255.0, 192/255.0, 255/255.0, 1.00);
    const ImVec4 red     = ImVec4(240/255.0, 000/255.0,  16/255.0, 1.00);
    const ImVec4 yellow  = ImVec4(240/255.0, 210/255.0, 000/255.0, 1.00);
    const ImVec4 orange  = ImVec4(255/255.0, 144/255.0, 000/255.0, 1.00);
    const ImVec4 lime    = ImVec4(192/255.0, 255/255.0, 000/255.0, 1.00);
    const ImVec4 aqua    = ImVec4(000/255.0, 255/255.0, 192/255.0, 1.00);
    const ImVec4 magenta = ImVec4(255/255.0, 000/255.0,  88/255.0, 1.00);
    const ImVec4 purple  = ImVec4(192/255.0, 000/255.0, 255/255.0, 1.00);

    ImVec4 alt = cyan;
    /**/ if( alt07 == 0 || alt07 == 'C' ) alt = cyan;
    else if( alt07 == 1 || alt07 == 'R' ) alt = red;
    else if( alt07 == 2 || alt07 == 'Y' ) alt = yellow;
    else if( alt07 == 3 || alt07 == 'O' ) alt = orange;
    else if( alt07 == 4 || alt07 == 'L' ) alt = lime;
    else if( alt07 == 5 || alt07 == 'A' ) alt = aqua;
    else if( alt07 == 6 || alt07 == 'M' ) alt = magenta;
    else if( alt07 == 7 || alt07 == 'P' ) alt = purple;
    if( lit01 ) alt = igThemeKit_dim(alt, lit01);

    ImVec4 hi = cyan, lo = igThemeKit_dim(cyan, lit01);
    /**/ if( hue07 == 0 || hue07 == 'C' ) lo = igThemeKit_dim( hi = cyan, lit01 );
    else if( hue07 == 1 || hue07 == 'R' ) lo = igThemeKit_dim( hi = red, lit01 );
    else if( hue07 == 2 || hue07 == 'Y' ) lo = igThemeKit_dim( hi = yellow, lit01 );
    else if( hue07 == 3 || hue07 == 'O' ) lo = igThemeKit_dim( hi = orange, lit01 );
    else if( hue07 == 4 || hue07 == 'L' ) lo = igThemeKit_dim( hi = lime, lit01 );
    else if( hue07 == 5 || hue07 == 'A' ) lo = igThemeKit_dim( hi = aqua, lit01 );
    else if( hue07 == 6 || hue07 == 'M' ) lo = igThemeKit_dim( hi = magenta, lit01 );
    else if( hue07 == 7 || hue07 == 'P' ) lo = igThemeKit_dim( hi = purple, lit01 );

    ImVec4 nav = orange;
    /**/ if( nav07 == 0 || nav07 == 'C' ) nav = cyan;
    else if( nav07 == 1 || nav07 == 'R' ) nav = red;
    else if( nav07 == 2 || nav07 == 'Y' ) nav = yellow;
    else if( nav07 == 3 || nav07 == 'O' ) nav = orange;
    else if( nav07 == 4 || nav07 == 'L' ) nav = lime;
    else if( nav07 == 5 || nav07 == 'A' ) nav = aqua;
    else if( nav07 == 6 || nav07 == 'M' ) nav = magenta;
    else if( nav07 == 7 || nav07 == 'P' ) nav = purple;
    if( lit01 ) nav = igThemeKit_dim(nav, lit01);

    float inc = lit01*0.15;
    ImVec4
        link  = ImVec4(0.26, 0.59, 0.98, 1.00),
        grey0 = ImVec4(0.03+inc, 0.03+inc, 0.05+inc, 1.00),
        grey1 = ImVec4(0.07+inc, 0.08+inc, 0.10+inc, 1.00),
        grey2 = ImVec4(0.10+inc, 0.11+inc, 0.13+inc, 1.00),
        grey3 = ImVec4(0.15+inc, 0.16+inc, 0.19+inc, 1.00),
        grey4 = ImVec4(0.20+inc, 0.21+inc, 0.24+inc, 1.00),
        grey5 = ImVec4(0.24+inc, 0.25+inc, 0.28+inc, 1.00);

#define Luma(v,a) ImVec4((v)/100.,(v)/100.,(v)/100.,(a)/100.)
#define tint(c,a) ImVec4(c.x, c.y, c.z, a)

    style->Colors[ImGuiCol_Text]                      = Luma(100,100);
    style->Colors[ImGuiCol_TextDisabled]              = Luma(39,100);
    style->Colors[ImGuiCol_WindowBg]                  = grey1;
    style->Colors[ImGuiCol_ChildBg]                   = ImVec4(0.09+inc, 0.10+inc, 0.12+inc, 1.00);
    style->Colors[ImGuiCol_PopupBg]                   = grey1;
    style->Colors[ImGuiCol_Border]                    = grey4;
    style->Colors[ImGuiCol_BorderShadow]              = grey1;
    style->Colors[ImGuiCol_FrameBg]                   = ImVec4(0.11+inc, 0.13+inc, 0.15+inc, 1.00);
    style->Colors[ImGuiCol_FrameBgHovered]            = grey4;
    style->Colors[ImGuiCol_FrameBgActive]             = grey4;
    style->Colors[ImGuiCol_TitleBg]                   = io->ConfigFlags & ImGuiConfigFlags_DockingEnable ? grey0 : grey3;
    style->Colors[ImGuiCol_TitleBgActive]             = io->ConfigFlags & ImGuiConfigFlags_DockingEnable ? grey2 : lo;
    style->Colors[ImGuiCol_TitleBgCollapsed]          = grey1;
    style->Colors[ImGuiCol_MenuBarBg]                 = grey2;
    style->Colors[ImGuiCol_ScrollbarBg]               = grey0;
    style->Colors[ImGuiCol_ScrollbarGrab]             = grey3;
    style->Colors[ImGuiCol_ScrollbarGrabHovered]      = lo;
    style->Colors[ImGuiCol_ScrollbarGrabActive]       = hi;
    style->Colors[ImGuiCol_CheckMark]                 = alt;
    style->Colors[ImGuiCol_SliderGrab]                = lo;
    style->Colors[ImGuiCol_SliderGrabActive]          = hi;
    style->Colors[ImGuiCol_Button]                    = grey4; // ImVec4(0.10, 0.11, 0.14, 1.00);
    style->Colors[ImGuiCol_ButtonHovered]             = lo;
    style->Colors[ImGuiCol_ButtonActive]              = grey5;
    style->Colors[ImGuiCol_Header]                    = grey3;
    style->Colors[ImGuiCol_HeaderHovered]             = lo;
    style->Colors[ImGuiCol_HeaderActive]              = tint(hi, 0.75f);
    style->Colors[ImGuiCol_Separator]                 = ImVec4(0.13+inc, 0.15+inc, 0.19+inc, 1.00);
    style->Colors[ImGuiCol_SeparatorHovered]          = lo;
    style->Colors[ImGuiCol_SeparatorActive]           = hi;
    style->Colors[ImGuiCol_ResizeGrip]                = Luma(15,100);
    style->Colors[ImGuiCol_ResizeGripHovered]         = lo;
    style->Colors[ImGuiCol_ResizeGripActive]          = hi;
    style->Colors[ImGuiCol_InputTextCursor]           = Luma(100,100);
    style->Colors[ImGuiCol_TabHovered]                = grey3;
    style->Colors[ImGuiCol_Tab]                       = grey1;
    style->Colors[ImGuiCol_TabSelected]               = grey3;
    style->Colors[ImGuiCol_TabSelectedOverline]       = hi;
    style->Colors[ImGuiCol_TabDimmed]                 = grey1;
    style->Colors[ImGuiCol_TabDimmedSelected]         = grey1;
    style->Colors[ImGuiCol_TabDimmedSelectedOverline] = lo;
    style->Colors[ImGuiCol_DockingPreview]            = grey1;
    style->Colors[ImGuiCol_DockingEmptyBg]            = Luma(20,100);
    style->Colors[ImGuiCol_PlotLines]                 = grey5;
    style->Colors[ImGuiCol_PlotLinesHovered]          = lo;
    style->Colors[ImGuiCol_PlotHistogram]             = grey5;
    style->Colors[ImGuiCol_PlotHistogramHovered]      = lo;
    style->Colors[ImGuiCol_TableHeaderBg]             = grey0;
    style->Colors[ImGuiCol_TableBorderStrong]         = grey0;
    style->Colors[ImGuiCol_TableBorderLight]          = grey0;
    style->Colors[ImGuiCol_TableRowBg]                = grey3;
    style->Colors[ImGuiCol_TableRowBgAlt]             = grey2;
    style->Colors[ImGuiCol_TextLink]                  = link;
    style->Colors[ImGuiCol_TextSelectedBg]            = Luma(39,100);
    style->Colors[ImGuiCol_TreeLines]                 = Luma(39,100);
    style->Colors[ImGuiCol_DragDropTarget]            = nav;
    style->Colors[ImGuiCol_NavCursor]                 = nav;
    style->Colors[ImGuiCol_NavWindowingHighlight]     = lo;
    style->Colors[ImGuiCol_NavWindowingDimBg]         = Luma(0,63);
    style->Colors[ImGuiCol_ModalWindowDimBg]          = Luma(0,63);

    if( lit01 ) {
        for(int i = 0; i < ImGuiCol_COUNT; i++) {
            float H, S, V;
            ImVec4 *col = &style->Colors[i];
            igColorConvertRGBtoHSV( col->x, col->y, col->z, &H, &S, &V );
            if( S < 0.5 ) V = 1.0 - V, S *= 0.15;
            igColorConvertHSVtoRGB( H, S, V, &col->x, &col->y, &col->z );
        }
    }

    #undef tint
    #undef Luma
    return 1;
}

int igThemeKit_(const char *theme) {
    const char *colors = "CRYOLAMP";
    int hue07 = theme[0] >= 'A' ? strchr(colors, theme[0]) - colors : theme[0] - '0';
    int alt07 = theme[1] >= 'A' ? strchr(colors, theme[1]) - colors : theme[1] - '0';
    int nav07 = theme[2] >= 'A' ? strchr(colors, theme[2]) - colors : theme[2] - '0';
    int lit01 = theme[3] - '0';
    int compact01 = theme[4] - '0';
    int borders01 = theme[5] - '0';
    int shape0123 = theme[6] - '0';
    return igThemeKit(hue07, alt07, nav07, lit01, compact01, borders01, shape0123, 0);
}

#endif
