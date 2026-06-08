#include "Aimbot.hpp"
#include <cmath>
#include <algorithm>
#include <thread>

constexpr float PI = 3.14159265f;

float AngleLerp(float a, float b, float t) {
    float diff = fmodf(b - a + 180.0f, 360.0f) - 180.0f;
    return a + diff * t;
}

void core::Features::cAimbot::SetViewAngles(CPed* Ped, D3DXVECTOR3 BonePos) {
    uintptr_t cam = driver.Read<uintptr_t>(core::sdk::Pointers::pCamGamePlayDirector + 0x2C0);
    if (!cam) return;

    D3DXVECTOR3 camPos = driver.Read<D3DXVECTOR3>(cam + 0x60);
    D3DXVECTOR3 direction = BonePos - camPos;
    D3DXVec3Normalize(&direction, &direction);

    float targetPitch = -asinf(direction.z) * (180.0f / PI);
    float targetYaw = atan2f(direction.y, direction.x) * (180.0f / PI);

    D3DXVECTOR3 currentDir = driver.Read<D3DXVECTOR3>(cam + 0x40);
    D3DXVec3Normalize(&currentDir, &currentDir);
    float currentPitch = -asinf(currentDir.z) * (180.0f / PI);
    float currentYaw = atan2f(currentDir.y, currentDir.x) * (180.0f / PI);

    float yawDiff = fmodf(targetYaw - currentYaw + 180.0f, 360.0f) - 180.0f;
    float pitchDiff = fmodf(targetPitch - currentPitch + 180.0f, 360.0f) - 180.0f;


    D3DXVECTOR2 targetScreenPos = core::sdk::game::w2s(BonePos);
    if (!core::sdk::game::IsOnScreen(targetScreenPos)) return;
    float screenDist = std::hypot(targetScreenPos.x - g_Variables.g_vGameWindowCenter.x, targetScreenPos.y - g_Variables.g_vGameWindowCenter.y);


    float baseSpeed = g_Config.Aimbot->AimbotSpeed;
    float smoothingMultiplier = 0.05f;
    float lerpFactor = std::clamp(baseSpeed * smoothingMultiplier * (screenDist / g_Config.Aimbot->FOV), 0.01f, 1.0f);

    float finalPitch = currentPitch + pitchDiff * lerpFactor;
    float finalYaw = currentYaw + yawDiff * lerpFactor;

    const float maxAngleChange = 0.1f;
    if (fabs(finalPitch - currentPitch) > maxAngleChange) {
        finalPitch = currentPitch + (finalPitch > currentPitch ? maxAngleChange : -maxAngleChange);
    }
    if (fabs(finalYaw - currentYaw) > maxAngleChange) {
        finalYaw = currentYaw + (finalYaw > currentYaw ? maxAngleChange : -maxAngleChange);
    }

    float radPitch = -finalPitch * (PI / 180.0f);
    float radYaw = finalYaw * (PI / 180.0f);

    D3DXVECTOR3 finalDir = {
        cosf(radPitch) * cosf(radYaw),
        cosf(radPitch) * sinf(radYaw),
        sinf(radPitch)
    };
    D3DXVec3Normalize(&finalDir, &finalDir);

    driver.Write<D3DXVECTOR3>(cam + 0x40, finalDir);
    driver.Write<D3DXVECTOR3>(cam + 0x3D0, finalDir);
}

void core::Features::cAimbot::Start() {
    while (true) {
        if (g_Config.Aimbot->Enabled && g_Config.Aimbot->KeyBind && (GetAsyncKeyState(g_Config.Aimbot->KeyBind) & 0x8000)
            && GetForegroundWindow() != g_Variables.g_hCheatWindow) {

            CPed* Ped = core::sdk::game::GetClosestPed(g_Config.Aimbot->MaxDistance, g_Config.Aimbot->IgnoreNPCs, g_Config.Aimbot->OnlyVisible);
            if (!Ped) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            D3DXVECTOR3 HeadPos = Ped->GetBonePosDefault(0);
            D3DXVECTOR2 ScreenHeadPos = core::sdk::game::w2s(HeadPos);

            if (core::sdk::game::IsOnScreen(ScreenHeadPos)) {
                float fovDistance = std::hypot(ScreenHeadPos.x - g_Variables.g_vGameWindowCenter.x, ScreenHeadPos.y - g_Variables.g_vGameWindowCenter.y);
                if (fovDistance < g_Config.Aimbot->FOV) {
                    SetViewAngles(Ped, HeadPos + D3DXVECTOR3(0, 0, 0.08f)); 
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
