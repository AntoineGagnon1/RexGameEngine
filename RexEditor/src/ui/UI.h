#pragma once

namespace RexEditor::UI
{
    // Fonts
    enum class FontScale { Small, Normal, Large };
    void PushFontScale(FontScale scale);
    void PopFontScale();

    void NewFrame();
    // Will render to the default frame buffer (id = 0)
    void RenderUI();
}