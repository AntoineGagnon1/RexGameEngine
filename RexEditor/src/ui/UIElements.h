#pragma once

#include <RexEngine.h>

#include <string>
#include <stack>

namespace RexEditor::UI
{
    //
    // Interfaces
    //
    class Hoverable
    {
    public:
        bool IsHovered() const { return m_hovered; }

    protected:

        // Will set m_hovered = ImGui::IsItemHovered()
        void CacheHovered();

        bool m_hovered; // Needs to be set, either manually or by calling CacheHovered()
    };

    class Clickable : public Hoverable
    {
    public:

        virtual bool IsClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left) const;
        virtual bool IsDoubleClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left) const;
    };

    //
    // Free Functions
    //
    
    // Put the next element on the same line as the last
    void SameLine();


    //
    // Window
    //
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

    WindowSetting operator | (WindowSetting lhs, WindowSetting rhs);
    WindowSetting& operator |= (WindowSetting& lhs, WindowSetting rhs);

	// Usage : 
	// If(Window w(title); w.IsVisible()) {...}
    class Window : public Hoverable
	{
    public:
        // Closed to nullptr will display no X to close the window
        Window(const std::string& title, bool* open = nullptr, WindowSetting settings = WindowSetting::None);
		~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Draw a texture that fills the window
        void DrawFullWindowTexture(const RexEngine::Texture& texture);

        bool IsVisible() const { return m_visible; }
        RexEngine::Vector2 Size() const;
        bool IsFocused() const;

    private:
        bool m_visible;
	};


    //
    // Anchors
    //
    enum class AnchorPos {
        None = 0,
        Top = 1 << 0,
        Bottom = 1 << 1,
        Right = 1 << 2,
        Center = 1 << 3,
        Left = 1 << 4,

        TopLeft = Top | Left,
        TopCenter = Top | Center,
        TopRight = Top | Right,
        Default = Left,
        BottomLeft = Bottom | Left,
        BottomCenter = Bottom | Center,
        BottomRight = Bottom | Right
    };

    // Usage : 
    //
    // {
    // Anchor a(AnchorPos);
    // UIElement(...);
    // UIElement(...);
    // } // The destructor will revert back to default
    // 
    // This will always stack vertically
    // Nested anchors wont combine, the most recent will be used
    class Anchor
    {
    public:
        Anchor(AnchorPos anchor);
        ~Anchor();

        // Add an offset to the current anchor
        // if there is no current anchor it will add the offset
        // to the cursor
        static void AddOffset(Vector2 offset);

        // Used internally, sets the cursor at the right place to
        // draw an element of size = size
        static void SetCursorPos(Vector2 size, bool moveCursor = true);

    private:
        AnchorPos m_anchor;
        Vector2 m_startPos;
        Vector2 m_currentPos;

        inline static std::stack<Anchor*> s_currentAnchors;
    };

    //
    // Text Input
    //
    // Only hovered when over the input part
    class TextInput : public Clickable
    {
    public:
        TextInput(const std::string& label, size_t maxSize);

        std::string Text() const { return m_text; }

    private:
        std::string m_text;
    };


    //
    // Button
    //
    // Usage : 
    //If(Button b(title); b.IsClicked()) {...}
    class Button : public Clickable
    {
    public:
        Button(const std::string& label);

        bool IsClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left) const override;

    private:
        bool m_clicked;
    };

    // A selectable made up of an image and a label (under the image)
    class Icon : public Clickable
    {
    public:
        Icon(const std::string& label, const RexEngine::Texture& icon, Vector2 iconSize);
    };


    //
    // Sliders
    //
    template<typename T>
    class Slider : public Hoverable
    {
    public:
        
        Slider(T& value) : m_valueInner(static_cast<T>(0)), m_value(value) {}
        Slider() : m_valueInner(static_cast<T>(0)), m_value(m_valueInner) {}

        T Value() const { return m_value; }

    protected:
        T& m_value; // Can be m_valueInner or an external value passed by the constructor
    private:
        T m_valueInner;
    };

    class FloatSlider : public Slider<float>
    {
    public:

        FloatSlider(const std::string& label, float min, float max, float width, float& value, int precision = 1);
        FloatSlider(const std::string& label, float min, float max, float width, int precision = 1);
    };


    //
    // Trees
    //
    enum class TreeNodeFlags
    {
        None = 0,
        Selected = 1 << 0,   // Draw as selected
        Framed = 1 << 1,   // Draw frame with background (e.g. for CollapsingHeader)
        AllowItemOverlap = 1 << 2,   // Hit testing to allow subsequent widgets to overlap this one
        NoTreePushOnOpen = 1 << 3,   // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
        NoAutoOpenOnLog = 1 << 4,   // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
        DefaultOpen = 1 << 5,   // Default node to be open
        OpenOnDoubleClick = 1 << 6,   // Need double-click to open node
        OpenOnArrow = 1 << 7,   // Only open when clicking on the arrow part. If ImGuiTreeNodeFlags_OpenOnDoubleClick is also set, single-click arrow or double-click all box to open.
        Leaf = 1 << 8,   // No collapsing, no arrow (use as a convenience for leaf nodes).
        Bullet = 1 << 9,   // Display a bullet instead of arrow
        FramePadding = 1 << 10,  // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding().
        SpanAvailWidth = 1 << 11,  // Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line. In the future we may refactor the hit system to be front-to-back, allowing natural overlaps and then this can become the default.
        SpanFullWidth = 1 << 12,  // Extend hit box to the left-most and right-most edges (bypass the indented area).
        NavLeftJumpsBackHere = 1 << 13,  // (WIP) Nav: left direction may move to this TreeNode() from any of its child (items submitted between TreeNode and TreePop)
        
        CollapsingHeader = Framed | NoTreePushOnOpen | NoAutoOpenOnLog,
    };

    TreeNodeFlags operator | (TreeNodeFlags lhs, TreeNodeFlags rhs);
    TreeNodeFlags& operator |= (TreeNodeFlags& lhs, TreeNodeFlags rhs);

    // Usage : 
    // If(TreeNode n(title); n.IsOpen()) {...}
    class TreeNode : public Clickable
    {
    public:
        TreeNode(const std::string& label, TreeNodeFlags flags);
        ~TreeNode();

        bool IsOpen() const { return m_open; }

    private:
        bool m_open;
    };


    //
    // Tables
    // Tables are like excel sheets, with a fixed number of columns but 
    // a variable number of rows
    // Does NOT respect Anchors
    // Usage : if(Table t("nameTable", 3); t.IsVisible()) {...}
    class Table
    {
    public:
        // The name is only for the id, it wont be visible
        Table(const std::string& name, int nbCols);
        ~Table();

        void SetCellPadding(Vector2Int padding);
        // Move to the next element
        void NextElement();

        bool IsVisible() const { return m_visible; }

    private:
        bool m_visible;
    };


    //
    // Text
    //
    class Text : public Clickable
    {
    public:
        Text(const std::string& text);
    };



    //
    // Menu
    //
    // Usage : if(Menu m("Menu"); m.IsOpen()) {...}
    class Menu
    {
    public:
        // enabled == false makes a gray non clickable menu
        Menu(const std::string& label, bool enabled = true);
        ~Menu();

        bool IsOpen() const { return m_open; }

    private:
        bool m_open;
    };

    class MenuItem : public Clickable
    {
    public:
        MenuItem(const std::string& name, bool enabled);

        bool IsClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left) const override;

    private:
        bool m_clicked;
    };
}