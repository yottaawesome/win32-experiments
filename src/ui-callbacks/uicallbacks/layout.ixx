export module uicallbacks:layout;
import std;
import :win32;
export namespace UiCallbacks
{
    struct LayoutItem 
    {
        Win32::HWND hwnd;
        float weight;   // 0 = fixed size, >0 = proportional
        int fixedSize;  // used when weight == 0
    };

    struct VBox 
    {
        int spacing;
        int margin;
        std::vector<LayoutItem> items;

        void Apply(int x, int y, int w, int h) 
        {
            float totalWeight = 0;
            int fixedTotal = margin * 2 + spacing * (static_cast<int>(items.size()) - 1);
            for (auto& item : items) 
            {
                if (item.weight == 0) 
                    fixedTotal += item.fixedSize;
                else 
                    totalWeight += item.weight;
            }
            int flexible = h - fixedTotal;
            int cy = y + margin;
            for (auto& item : items) 
            {
                int itemH = item.weight > 0
                    ? static_cast<int>(flexible * item.weight / totalWeight)
                    : item.fixedSize;
                Win32::MoveWindow(item.hwnd, x + margin, cy, w - margin * 2, itemH, true);
                cy += itemH + spacing;
            }
        }
    };
}