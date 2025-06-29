#pragma once
#include "imgui.h"
#include <vector>
#include <string>
#include "imgui_internal.h"
#include <unordered_map>
#include <format>

class CustomMenu {
public:
    CustomMenu(ImDrawList* DrawList)
        : DrawList(DrawList), MenuPos(100, 100), MenuSize(400, 400),
        Dragging(false), CurrentTab(0), ActiveControl(-1), ControlID(0) {

        this->Tabs = { "Aimbot", "Visuals", "Colors", "Info" };

        this->TabAnimProgress.resize(this->Tabs.size(), 0.0f);

        this->ColBackground = IM_COL32(25, 25, 25, 255);
        this->ColBorder = IM_COL32(60, 60, 60, 255);
        this->ColTitleBar = IM_COL32(40, 40, 40, 255);
        this->ColText = IM_COL32(220, 220, 220, 255);
        this->ColWidget = IM_COL32(40, 40, 40, 255);
        this->ColWidgetActive = IM_COL32(48, 85, 255, 255);
        this->ColButtonActive = IM_COL32(50, 50, 50, 255);
        this->ColSliderGrab = IM_COL32(180, 180, 180, 255);
        this->ColTabInactive = IM_COL32(100, 100, 100, 255);
        this->ColTabHover = IM_COL32(150, 150, 150, 255);
        this->ColTabActive = IM_COL32(48, 85, 255, 255);
    }

    void Render() {
        ImGuiIO& IO = ImGui::GetIO();
        ImVec2 MousePos = IO.MousePos;
        this->HandleDragging(MousePos, IO);

        this->DrawList->AddRectFilled(this->MenuPos, this->MenuPos + this->MenuSize, this->ColBackground, 4);
        this->DrawList->AddRect(this->MenuPos, this->MenuPos + this->MenuSize, this->ColBorder, 4);

        ImVec2 TitleBarMin = this->MenuPos;
        ImVec2 TitleBarMax = this->MenuPos + ImVec2(this->MenuSize.x, 28);
        this->DrawList->AddRectFilled(TitleBarMin, TitleBarMax, this->ColTitleBar, 4, ImDrawFlags_RoundCornersTop);
        this->DrawList->AddRect(TitleBarMin, TitleBarMax, this->ColBorder, 4, ImDrawFlags_RoundCornersTop);

        const char* Title = X("Lumina CS2 Free");
        ImVec2 TitleSize = ImGui::CalcTextSize(Title);
        ImVec2 TitleCenter = TitleBarMin + ImVec2((this->MenuSize.x - TitleSize.x) * 0.5f, (28 - TitleSize.y) * 0.5f);
        this->DrawList->AddText(TitleCenter, this->ColText, Title);

        this->DrawTabs();

        ImVec2 FooterBarMin = this->MenuPos + ImVec2(0, this->MenuSize.y - 28);
        ImVec2 FooterBarMax = this->MenuPos + this->MenuSize;
        this->DrawList->AddRectFilled(FooterBarMin, FooterBarMax, this->ColTitleBar, 4, ImDrawFlags_RoundCornersBottom);
        this->DrawList->AddRect(FooterBarMin, FooterBarMax, this->ColBorder, 4, ImDrawFlags_RoundCornersBottom);
        const char* FooterText = X("Version: 1");
        ImVec2 TextSize = ImGui::CalcTextSize(FooterText);
        ImVec2 FooterCenter = FooterBarMin + ImVec2((this->MenuSize.x - TextSize.x) * 0.5f, (28 - TextSize.y) * 0.5f);
        this->DrawList->AddText(FooterCenter, this->ColText, FooterText);

        this->ContentPos = this->MenuPos + ImVec2(12, 28 + 6.f + 26.f + 10);
        this->ContentSize = this->MenuSize - ImVec2(24, 28 + 26 + 16);
        this->ContentCursorPos = this->ContentPos;
    }

