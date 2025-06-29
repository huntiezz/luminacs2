#pragma once
#include "settings.hpp"
#include <vector>
#include "imgui/custom.hpp"

void Particles() {
	struct Particle {
		ImVec2 Position;
		ImVec2 Velocity;
	};

    static std::vector<Particle> particles;
    static bool initialized = false;
    static const int numParticles = 100;
    static const float maxSpeed = 1.5f;
    static const float connectionRadius = 100.0f;

    if (!initialized) {
        particles.reserve(numParticles);

        for (int i = 0; i < numParticles; ++i) {
            ImVec2 pos = ImVec2(rand() % (int)ImGui::GetIO().DisplaySize.x, rand() % (int)ImGui::GetIO().DisplaySize.y);
            ImVec2 vel = ImVec2((rand() / (float)RAND_MAX - 0.5f) * maxSpeed * 2.0f, (rand() / (float)RAND_MAX - 0.5f) * maxSpeed * 2.0f);

            particles.push_back({ pos, vel });
        }

        initialized = true;
    }

    for (auto& p : particles) {
        p.Position.x += p.Velocity.x;
        p.Position.y += p.Velocity.y;

        if (p.Position.x < 0) p.Position.x = ImGui::GetIO().DisplaySize.x;
        if (p.Position.x > ImGui::GetIO().DisplaySize.x) p.Position.x = 0;
        if (p.Position.y < 0) p.Position.y = ImGui::GetIO().DisplaySize.y;
        if (p.Position.y > ImGui::GetIO().DisplaySize.y) p.Position.y = 0;
    }

    for (const auto& p : particles) {
        ImGui::GetBackgroundDrawList()->AddCircleFilled(p.Position, 2.0f, ImColor(48, 85, 255, 255));
    }

    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            float dx = particles[i].Position.x - particles[j].Position.x;
            float dy = particles[i].Position.y - particles[j].Position.y;
            float distSq = dx * dx + dy * dy;
            if (distSq < connectionRadius * connectionRadius) {
                float alpha = 1.0f - (sqrtf(distSq) / connectionRadius);
                ImGui::GetBackgroundDrawList()->AddLine(particles[i].Position, particles[j].Position, ImColor(48, 85, 255, (int)(alpha * 255.0f)));
            }
        }
    }
}

namespace GUI {
	void Render() {
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImGui::GetIO().DisplaySize, ImColor(0, 0, 0, 160));
        Particles();

        static CustomMenu menu(ImGui::GetBackgroundDrawList());

        menu.Render();

        int tab = menu.GetTab();
        if (tab == 0) {
            menu.Checkbox(X("Enable"), &Settings::Aimbot);

            if (Settings::Aimbot) {
                menu.Checkbox(X("FOV Circle"), &Settings::FOVCircle);
                menu.Checkbox(X("Ignore Team"), &Settings::AimIgnoreTeam);
                menu.Checkbox(X("Visible Only"), &Settings::VisibleOnly);
                menu.SliderInt(X("FOV"), &Settings::FOV, 1, 80);
                menu.SliderInt(X("Smoothing"), &Settings::Smoothing, 5, 100);
                menu.KeyInput(X("Keybind"), &Settings::Keybind);
                menu.Combo(X("Hitbox"), &Settings::CurrentBoneId, Settings::BoneIdNames);
            }
        }
        else if (tab == 1) {
            menu.Checkbox(X("Enable"), &Settings::Esp);

            if (Settings::Esp) {
                menu.Checkbox(X("Bounding Box"), &Settings::BoundingBox);
                menu.Checkbox(X("Health Bar"), &Settings::HealthBar);
                menu.Checkbox(X("Username"), &Settings::Username);
                menu.Checkbox(X("Distance"), &Settings::Distance);
                menu.Checkbox(X("Ignore Team"), &Settings::EspIgnoreTeam);
            }
        }
        else if (tab == 2) {
            menu.ColorPicker(X("FOV Circle"), &Settings::FOVCircleColor);
            menu.ColorPicker(X("Visible"), &Settings::EspColorVis);
            menu.ColorPicker(X("Invisible"), &Settings::EspColorInvis);
        }
        else if (tab == 3) {
            menu.Text(X("Website: luminacheats.com"));
            menu.Text(X("Discord: discord.gg/UmUcE4rXDj"));
            menu.Text(X("Last Updated: " __DATE__ " " __TIME__));
        }
	}
}