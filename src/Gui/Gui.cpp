#include "Gui.hpp"
#include <Core/Features/Exploits/ResourceList.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <algorithm>

static std::once_flag resource_list_thread_flag;
namespace Exploits = core::Features::Exploits;

void StartResourceListThread() {
    std::thread([] {
        while (true) {
            Exploits::g_ResourceList.List();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }).detach();
}

void Gui::Rendering() {
    static int selected_tab = 0;
    constexpr const char* tab_names[] = { "Combat", "Visuals", "Local", "World", "Exploits", "Settings" };
    constexpr int tab_count = sizeof(tab_names) / sizeof(tab_names[0]);

    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Once);

    if (ImGui::Begin("secovce services made by eso - base", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        if (ImGui::BeginTabBar("Tabs")) {
            for (int i = 0; i < tab_count; ++i) {
                if (ImGui::BeginTabItem(tab_names[i])) {
                    selected_tab = i;
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::Separator();

        switch (selected_tab) {
        case 0:
            ImGui::Text("combat cigan");
            break;
        case 1:
            ImGui::Checkbox("enable", &g_Config.ESP->Enabled);
            ImGui::Checkbox("box", &g_Config.ESP->Box);
            ImGui::Checkbox("usernames", &g_Config.ESP->UserNames);
            ImGui::Checkbox("weapon names", &g_Config.ESP->WeaponName);
            ImGui::Checkbox("distance", &g_Config.ESP->DistanceFromMe);
            ImGui::Checkbox("skeleton", &g_Config.ESP->p_skeleton);
            ImGui::Checkbox("health bar", &g_Config.ESP->HealthBar);
            ImGui::Checkbox("armor bar", &g_Config.ESP->ArmorBar);
            ImGui::Checkbox("include localplayer", &g_Config.ESP->ShowLocalPlayer);
            ImGui::Checkbox("exclude npcs", &g_Config.ESP->IgnoreNPCs);
            ImGui::Checkbox("exclude deads", &g_Config.ESP->IgnoreDead);
            ImGui::SliderInt("max render distance", &g_Config.ESP->MaxDistance, 0, 1000, "%dm");
            break;
        case 2:
            ImGui::SliderFloat("armor value", &g_Config.Player->CurrentArmorValue, 0.f, 99.f, "%.1f");
            if (ImGui::Button("give armor")) {
                core::sdk::Pointers::pLocalPlayer->SetArmor(g_Config.Player->CurrentArmorValue);
            }
            break;
        case 4: {
            std::call_once(resource_list_thread_flag, StartResourceListThread);

            ImGui::Text(xorstr("resource stopper made by eso"));

            static char resource_search[128] = "";
            static char last_resource_search[128] = "";
            static bool suppress_next_input = false;

            if (!suppress_next_input) {
                if (ImGui::InputTextWithHint(xorstr("##resource_search"), xorstr("(e.g monitor) do not use it on freshrp"), resource_search, sizeof(resource_search))) {
                    if (strcmp(resource_search, last_resource_search) != 0) {
                        strcpy(last_resource_search, resource_search);
                        suppress_next_input = true;
                    }
                }
            } else {
                suppress_next_input = false;
            }

            ImGui::Separator();

            auto& resources = Exploits::vResources;
            std::vector<std::pair<size_t, Exploits::Resources_t*>> filtered;
            std::string search_str = last_resource_search;
            std::transform(search_str.begin(), search_str.end(), search_str.begin(), ::tolower);

            for (size_t i = 0; i < resources.size(); ++i) {
                auto& res = resources[i];
                std::string path_lower = res.Path;
                std::transform(path_lower.begin(), path_lower.end(), path_lower.begin(), ::tolower);
                if (search_str.empty() || path_lower.find(search_str) != std::string::npos) {
                    filtered.emplace_back(i, &res);
                }
            }

            if (filtered.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), xorstr("picka"));
            } else {
                for (auto& [i, res] : filtered) {
                    ImGui::PushID(static_cast<int>(i));
                    ImGui::TextUnformatted(res->Path.c_str());
                    ImGui::SameLine();
                    switch (res->State) {
                    case Exploits::eResourceState::Stopped:
                        ImGui::TextColored(ImVec4(0.7f, 0.2f, 0.2f, 1.0f), xorstr("stopped"));
                        break;
                    case Exploits::eResourceState::Starting:
                        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.2f, 1.0f), xorstr("starting"));
                        break;
                    case Exploits::eResourceState::Started:
                        if (ImGui::Button(xorstr("Stop"))) {
                            uintptr_t ptr = res->Pointer;
                            std::thread([ptr] { Exploits::g_ResourceList.Stop(ptr); }).detach();
                        }
                        break;
                    case Exploits::eResourceState::Stopping:
                        ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.2f, 1.0f), xorstr("stopping"));
                        break;
                    case Exploits::eResourceState::Uninitialized:
                    default:
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), xorstr("uninitialized"));
                        break;
                    }
                    ImGui::PopID();
                }
            }
            break;
        }
        case 5:
            ImGui::Text("general settings");
            ImGui::Checkbox("VSync", &g_Config.General->VSync);
            break;
        default:
            ImGui::Text("there is nothing skinky skid");
            break;
        }
    }
    ImGui::End();
}