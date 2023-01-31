#pragma once

#include <RexEngine.h>

#include <string>
#include <stack>
#include <filesystem>
#include <vector>

#include "SystemDialogs.h"

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

    enum class MouseAction { Clicked, Released, Pressed };

    class Clickable : public Hoverable
    {
    public:

        virtual bool IsClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left, MouseAction action = MouseAction::Clicked) const;
        virtual bool IsDoubleClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left) const;
    };

    //
    // Free Functions
    //
    
    // Put the next element on the same line as the last
    void SameLine();
    
    // A thin separator line
    void Separator();

    // An empty line, the height of a text line
    void EmptyLine();

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
    class Window : public Clickable
	{
    public:
        // Closed to nullptr will display no X to close the window
        Window(const std::string& title, bool* open = nullptr, WindowSetting settings = WindowSetting::None);
		~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Draw a texture that fills the window
        void DrawFullWindowTexture(const Texture& texture);

        bool IsVisible() const { return m_visible; }
        RexEngine::Vector2 Size() const;
        bool IsFocused() const;

        static void SetFocused(const std::string& title);

    private:
        bool m_visible;
        std::string m_title;
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
        static void AddOffset(RexEngine::Vector2 offset);

        // Used internally, sets the cursor at the right place to
        // draw an element of size = size
        static void SetCursorPos(RexEngine::Vector2 size, bool moveCursor = true);

    private:
        AnchorPos m_anchor;
        RexEngine::Vector2 m_startPos;
        RexEngine::Vector2 m_currentPos;

        inline static std::stack<Anchor*> s_currentAnchors;
    };

    //
    // Input Fields
    //
    
    // Base class for input types (TextInput, Vector3Input, ...)
    template<typename T>
    class Input : public Clickable
    {
    public:

        Input(T& value) : m_value(value), m_changed(false) {}

        T Value() const { return m_value; }

        bool HasChanged() const { return m_changed; }

    protected:
        T& m_value;
        bool m_changed; // Needs to be set by the child class
    };
    
    // Only hovered when over the input part
    class TextInput : public Input<std::string>
    {
    public:
        // maxSize : max size of the input string
        TextInput(const std::string& label, size_t maxSize, std::string& value, bool readOnly = false);
    };

    class Vector2IntInput : public Input<RexEngine::Vector2Int>
    {
    public:
        Vector2IntInput(const std::string& label, Vector2Int& value);
    };

    class Vector2Input : public Input<RexEngine::Vector2>
    {
    public:
        Vector2Input(const std::string& label, Vector2& value);
    };

    class Vector3Input : public Input<RexEngine::Vector3>
    {
    public:
        Vector3Input(const std::string& label, Vector3& value);
    };

    class Vector4Input : public Input<RexEngine::Vector4>
    {
    public:
        Vector4Input(const std::string& label, Vector4& value);
    };

    class Matrix4Input : public Input<RexEngine::Matrix4>
    {
    public:
        Matrix4Input(const std::string& label, Matrix4& value);
    };

 
    namespace Internal
    {
        // Non template functon for AssetInput
        // returns the new path, or an empty path if the asset has not changed
        std::filesystem::path AssetInputUI(const std::string& label, const std::string& assetType, const std::vector<std::string>& filter, const Guid& currentGuid, bool& hovered);

        // ratio is y/x
        std::filesystem::path TextureInputUI(const std::string& label, const std::string& assetType, RenderApi::TextureID id, float ratio, const std::vector<std::string>& filter, const Guid& currentGuid, bool& hovered);
    }

    // Used to select an asset, 
    // supports both drag/drop and a file selection prompt
    // The function std::string GetAssetFilter<T>(const Asset<T>& value) must be implemented
    template<typename T>
    class AssetInput : public Input<Asset<T>>
    {
    private:
        using AssetType = Asset<T>;
    public:
        AssetInput(const std::string& label, AssetType& value)
            : Input<AssetType>(value)
        {
            auto newPath = Internal::AssetInputUI(label, AssetTypes::GetAssetType<T>().name, SystemDialogs::GetAssetTypeFilter<T>(), value.GetAssetGuid(), Hoverable::m_hovered);

            if (!newPath.empty())
            {
                auto newGuid = AssetManager::GetAssetGuidFromPath(newPath);
                if (newGuid == Guid::Empty)
                {// Load the asset
                    newGuid = Guid::Generate();
                    if (!AssetManager::AddAsset<T>(newGuid, newPath))
                    {
                        SystemDialogs::Alert("Error", "Could not load the asset !");
                        return;
                    }
                }

                // Replace the asset
                value = AssetManager::GetAsset<T>(newGuid);
                Input<AssetType>::m_changed = true;
            }
        }
    };

    template<>
    class AssetInput<Texture> : public Input<Asset<Texture>>
    {
    private:
        using AssetType = Asset<Texture>;
    public:
        AssetInput(const std::string& label, AssetType& value)
            : Input<AssetType>(value)
        {
            RenderApi::TextureID id = RenderApi::InvalidTextureID;
            float ratio = 1.0f;
            if (value)
            {
                id = value->GetId();
                ratio = (float)value->Size().y / (float)value->Size().x;
            }

            auto newPath = Internal::TextureInputUI(label, AssetTypes::GetAssetType<Texture>().name, id, ratio, SystemDialogs::GetAssetTypeFilter<Texture>(), value.GetAssetGuid(), Hoverable::m_hovered);

            if (!newPath.empty())
            {
                auto newGuid = AssetManager::GetAssetGuidFromPath(newPath);
                if (newGuid == Guid::Empty)
                {// Load the asset
                    newGuid = Guid::Generate();
                    if (!AssetManager::AddAsset<Texture>(newGuid, newPath))
                    {
                        SystemDialogs::Alert("Error", "Could not load the asset !");
                        return;
                    }
                }

                // Replace the asset
                value = AssetManager::GetAsset<Texture>(newGuid);
                Input<AssetType>::m_changed = true;
            }
        }
    };

    // A char (number, not character) input
    class ByteInput : public Input<char>
    {
    public:
        ByteInput(const std::string& label, char& value);
    };

    class IntInput : public Input<int>
    {
    public:
        IntInput(const std::string& label, int& value);
    };

    class FloatInput : public Input<float>
    {
    public:
        FloatInput(const std::string& label, float& value);
    };

    class ColorInput : public Input<Color>
    {
    public:
        ColorInput(const std::string& label, Color& value, bool useAlpha = true);
    };

    // a bool input
    class CheckBox : public Input<bool>
    {
    public:
        CheckBox(const std::string& label, bool& value);
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

        bool IsClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left, MouseAction action = MouseAction::Clicked) const override;

    private:
        bool m_clicked;
    };

    class ImageButton : public Clickable
    {
    public:
        // size is the size of the whole button, padding included
        ImageButton(const std::string& id, const RexEngine::Texture& texture, RexEngine::Vector2 size);

        bool IsClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left, MouseAction action = MouseAction::Clicked) const override;

    private:
        bool m_clicked;
    };

    // A selectable made up of an image and a label (under the image)
    class Icon : public Clickable
    {
    public:
        Icon(const std::string& label, const RexEngine::Texture& icon, RexEngine::Vector2 iconSize);

        bool IsClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left, MouseAction action = MouseAction::Clicked) const override;

    private:
        bool m_clicked;
    };


    //
    // Sliders
    //
    class FloatSlider : public Input<float>
    {
    public:

        // Set width to -1 for normal input (full width)
        FloatSlider(const std::string& label, float min, float max, float width, float& value, int precision = 1);
    };


    //
    // ComboBox
    //
    // A drop down selection menu
    class ComboBox : public Input<int>
    {
    public:
        ComboBox(const std::string& label, std::vector<std::string> options, int& selected);
    };

    // ComboBox for enums
    template<typename T>
    class ComboBoxEnum : public Input<T>
    {
    public:
        ComboBoxEnum(const std::string& label, std::vector<std::string> options, T& selected)
            : Input<T>(selected)
        {
            int value = static_cast<int>(selected);
            ComboBox combo(label, options, value);
            Hoverable::CacheHovered();
            selected = static_cast<T>(combo.Value());
            Input<T>::m_changed = combo.HasChanged();
        }
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
        TreeNode(const std::string& label, TreeNodeFlags flags = TreeNodeFlags::None);
        ~TreeNode();

        bool IsOpen() const { return m_open; }

    private:
        bool m_open;
        bool m_shouldPop;
    };


    enum class TableFlags
    {
        // Features
        None = 0,
        Resizable = 1 << 0,   // Enable resizing columns.
        Reorderable = 1 << 1,   // Enable reordering columns in header row (need calling TableSetupColumn() + TableHeadersRow() to display headers)
        Hideable = 1 << 2,   // Enable hiding/disabling columns in context menu.
        Sortable = 1 << 3,   // Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see ImGuiTableFlags_SortMulti and ImGuiTableFlags_SortTristate.
        NoSavedSettings = 1 << 4,   // Disable persisting columns order, width and sort settings in the .ini file.
        ContextMenuInBody = 1 << 5,   // Right-click on columns body/contents will display table context menu. By default it is available in TableHeadersRow().
        // Decorations
        RowBg = 1 << 6,   // Set each RowBg color with ImGuiCol_TableRowBg or ImGuiCol_TableRowBgAlt (equivalent of calling TableSetBgColor with ImGuiTableBgFlags_RowBg0 on each row manually)
        BordersInnerH = 1 << 7,   // Draw horizontal borders between rows.
        BordersOuterH = 1 << 8,   // Draw horizontal borders at the top and bottom.
        BordersInnerV = 1 << 9,   // Draw vertical borders between columns.
        BordersOuterV = 1 << 10,  // Draw vertical borders on the left and right sides.
        BordersH = BordersInnerH | BordersOuterH, // Draw horizontal borders.
        BordersV = BordersInnerV | BordersOuterV, // Draw vertical borders.
        BordersInner = BordersInnerV | BordersInnerH, // Draw inner borders.
        BordersOuter = BordersOuterV | BordersOuterH, // Draw outer borders.
        Borders = BordersInner | BordersOuter,   // Draw all borders.
        NoBordersInBody = 1 << 11,  // [ALPHA] Disable vertical borders in columns Body (borders will always appear in Headers). -> May move to style
        NoBordersInBodyUntilResize = 1 << 12,  // [ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers). -> May move to style
        // Sizing Policy (read above for defaults)
        SizingFixedFit = 1 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.
        SizingFixedSame = 2 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable ImGuiTableFlags_NoKeepColumnsVisible.
        SizingStretchProp = 3 << 13,  // Columns default to _WidthStretch with default weights proportional to each columns contents widths.
        SizingStretchSame = 4 << 13,  // Columns default to _WidthStretch with default weights all equal, unless overridden by TableSetupColumn().
        // Sizing Extra Options
        NoHostExtendX = 1 << 16,  // Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.
        NoHostExtendY = 1 << 17,  // Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.
        NoKeepColumnsVisible = 1 << 18,  // Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.
        PreciseWidths = 1 << 19,  // Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.
        // Clipping
        NoClip = 1 << 20,  // Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with TableSetupScrollFreeze().
        // Padding
        PadOuterX = 1 << 21,  // Default if BordersOuterV is on. Enable outermost padding. Generally desirable if you have headers.
        NoPadOuterX = 1 << 22,  // Default if BordersOuterV is off. Disable outermost padding.
        NoPadInnerX = 1 << 23,  // Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off).
        // Scrolling
        ScrollX = 1 << 24,  // Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size. Changes default sizing policy. Because this creates a child window, ScrollY is currently generally recommended when using ScrollX.
        ScrollY = 1 << 25,  // Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size.
        // Sorting
        SortMulti = 1 << 26,  // Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).
        SortTristate = 1 << 27,  // Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).
    };

    TableFlags operator | (TableFlags lhs, TableFlags rhs);
    TableFlags& operator |= (TableFlags& lhs, TableFlags rhs);

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
        Table(const std::string& name, int nbCols, TableFlags flags = TableFlags::None);
        ~Table();

        void SetCellPadding(RexEngine::Vector2Int padding);
        // Move to the next element
        void NextElement();
        void PreviousElement(); // Move back once

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

    class FramedText : public Clickable
    {
    public:
        // Will take the full width if padding.x == -1
        FramedText(const std::string& text, bool border = false, RexEngine::Vector2 padding = {0.0f, 0.0f});
    };

    //
    // Image
    //
    class Image : public Clickable
    {
    public:
        Image(const Texture& texture, Vector2 size = {-1,-1});
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
        MenuItem(const std::string& name, bool enabled = true);

        bool IsClicked(RexEngine::MouseButton mouseButton = RexEngine::MouseButton::Left, MouseAction action = MouseAction::Clicked) const override;

    private:
        bool m_clicked;
    };


    //
    // Popup
    //
    // Usage :
    // if(...) 
    //   Popup::Open("name");
    //
    // if(Popup p("name"); p.IsOpen()) {...}
    class Popup
    {
    public:
        Popup(const std::string& name);
        ~Popup();

        bool IsOpen() const { return m_open; }

        static void Open(const std::string& name);

    private:
        bool m_open;
    };

    // Opens on right click on the window
    // Usage :
    // if(ContextMenu c("name"); c.IsOpen()) {...}
    class ContextMenu
    {
    public:
        ContextMenu(const std::string& name);
        ~ContextMenu();

        bool IsOpen() const { return m_open; }

    private:
        bool m_open;

    };


    //
    // Read-Only
    //

    namespace Internal
    {
        void BeginDisabled();
        void EndDisabled();
    }

    // Usage : ReadOnly<FloatInput> readonly(...);
    template<typename T>
    class ReadOnly
    {
    public:
        template<typename ...Args>
        ReadOnly(const std::string& label, Args... args) // Take the args by copy, so make_unique has something to ref on
        {
            Internal::BeginDisabled();
            m_input = std::make_unique<T>(label, args...);
            Internal::EndDisabled();
        }

        operator T& () { return *m_input; }
        operator const T& () const { return *m_input; }

        T* operator->() { return m_input; }
        const T* operator->() const { return m_input; }

    private:
        std::unique_ptr<T> m_input;
    };
}