    bool Button(const char* Label) {
        ImVec2 TextSize = ImGui::CalcTextSize(Label);
        ImVec2 Padding = ImVec2(12, 4);
        ImVec2 Size = ImVec2(ImMax(90.0f, TextSize.x + Padding.x * 2), ImMax(22.0f, TextSize.y + Padding.y * 2));

        ImVec2 Pos = this->ContentCursorPos;
        ImRect Rect(Pos, Pos + Size);

        bool Hovered = this->RectHover(Rect);
        bool Clicked = Hovered && ImGui::GetIO().MouseClicked[0];

        this->DrawList->AddRectFilled(Rect.Min, Rect.Max, this->ColWidget);
        this->DrawList->AddRect(Rect.Min, Rect.Max, this->ColBorder, 3);

        ImVec2 TextPos = Rect.Min + (Size - TextSize) * 0.5f;
        this->DrawList->AddText(TextPos, this->ColText, Label);

        this->ContentCursorPos.y += Size.y + 10;

        return Clicked;
    }

    bool Checkbox(const char* Label, bool* V) {
        ImVec2 BoxSize(16, 16);
        ImVec2 Pos = this->ContentCursorPos;
        ImRect Box(Pos, Pos + BoxSize);

        ImGuiID ID = ImGui::GetID(Label);
        float& Anim = this->CheckboxAnimProgress[ID];

        bool Hovered = this->RectHover(Box);
        bool Clicked = Hovered && ImGui::GetIO().MouseClicked[0];
        if (Clicked) {
            *V = !*V;
            Anim = 0.f;
        }

        float Target = *V ? 1.0f : 0.0f;
        Anim = ImClamp(ImLerp(Anim, Target, ImGui::GetIO().DeltaTime * 10.f), 0.0f, 1.0f);

        ImColor From = ImColor(this->ColWidget);
        ImColor To = ImColor(this->ColWidgetActive);
        ImColor Blended = ImColor(
            ImLerp(From.Value.x, To.Value.x, Anim),
            ImLerp(From.Value.y, To.Value.y, Anim),
            ImLerp(From.Value.z, To.Value.z, Anim),
            ImLerp(From.Value.w, To.Value.w, Anim)
        );
        ImU32 FillColor = Blended;

        this->DrawList->AddRectFilled(Box.Min, Box.Max, FillColor, 2);
        this->DrawList->AddRect(Box.Min, Box.Max, this->ColBorder, 2);

        ImVec2 LabelSize = ImGui::CalcTextSize(Label);
        ImVec2 LabelPos = ImVec2(Box.Max.x + 8, Box.Min.y + (BoxSize.y - LabelSize.y) * 0.5f);
        this->DrawList->AddText(LabelPos, this->ColText, Label);

        this->ContentCursorPos.y += BoxSize.y + 10;

        return Clicked;
    }

    bool SliderInt(const char* Label, int* V, int Min, int Max) {
        ImVec2 Pos = this->ContentCursorPos;
        float BarWidth = 250.f;
        float SliderHeight = 10.f;
        ImRect Slider(Pos + ImVec2(0, 18), Pos + ImVec2(BarWidth, 18 + SliderHeight));

        ImGuiID ID = ImGui::GetID(Label);
        float& Anim = this->SliderAnimProgress[ID];

        float Target = (*V - Min) / float(Max - Min);
        Anim = ImLerp(Anim, Target, ImGui::GetIO().DeltaTime * 10.f);

        ImRect Fill = Slider;
        Fill.Max.x = Fill.Min.x + Anim * Slider.GetWidth();

        this->DrawList->AddText(Pos, this->ColText, Label);
        this->DrawList->AddRectFilled(Slider.Min, Slider.Max, this->ColWidget, 2);
        this->DrawList->AddRectFilled(Fill.Min, Fill.Max, this->ColWidgetActive, 2);
        this->DrawList->AddRect(Slider.Min, Slider.Max, this->ColBorder, 2);

        float MarkerHeight = SliderHeight + 5.0f;
        float MarkerY = Slider.Min.y + (SliderHeight - MarkerHeight) * 0.5f;
        this->DrawList->AddLine(ImVec2(Fill.Max.x, MarkerY), ImVec2(Fill.Max.x, MarkerY + MarkerHeight), this->ColSliderGrab, 4);

        if (this->RectHover(Slider) && ImGui::GetIO().MouseDown[0]) {
            float Rel = ImGui::GetIO().MousePos.x - Slider.Min.x;
            Rel = ImClamp(Rel, 0.f, Slider.GetWidth());
            *V = Min + int((Rel / Slider.GetWidth()) * (Max - Min));
        }

        char ValueBuf[64];
        const char* End = ValueBuf + ImGui::DataTypeFormatString(ValueBuf, IM_ARRAYSIZE(ValueBuf), ImGuiDataType_S32, V, "%d");
        ImVec2 ValSize = ImGui::CalcTextSize(ValueBuf);
        this->DrawList->AddText(ImVec2(Slider.Max.x - ValSize.x, Pos.y), this->ColText, ValueBuf, End);

        this->ContentCursorPos.y += 40;

        return true;
    }

