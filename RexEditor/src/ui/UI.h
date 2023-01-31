#pragma once

namespace RexEditor::UI
{
    // Fonts
    enum class FontScale { Small, Normal, Large };
    void PushFontScale(FontScale scale);
    void PopFontScale();
    void SetDefaultFont(FontScale scale);

    void PushFontColor(RexEngine::Color color);
    void PopFontColor();

    // Will render to the default frame buffer (id = 0)
    void RenderUI();

    constexpr const char* FontLocation = "assets/fonts/Inter-Regular.ttf";
}