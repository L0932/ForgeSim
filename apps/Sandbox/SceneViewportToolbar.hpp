#pragma once

#include "TransformEditorUIState.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <imgui.h>
#include <nlohmann/json.hpp>

namespace ForgeSim::Sandbox
{
    // Scene-local overlay placement is independent of the editor window dock tree.
    class SceneViewportTools final
    {
    public:
        enum class Placement { Left, Right, Top, Bottom, Floating };

        void Load(const std::filesystem::path& path)
        {
            m_Path = path;
            try
            {
                std::ifstream input(path);
                const auto json = nlohmann::json::parse(input, nullptr, false);
                if (!json.is_object() || json.value("version", 0) != 1) return;
                const int placement = json.at("placement").get<int>();
                const float along = json.at("along").get<float>();
                const float x = json.at("x").get<float>(), y = json.at("y").get<float>();
                const bool vertical = json.at("vertical").get<bool>();
                if (placement < 0 || placement > 4 || !std::isfinite(along) ||
                    !std::isfinite(x) || !std::isfinite(y)) return;
                m_Placement = static_cast<Placement>(placement);
                m_Along = std::clamp(along, 0.0f, 1.0f);
                m_Floating = {std::clamp(x, 0.0f, 1.0f), std::clamp(y, 0.0f, 1.0f)};
                m_Vertical = vertical;
            }
            catch (...) { /* Invalid local preferences cannot prevent startup. */ }
        }

