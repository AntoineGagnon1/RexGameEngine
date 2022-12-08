#pragma once

#include <RexEngine.h>
#include <string>
#include <vector>

#include "EditorEvents.h"

struct ImFont;

namespace RexEditor
{
    enum class WindowSetting // Used as flags
    {
        None = 0,
        NoTitleBar = 1 << 0,   // Disable title-bar
        NoResize = 1 << 1,   // Disable user resizing with the lower-right grip
        NoMove = 1 << 2,   // Disable user moving the window
        NoScrollbar = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
        NoScrollWithMouse = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
        NoCollapse = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
        AlwaysAutoResize = 1 << 6,   // Resize every window to its content every frame
        NoBackground = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
        NoSavedSettings = 1 << 8,   // Never load/save settings in .ini file
        NoMouseInputs = 1 << 9,   // Disable catching mouse, hovering test with pass through.
        MenuBar = 1 << 10,  // Has a menu-bar
        HorizontalScrollbar = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
        NoFocusOnAppearing = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
        NoBringToFrontOnFocus = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
        AlwaysVerticalScrollbar = 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
        AlwaysHorizontalScrollbar = 1 << 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
        AlwaysUseWindowPadding = 1 << 16,  // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
        NoNavInputs = 1 << 18,  // No gamepad/keyboard navigation within the window
        NoNavFocus = 1 << 19,  // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
        UnsavedDocument = 1 << 20,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
        NoDocking = 1 << 21,  // Disable docking of this window

        NoNav = NoNavInputs | NoNavFocus,
        NoDecoration = NoTitleBar | NoResize | NoScrollbar | NoCollapse,
        NoInputs = NoMouseInputs | NoNavInputs | NoNavFocus,
    };

    inline WindowSetting operator | (WindowSetting lhs, WindowSetting rhs)
    {
        using T = std::underlying_type_t <WindowSetting>;
        return static_cast<WindowSetting>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    inline WindowSetting& operator |= (WindowSetting& lhs, WindowSetting rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    enum class Alignement { Left, Center, Right };
    // Default is the normal top to bottom mode
    enum class VerticalPos { Default, Bottom };

    enum class FontScale { Small, Normal, Large };

	class Imgui
	{
	public:
		
		static void NewFrame();
		static void RenderGui();

        [[nodiscard("Usage : if(BeginWindow()){...}")]] 
        static bool BeginWindow(const std::string& name, bool& open, WindowSetting settings = WindowSetting::None);
		static void EndWindow();
        static Vector2Int GetWindowSize();
        static bool IsWindowFocused();
        static bool IsWindowHovered();

        // Draw a texture that fils the window
        static void DrawFullWindowTexture(const RexEngine::Texture& texture);

        // Menu bar
        static bool BeginMainMenuBar();
        static void EndMainMenuBar();

        static bool BeginMenu(const std::string& name, bool enabled = true);
        static void EndMenu();

        static void MenuItem(const std::string& name, std::function<void()> toCall, bool enabled = true);

        // User Inputs
        static void TextInput(const std::string& label, std::string& result, size_t maxSize);
        [[nodiscard("Usage : if(Button()){...}")]]
        static bool Button(const std::string& label, Alignement align = Alignement::Left, VerticalPos vPos = VerticalPos::Default);
        static void IconButton(const std::string& text, const RexEngine::Texture& icon, Vector2Int size, bool& clicked, bool& doubleClicked);
        static void SliderFloat(const std::string& label, float min, float max, float& value, int width, int precision = 1, VerticalPos vPos = VerticalPos::Default);
        static bool IsItemClicked(MouseButton mouseButton = RexEngine::MouseButton::Left);

        // Fields
        static void Text(const std::string& text);
        // openArrowOnly : only open if the arrow is pressed
        static bool TreeNode(const std::string& text, bool leaf = false, bool openArrowOnly = true, bool selected = false);
        static void TreePop();

        // Formating
        static void SameLine();
        static void Space();
        static void Indent(float amount = 0.0f);
        static bool BeginTable(const std::string& name, int nbCols, Vector2Int padding);
        static void EndTable();
        static void TableNextElement();


        // Font
        static void PushFontScale(FontScale scale);
        static void PopFontScale();

	private:
        static void Init();
        static void Close();

        // 0 = small, 1 = normal, 2 = large
        inline static std::vector<ImFont*> s_fonts;

        RE_STATIC_CONSTRUCTOR({
            EditorEvents::OnEditorStart().Register<&Init>();
            EditorEvents::OnEditorStop().Register<&Close>();
        });

	};
}