    bool KeyInput(const char* Label, int* OutKey) {
        ImVec2 BoxSize(90.f, 22.f);
        ImVec2 Pos = this->ContentCursorPos;
        ImRect Box(Pos, Pos + BoxSize);

        ImGuiID Id = ImGui::GetID(Label);
        bool Hovered = this->RectHover(Box);
        bool Clicked = Hovered && ImGui::IsMouseClicked(0);

        static ImGuiID ListeningID = 0;
        bool Listening = (ListeningID == Id);

        if (Clicked) ListeningID = Listening ? 0 : Id;
        if (Listening) {
            for (int i = 0; i < ImGuiKey_COUNT; ++i) {
                if (ImGui::IsKeyPressed((ImGuiKey)i, false)) {
                    *OutKey = i;
                    ListeningID = 0;
                    break;
                }
            }
        }

        const char* Display = Listening ? X("...") : (*OutKey > 0 && *OutKey < ImGuiKey_COUNT) ? ImGui::GetKeyName((ImGuiKey)*OutKey) : X("None");

        ImVec2 TextSize = ImGui::CalcTextSize(Display);
        float Width = ImMax(90.0f, TextSize.x + 12.0f * 2);
        Box.Max.x = Box.Min.x + Width;

        this->DrawList->AddRectFilled(Box.Min, Box.Max, this->ColWidget, 3);
        this->DrawList->AddRect(Box.Min, Box.Max, this->ColBorder, 3);

        ImVec2 TextPos = Box.Min + (Box.GetSize() - TextSize) * 0.5f;
        this->DrawList->AddText(TextPos, this->ColText, Display);

        ImVec2 LabelSize = ImGui::CalcTextSize(Label);
        ImVec2 LabelPos = ImVec2(Box.Max.x + 8, Box.Min.y + (BoxSize.y - LabelSize.y) * 0.5f);
        this->DrawList->AddText(LabelPos, this->ColText, Label);

        this->ContentCursorPos.y += BoxSize.y + 10.f;

        return Clicked;
    }

    void Text(const char* Label) {
        ImVec2 Pos = this->ContentCursorPos;
        ImVec2 LabelSize = ImGui::CalcTextSize(Label);

        this->DrawList->AddText(Pos, this->ColText, Label);

        this->ContentCursorPos.y += LabelSize.y + 10;
    }