        // Draw after the image in its Scene window. Block scene input over
        // the overlay, its menu, and throughout a drag, including mouse release.
        bool Draw(TransformEditorUIState& state, ImVec2 origin, ImVec2 extent, bool disabled)
        {
            const auto& style = ImGui::GetStyle();
            const float unit = ImGui::GetFontSize();
            const float button = unit + 2.0f * style.FramePadding.y;
            const float gap = std::max(2.0f, unit * 0.16f), padding = gap * 2.0f;
            const float length = 6.0f * button + 5.0f * gap + 2.0f * padding;
            const float thickness = button + 2.0f * padding;
            bool vertical = m_Placement == Placement::Left || m_Placement == Placement::Right ||
                (m_Placement == Placement::Floating && m_Vertical);
            if (vertical && length > extent.y - 2.0f * padding) vertical = false;
            if (!vertical && length > extent.x - 2.0f * padding && length <= extent.y - 2.0f * padding)
                vertical = true;
            const ImVec2 size{std::min(vertical ? thickness : length, extent.x),
                std::min(vertical ? length : thickness, extent.y)};
            const ImVec2 room{std::max(0.0f, extent.x - size.x - 2.0f * padding),
                std::max(0.0f, extent.y - size.y - 2.0f * padding)};
            ImVec2 position{origin.x + padding, origin.y + padding};
            if (m_Placement == Placement::Right) position.x += room.x;
            if (m_Placement == Placement::Bottom) position.y += room.y;
            if (m_Placement == Placement::Left || m_Placement == Placement::Right)
                position.y += room.y * m_Along;
            else if (m_Placement == Placement::Top || m_Placement == Placement::Bottom)
                position.x += room.x * m_Along;
            else { position.x += room.x * m_Floating.x; position.y += room.y * m_Floating.y; }
            const auto clamp = [&](ImVec2 p)
            {
                return ImVec2(std::clamp(p.x, origin.x + padding, origin.x + padding + room.x),
                    std::clamp(p.y, origin.y + padding, origin.y + padding + room.y));
            };
            if (m_Dragging)
            {
                const auto mouse = ImGui::GetMousePos();
                position = clamp({mouse.x - m_DragOffset.x, mouse.y - m_DragOffset.y});
                m_DragPosition = position;
            }
            const auto savedCursor = ImGui::GetCursorScreenPos();
            ImGui::SetCursorScreenPos(position);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap, gap));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style.FrameRounding);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_WindowBg]);
            bool blocked = false, changed = false;
            if (ImGui::BeginChild("##SceneTools", size, ImGuiChildFlags_Borders,
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                const auto mouse = ImGui::GetMousePos();
                blocked = ImGui::IsMouseHoveringRect(position, {position.x + size.x, position.y + size.y});
                ImGui::BeginDisabled(disabled);
                ImGui::Button("##ToolsGrip", {button, button});
                if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                const auto grip = ImGui::GetItemRectMin();
                auto* draw = ImGui::GetWindowDrawList();
                for (int row = -1; row <= 1; ++row)
                    for (int col : {-1, 1})
                        draw->AddCircleFilled({grip.x + button*.5f + col*unit*.12f,
                            grip.y + button*.5f + row*unit*.22f}, unit*.055f,
                            ImGui::GetColorU32(ImGuiCol_TextDisabled));
                ImGui::SetItemTooltip("Tools: drag to move; release near a Scene edge to snap.\nRight-click for layout options.");
                if (ImGui::IsItemActivated()) m_DragOffset = {mouse.x-position.x, mouse.y-position.y};
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    m_Dragging = true;
                    m_DragPosition = clamp({mouse.x-m_DragOffset.x, mouse.y-m_DragOffset.y});
                }
                if (ImGui::BeginPopupContextItem("ToolsLayout"))
                {
                    blocked = true;
                    const char* labels[]{"Snap left", "Snap right", "Snap top", "Snap bottom"};
                    for (int i=0; i<4; ++i)
                        if (ImGui::MenuItem(labels[i], nullptr, static_cast<int>(m_Placement)==i))
                        { m_Placement=static_cast<Placement>(i); changed=true; }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Float horizontally", nullptr,
                        m_Placement==Placement::Floating && !m_Vertical))
                    { m_Placement=Placement::Floating; m_Vertical=false; changed=true; }
                    if (ImGui::MenuItem("Float vertically", nullptr,
                        m_Placement==Placement::Floating && m_Vertical))
                    { m_Placement=Placement::Floating; m_Vertical=true; changed=true; }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Reset tools placement"))
                    { m_Placement=Placement::Left; m_Along=0; m_Floating={.15f,.15f}; m_Vertical=true; changed=true; }
                    ImGui::EndPopup();
                }
                const auto next = [&] { if (!vertical) ImGui::SameLine(); };
                next();
                if (IconButton("##Move", 0, state.operation==GizmoOperation::Translate, button, "Move (W)"))
                    state.operation=GizmoOperation::Translate;
                next();
                if (IconButton("##Rotate", 1, state.operation==GizmoOperation::Rotate, button, "Rotate (E)"))
                    state.operation=GizmoOperation::Rotate;
                next();
                if (IconButton("##Scale", 2, state.operation==GizmoOperation::Scale, button, "Scale (R); uses local axes"))
                    state.operation=GizmoOperation::Scale;
                const bool scaling=state.operation==GizmoOperation::Scale;
                ImGui::BeginDisabled(scaling);
                next();
                if (IconButton("##World", 3, !scaling && state.space==GizmoSpace::World, button, "World axes (X toggles space)"))
                    state.space=GizmoSpace::World;
                next();
                if (IconButton("##Local", 4, scaling || state.space==GizmoSpace::Local, button,
                    "Local axes (X toggles space); scale always uses local axes")) state.space=GizmoSpace::Local;
                ImGui::EndDisabled();
                ImGui::EndDisabled();
                blocked |= ImGui::IsAnyItemActive() && ImGui::IsWindowFocused();
            }
            ImGui::EndChild();
            ImGui::PopStyleColor(); ImGui::PopStyleVar(3);
            ImGui::SetCursorScreenPos(savedCursor);
            ImGui::Dummy({0,0});
            if (m_Dragging)
            {
                blocked=true;
                const auto mouse=ImGui::GetMousePos();
                const float distances[]{std::abs(mouse.x-origin.x), std::abs(mouse.x-origin.x-extent.x),
                    std::abs(mouse.y-origin.y), std::abs(mouse.y-origin.y-extent.y)};
                const auto nearest=std::min_element(std::begin(distances),std::end(distances));
                const auto snap=*nearest <= unit*2 ? static_cast<Placement>(nearest-distances) : Placement::Floating;
                if (snap!=Placement::Floating)
                {
                    ImVec2 min=origin,max{origin.x+extent.x,origin.y+extent.y};
                    if (snap==Placement::Left) max.x=min.x+unit*.25f;
                    if (snap==Placement::Right) min.x=max.x-unit*.25f;
                    if (snap==Placement::Top) max.y=min.y+unit*.25f;
                    if (snap==Placement::Bottom) min.y=max.y-unit*.25f;
                    ImGui::GetWindowDrawList()->AddRectFilled(min,max,ImGui::GetColorU32(ImGuiCol_NavCursor));
                }
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    m_Dragging=false; m_Placement=snap; m_Vertical=vertical;
                    m_Floating={room.x>0 ? (m_DragPosition.x-origin.x-padding)/room.x : 0,
                        room.y>0 ? (m_DragPosition.y-origin.y-padding)/room.y : 0};
                    m_Along=(snap==Placement::Left || snap==Placement::Right) ? m_Floating.y : m_Floating.x;
                    changed=true;
                }
            }
            if (changed) Save();
            return blocked;
        }

    private:
        static bool IconButton(const char* id,int icon,bool selected,float size,const char* tooltip)
        {
            if (selected) ImGui::PushStyleColor(ImGuiCol_Button,ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
            const bool pressed=ImGui::Button(id,{size,size});
            if (selected) ImGui::PopStyleColor();
            ImGui::SetItemTooltip("%s",tooltip);
            auto* draw=ImGui::GetWindowDrawList(); const auto min=ImGui::GetItemRectMin();
            const ImVec2 center{min.x+size*.5f,min.y+size*.5f};
            const float r=size*.27f,weight=std::max(1.0f,size*.055f);
            const auto color=ImGui::GetColorU32(ImGuiCol_Text);
            const auto point=[&](float x,float y){return ImVec2(center.x+r*x,center.y+r*y);};
            const auto line=[&](float x,float y,float a,float b){draw->AddLine(point(x,y),point(a,b),color,weight);};
            if (icon==0)
            {
                line(-1,0,1,0); line(0,-1,0,1);
                for(float sign:{-1.f,1.f})
                { line(sign,0,sign*.6f,-.35f); line(sign,0,sign*.6f,.35f);
                  line(0,sign,-.35f,sign*.6f); line(0,sign,.35f,sign*.6f); }
            }
            else if(icon==1)
            { draw->PathArcTo(center,r,0,5.1f,20); draw->PathStroke(color,0,weight);
              line(.38f,-.92f,-.1f,-.8f); line(.38f,-.92f,.25f,-.45f); }
            else if(icon==2)
            { draw->AddRect(point(-1,-1),point(.3f,.3f),color,0,0,weight);
              line(-.4f,-.4f,1,1); line(1,1,1,.35f); line(1,1,.35f,1); }
            else if(icon==3)
            { draw->AddCircle(center,r,color,20,weight); line(-1,0,1,0);
              draw->AddEllipse(center,{r*.45f,r},color,0,20,weight); }
            else
            { line(-.7f,.7f,-.7f,-1); line(-.7f,.7f,1,.7f); line(-.7f,.7f,.6f,-.6f);
              line(-.7f,-1,-1,-.65f); line(-.7f,-1,-.4f,-.65f);
              line(1,.7f,.65f,.4f); line(1,.7f,.65f,1); }
            return pressed;
        }

        void Save() const noexcept
        {
            if(m_Path.empty()) return;
            try
            {
                std::ofstream output(m_Path,std::ios::trunc);
                if(output) output << nlohmann::json{{"version",1},{"placement",static_cast<int>(m_Placement)},
                    {"along",m_Along},{"x",m_Floating.x},{"y",m_Floating.y},{"vertical",m_Vertical}}.dump(2) << '\n';
            }
            catch(...) { /* Placement remains usable when saving is unavailable. */ }
        }
        std::filesystem::path m_Path;
        Placement m_Placement=Placement::Left;
        float m_Along=0;
        ImVec2 m_Floating{.15f,.15f};
        bool m_Vertical=true,m_Dragging=false;
        ImVec2 m_DragOffset{},m_DragPosition{};
    };
}
