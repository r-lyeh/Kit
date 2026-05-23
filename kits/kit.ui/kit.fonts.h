#ifndef UI2_FONT0
//#define UI2_FONT0 14.f, 1.00f, "demos/res/figtree/static/Figtree-Regular.ttf"
//#define UI2_FONT1 16.f, 1.50f, "demos/res/figtree/static/Figtree-BoldItalic.ttf"

#define UI2_FONT0 14.f, 1.00f, "demos/res/mplus-1p-medium.ttf"

//#define UI2_FONT0 14.f, 1.00f, "demos/res/B612Regular.ttf"
#define UI2_FONT1 16.f, 1.50f, "demos/res/B612BoldItalic.ttf"
#define UI2_ICONFONT    23.5f, "demos/res/MaterialSymbolsRounded_28pt-Regular.ttf"
#endif

#ifndef UI2_ICON
#define UI2_ICON(name) #name
#endif

int   ui2_loadfonts();

#if KIT_CODE
#pragma once

#include "3rd_icon_ms.h"

#ifdef __cplusplus
extern "C" {
#endif

// internals
int   ui2_loadfont(unsigned slot, float pt, float brightness, const char *resource, const void *runes);
void**ui2_fonthandles(unsigned *count);

ImFont *ui2_fonts[4] = {0};

void** ui2_fonthandles(unsigned *count) {
    if( count ) {
        *count = 0;
        for( int i = 0; i < COUNTOF(ui2_fonts); ++i ) *count += !!ui2_fonts[i];
    }
    return (void**)ui2_fonts;
}

int ui2_loadfont(unsigned slot, float pt, float brightness, const char *resource, const void *runes) {
    ImFontConfig*cfg = ImFontConfig_ImFontConfig();

    bool ok = 0;
    if( slot < COUNTOF(ui2_fonts) && pt && brightness && resource ) {
        cfg->FontDataOwnedByAtlas = false; // TTF/OTF data ownership taken by the container ImFontAtlas (will delete memory itself).
        cfg->RasterizerMultiply = brightness;
        int len; char *bin = file_read(resource, &len);
        if( bin && len )
        ok = !!(ui2_fonts[slot] = ImFontAtlas_AddFontFromMemoryTTF(igGetIO_Nil()->Fonts, bin, len, pt, cfg, (const ImWchar *)runes));
    } else {
        ok = !!(ui2_fonts[slot] = ImFontAtlas_AddFontDefault(igGetIO_Nil()->Fonts, cfg));
    }

    ImFontConfig_destroy(cfg);
    return ok;
}

int ui2_loadfonts() { // return num loaded slots

    // load primary font [0]
    // else load os fallbacks for primary font [0]
    // else load embedded default font
    if( !ui2_loadfont(0, UI2_FONT0, NULL) )
#if !KIT_MACOS
        if( !ui2_loadfont(0, 15.f, 1.f, ifdef(KIT_WINDOWS,"C:/Windows/Fonts/Arial.ttf","/usr/share/fonts/liberation/LiberationSans-Regular.ttf"),NULL) )
#endif
            if( !ui2_loadfont(0, 0, 0, 0, NULL)) {}
    
    // merge icon glyphs in primary font
    // [x] MaterialSymbolsOutlined_28pt-Regular.ttf
    // [x] MaterialSymbolsRounded_28pt-Regular.ttf
    // [x] MaterialSymbolsSharp_Filled_28pt-Regular.ttf
    float iconsize;
    int len; char *bin = file_read((iconsize = UI2_ICONFONT), &len);
    if( bin && len ) {
        static const unsigned/*ImWchar*/ runes[] = { ICON_MIN_MS, ICON_MAX_MS, 0, };
        ImFontConfig*cfg = ImFontConfig_ImFontConfig();
            cfg->FontDataOwnedByAtlas = false; // TTF/OTF data ownership taken by the container ImFontAtlas (will delete memory itself).
            cfg->OversampleH = 1; // Rasterize at higher quality for sub-pixel positioning. Note the difference between 2 and 3 is minimal. You can reduce this to 1 for large glyphs save memory. Read https://github.com/nothings/stb/blob/master/tests/oversample/README.md for details.
            cfg->OversampleV = 1; // Rasterize at higher quality for sub-pixel positioning. This is not really useful as we don't use sub-pixel positions on the Y axis.
            cfg->PixelSnapH = true;    // Align every glyph to pixel boundary. Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, you can set OversampleH/V to 1.
            // ImVec2          GlyphExtraSpacing;      // 0, 0     // Extra spacing (in pixels) between glyphs. Only X axis is supported for now.
            cfg->GlyphOffset.y = 5;  // MD:8           // 0, 0     // Offset all glyphs from this font input.
            // const ImWchar*  GlyphRanges;            // NULL     // THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE. Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list).
            // float           GlyphMinAdvanceX;       // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font
            // float           GlyphMaxAdvanceX;       // FLT_MAX  // Maximum AdvanceX for glyphs
            cfg->MergeMode = true; // Merge into previous ImFont, so you can combine multiple inputs font into one ImFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
            // unsigned int    FontBuilderFlags;       // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
            // float           RasterizerMultiply;     // 1.0f     // Linearly brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable. This is a silly thing we may remove in the future.
            // float           RasterizerDensity;      // 1.0f     // DPI scale for rasterization, not altering other font metrics: make it easy to swap between e.g. a 100% and a 400% fonts for a zooming display. IMPORTANT: If you increase this it is expected that you increase font scale accordingly, otherwise quality may look lowered.
            // ImWchar         EllipsisChar;           // -1       // Explicitly specify unicode codepoint of ellipsis character. When fonts are being merged first specified ellipsis will be used.
            ImFontAtlas_AddFontFromMemoryTTF(igGetIO_Nil()->Fonts, bin, len, iconsize, cfg, (const ImWchar*)runes);
        ImFontConfig_destroy(cfg);
    }

    // load bold font [1]
    // else load embedded default font
    if( !ui2_loadfont(1, UI2_FONT1, NULL) )
        ui2_fonts[1] = ui2_fonts[0];

    return !!ui2_fonts[0] + !!ui2_fonts[1] + !!ui2_fonts[2] + !!ui2_fonts[3];
}

int igLoadFonts() { return ui2_loadfonts(); }

#ifdef __cplusplus
}
#endif

#endif