    bool ColorPicker(const char* Label, ImColor* Color) {
        ImVec2 BoxSize(33.f, 16.f);
        ImVec2 Pos = this->ContentCursorPos;
        ImRect Box(Pos, Pos + BoxSize);

        ImGuiID ID = ImGui::GetID(Label);
        bool Hovered = this->RectHover(Box);
        bool Clicked = Hovered && ImGui::IsMouseClicked(0);

        this->DrawList->AddRectFilled(Box.Min, Box.Max, ImGui::ColorConvertFloat4ToU32(Color->Value), 3);
        this->DrawList->AddRect(Box.Min, Box.Max, this->ColBorder, 3);

        ImVec2 LabelSize = ImGui::CalcTextSize(Label);
        ImVec2 LabelPos = ImVec2(Box.Max.x + 8, Box.Min.y + (BoxSize.y - LabelSize.y) * 0.5f);
        this->DrawList->AddText(LabelPos, this->ColText, Label);

        this->ContentCursorPos.y += BoxSize.y + 10.f;

        std::string PopupName = std::string(X("ColorPicker##")) + std::to_string(ID);

        if (Clicked) ImGui::OpenPopup(PopupName.c_str());

        bool Changed = false;

        ImGui::PushStyleColor(ImGuiCol_PopupBg, this->ColBackground);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f);
        if (ImGui::BeginPopup(PopupName.c_str())) {
            float Col[4] = {
                Color->Value.x,
                Color->Value.y,
                Color->Value.z,
                Color->Value.w
            };

            Changed = ImGui::ColorPicker4(X("##picker"), Col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview);

            if (Changed) *Color = ImColor(Col[0], Col[1], Col[2], Col[3]);

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        return Changed;
    }

    bool Combo(const char* Label, int* CurrentValue, const std::vector<std::pair<std::string, int>>& Options) {
        int CurrentIndex = 0;
        for (size_t i = 0; i < Options.size(); ++i) {
            if (Options[i].second == *CurrentValue) {
                CurrentIndex = (int)i;

                break;
            }
        }

        ImVec2 Pos = this->ContentCursorPos;
        ImVec2 ArrowSize(16, 16);
        ImVec2 BoxSize(33.f, 16.f);

        ImRect LeftArrow(Pos, Pos + ArrowSize);
        ImRect RightArrow(Pos + ImVec2(ArrowSize.x + BoxSize.x + 6, 0), Pos + ImVec2(ArrowSize.x * 2 + BoxSize.x + 6, ArrowSize.y));

        ImRect TextBox(Pos + ImVec2(ArrowSize.x + 3, 0), Pos + ImVec2(ArrowSize.x + BoxSize.x + 3, BoxSize.y));

        bool HoverLeft = this->RectHover(LeftArrow);
        bool HoverRight = this->RectHover(RightArrow);

        bool ClickLeft = HoverLeft && ImGui::IsMouseClicked(0);
        bool ClickRight = HoverRight && ImGui::IsMouseClicked(0);

        bool Changed = false;

        if (ClickLeft) {
            CurrentIndex--;
            if (CurrentIndex < 0) CurrentIndex = (int)Options.size() - 1;
            Changed = true;
        }
        if (ClickRight) {
            CurrentIndex++;
            if (CurrentIndex >= (int)Options.size()) CurrentIndex = 0;
            Changed = true;
        }

        if (!Options.empty()) {
            *CurrentValue = Options[CurrentIndex].second;
        }

        this->DrawList->AddRectFilled(LeftArrow.Min, LeftArrow.Max, this->ColWidget, 3);
        this->DrawList->AddRect(LeftArrow.Min, LeftArrow.Max, this->ColBorder, 2);
        this->DrawList->AddText(LeftArrow.Min + ImVec2(4, 0), this->ColText, "<");

        this->DrawList->AddRectFilled(RightArrow.Min, RightArrow.Max, this->ColWidget, 3);
        this->DrawList->AddRect(RightArrow.Min, RightArrow.Max, this->ColBorder, 2);
        this->DrawList->AddText(RightArrow.Min + ImVec2(4, 0), this->ColText, ">");

        if (!Options.empty()) {
            const char* CurrentText = Options[CurrentIndex].first.c_str();
            ImVec2 TextSize = ImGui::CalcTextSize(CurrentText);
            ImVec2 Center = TextBox.Min + (TextBox.GetSize() - TextSize) * 0.5f;
            this->DrawList->AddText(Center, this->ColText, CurrentText);
        }

        ImVec2 LabelSize = ImGui::CalcTextSize(Label);
        ImVec2 LabelPos = ImVec2(RightArrow.Max.x + 8, Pos.y + (BoxSize.y - LabelSize.y) * 0.5f);
        this->DrawList->AddText(LabelPos, this->ColText, Label);

        this->ContentCursorPos.y += BoxSize.y + 10.f;

        return Changed;
    }

