#include "imgui_plot_var.h"

#include <map>
#include <vector>
#include <algorithm>
#include <array>

int GetAdapativeScale(float time);

struct PlotVarData
{
    ImGuiID        ID;
    std::vector<float>  Data;
    int            DataInsertIdx;
    int            LastFrame;
    
    PlotVarData() : ID(0), DataInsertIdx(0), LastFrame(-1) {}
};

typedef std::map<ImGuiID, PlotVarData> PlotVarsMap;
static PlotVarsMap	g_PlotVarsMap;

// Plot value over time
// Call with 'value == FLT_MAX' to draw without adding new value to the buffer
void ImGui::PlotVar(const char* label, float value, float scale_min, float scale_max, size_t buffer_size)
{
    assert(label);
    if (buffer_size == 0)
        buffer_size = 240; // 4 seconds at 16.66 ms (60fps)
    
    ImGui::PushID(label);
    ImGuiID id = ImGui::GetID("");
    
    // Lookup O(log N)
    PlotVarData& pvd = g_PlotVarsMap[id];
    
    // Setup
    if (pvd.Data.capacity() != buffer_size)
    {
        pvd.Data.resize(buffer_size);
        memset(&pvd.Data[0], 0, sizeof(float) * buffer_size);
        pvd.DataInsertIdx = 0;
        pvd.LastFrame = -1;
    }
    
    // Insert (avoid unnecessary modulo operator)
    if (pvd.DataInsertIdx == buffer_size)
        pvd.DataInsertIdx = 0;
    int display_idx = pvd.DataInsertIdx;
    if (value != FLT_MAX)
        pvd.Data[pvd.DataInsertIdx++] = value;
    
    //
    auto minmax = std::minmax_element(pvd.Data.begin(), pvd.Data.end());
    
    float avg = 0;
    for (const auto &value :pvd.Data) {
        avg += value;
    }
    avg /= pvd.Data.size();
    
    char buffer[128] = "";
    snprintf(buffer, 128, "[ %.2f - %.2f ]", *minmax.first, *minmax.second);
    
    scale_max = (float)GetAdapativeScale(*minmax.second);
    
    char _label[64] = "";
    snprintf(_label, 64, "%.0f ms", scale_max);
    
    int current_frame = ImGui::GetFrameCount();
    if (pvd.LastFrame != current_frame)
    {
        ImGui::BeginChild("Sub1", ImVec2(-50, 80));
        ImGui::PlotLines("", &pvd.Data[0], buffer_size, pvd.DataInsertIdx, buffer, scale_min, scale_max, ImVec2(ImGui::GetContentRegionAvailWidth(), 80));
        ImGui::EndChild();
        
        ImGui::SameLine();
    
        ImGui::BeginChild("Sub2", ImVec2(0, 80));
//        ImGui::Text("%.0f ms\n\n%.0f ms\n\n%.0f ms", scale_max, value, scale_min);
        ImGui::Text("%.0f ms\n\n\n\n%.0f ms", scale_max, scale_min);
        ImGui::EndChild();
        
        pvd.LastFrame = current_frame;
    }
    
    ImGui::PopID();
}

void ImGui::PlotVarFlushOldEntries()
{
    int current_frame = ImGui::GetFrameCount();
    for (PlotVarsMap::iterator it = g_PlotVarsMap.begin(); it != g_PlotVarsMap.end(); )
    {
        PlotVarData& pvd = it->second;
        if (pvd.LastFrame < current_frame - std::max(400,(int)pvd.Data.size()))
            it = g_PlotVarsMap.erase(it);
        else
            ++it;
    }
}


int GetAdapativeScale(float time) {
 
    static std::array<int, 9> tresholds = {5000, 1000, 500, 200, 100, 50, 25};
    
    auto max_treshold = 10000000;
    for (const auto &treshold : tresholds) {
        if (time < treshold)
            max_treshold = treshold;
    }
    
    return max_treshold;
}