    void SetTab(int I) { if (I >= 0 && I < this->Tabs.size()) this->CurrentTab = I; }
    int GetTab() const { return this->CurrentTab; }

private:
    ImDrawList* DrawList;
    ImVec2 MenuPos, MenuSize;
    ImVec2 ContentPos, ContentSize, ContentCursorPos;
    std::vector<const char*> Tabs;
    int CurrentTab;
    bool Dragging;
    ImVec2 DragOffset;
    int ActiveControl;
    int ControlID;
    std::vector<float> TabAnimProgress;
    std::unordered_map<ImGuiID, float> CheckboxAnimProgress;
    std::unordered_map<ImGuiID, float> SliderAnimProgress;

    ImU32 ColBackground, ColBorder, ColTitleBar, ColText, ColWidget, ColWidgetActive, ColButtonActive, ColSliderGrab, ColTabInactive, ColTabHover, ColTabActive;

    bool RectHover(const ImRect& R) {
        return R.Contains(ImGui::GetIO().MousePos);
    }

    void HandleDragging(ImVec2 Mouse, ImGuiIO& IO) {
        ImRect Head(this->MenuPos, this->MenuPos + ImVec2(this->MenuSize.x, 28));
        if (this->Dragging && !IO.MouseDown[0]) this->Dragging = false;
        if (this->Dragging) this->MenuPos = Mouse - this->DragOffset;
        else if (IO.MouseDown[0] && Head.Contains(Mouse)) {
            this->Dragging = true;
            this->DragOffset = Mouse - this->MenuPos;
        }
    }

    void DrawTabs() {
        int TabCount = this->Tabs.size();
        float TabHeight = 26.f;
        float Gap = 6.f;
        float TotalWidth = this->MenuSize.x - 20.f;
        float TotalGaps = Gap * (TabCount - 1);
        float TabWidth = (TotalWidth - TotalGaps) / TabCount;
        ImVec2 Start = this->MenuPos + ImVec2(10, 28 + 6.f);
        float Delta = ImGui::GetIO().DeltaTime * 10.0f;

        for (int I = 0; I < TabCount; ++I) {
            ImVec2 Pos = Start + ImVec2(I * (TabWidth + Gap), 0);

            ImRect TabRect(Pos, Pos + ImVec2(TabWidth, TabHeight));
            bool Hovered = TabRect.Contains(ImGui::GetIO().MousePos);

            float& Anim = this->TabAnimProgress[I];
            float Target = (I == this->CurrentTab || Hovered) ? 1.0f : 0.0f;
            Anim = ImClamp(ImLerp(Anim, Target, Delta), 0.0f, 1.0f);

            ImColor From = ImColor(this->ColTabInactive);
            ImColor To = ImColor(I == this->CurrentTab ? this->ColTabActive : this->ColTabHover);
            ImColor Blended = ImColor(
                ImLerp(From.Value.x, To.Value.x, Anim),
                ImLerp(From.Value.y, To.Value.y, Anim),
                ImLerp(From.Value.z, To.Value.z, Anim),
                ImLerp(From.Value.w, To.Value.w, Anim)
            );
            ImU32 Color = Blended;
            ImVec2 TextSize = ImGui::CalcTextSize(this->Tabs[I]);
            ImVec2 Center = Pos + ImVec2(TabWidth * 0.5f, TabHeight * 0.5f);
            ImVec2 TextPos = ImVec2(Center.x - TextSize.x * 0.5f, Pos.y + 4);

            this->DrawList->AddText(TextPos, Color, this->Tabs[I]);

            if (I == this->CurrentTab || Anim > 0.1f) {
                float UnderlineWidth = TabWidth * Anim;
                float UnderlineX = Pos.x + (TabWidth - UnderlineWidth) * 0.5f;
                this->DrawList->AddRectFilled(ImVec2(UnderlineX, Pos.y + TabHeight - 2), ImVec2(UnderlineX + UnderlineWidth, Pos.y + TabHeight), this->ColTabActive, 1);
            }

            if (ImGui::IsMouseClicked(0) && Hovered) this->CurrentTab = I;
        }
    }
};