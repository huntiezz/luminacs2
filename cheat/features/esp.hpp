#pragma once

bool GetBoundingBox(C_CSPlayerPawn* Player, ImVec2& TopLeft, ImVec2& BottomRight, Vector_t Origin) {
	if (!Player || !Player->m_pCollision()) return false;

	Vector_t Mins = Player->m_pCollision()->m_vecMins();
	Vector_t Maxs = Player->m_pCollision()->m_vecMaxs();

	Vector_t Points[8] = {
		Origin + Vector_t(Mins.x, Mins.y, Mins.z),
		Origin + Vector_t(Mins.x, Maxs.y, Mins.z),
		Origin + Vector_t(Maxs.x, Maxs.y, Mins.z),
		Origin + Vector_t(Maxs.x, Mins.y, Mins.z),
		Origin + Vector_t(Mins.x, Mins.y, Maxs.z),
		Origin + Vector_t(Mins.x, Maxs.y, Maxs.z),
		Origin + Vector_t(Maxs.x, Maxs.y, Maxs.z),
		Origin + Vector_t(Maxs.x, Mins.y, Maxs.z)
	};

	float Left = FLT_MAX;
	float Top = FLT_MAX;
	float Right = -FLT_MAX;
	float Bottom = -FLT_MAX;

	for (int i = 0; i < 8; ++i) {
		Vector_t Screen;
		if (!VM.WorldToScreen(Points[i], Screen)) return false;

		Left = min(Left, Screen.x);
		Top = min(Top, Screen.y);
		Right = max(Right, Screen.x);
		Bottom = max(Bottom, Screen.y);
	}

	if ((Right - Left) < 1.0f || (Bottom - Top) < 1.0f) return false;

	TopLeft = ImVec2(Left, Top);
	BottomRight = ImVec2(Right, Bottom);

	return true;
}

static std::unordered_map<uintptr_t, float> HealthCache;

ImVec2 GetScaledTextSize(const char* Text, ImFont* Font, float scaledFontSize) {
	return Font->CalcTextSizeA(scaledFontSize, FLT_MAX, 0.0f, Text);
}

namespace ESP {
	void Render() {
		if (Interface::Source2EngineToClient->IsInGame() && Interface::Source2EngineToClient->IsConnected()) {
			if (!Settings::Esp) return;
			if (!CachedLocalPlayer.Player) return;
			if (CachedPlayers.empty()) return;

			for (auto& Player : CachedPlayers) {
				if (!Player.Player) continue;
				if (Settings::EspIgnoreTeam && (Player.TeamNum == CachedLocalPlayer.TeamNum)) continue;

				float Distance = (Player.Origin - CachedLocalPlayer.Origin).Length();
				float BoxThickness = std::clamp(2.0f - (Distance / 800.f), 0.75f, 2.0f);
				float ScaledFontSize = std::clamp(15.0f * 100.0f / Distance, 8.0f, 15.0f);

				ImVec2 TopLeft;
				ImVec2 BottomRight;
				if (!GetBoundingBox(Player.Player, TopLeft, BottomRight, Player.Origin)) continue;

				float BoxX = TopLeft.x;
				float BoxY = TopLeft.y;
				float BoxWidth = BottomRight.x - TopLeft.x;
				float BoxHeight = BottomRight.y - TopLeft.y;

				Ray_t Ray = {};
				TraceFilter_t Filter(0x1C3003, CachedLocalPlayer.Player, nullptr, 7);
				GameTrace_t Trace = {};
				Interface::GameTraceManager->TraceShape(&Ray, GetEyePos(CachedLocalPlayer.Player), Player.BoneData->GetOrigin(6), &Filter, &Trace);

				ImColor EspColor;
				if (Trace.m_pHitEntity != Player.Player) {
					EspColor = Settings::EspColorInvis;
				}
				else {
					EspColor = Settings::EspColorVis;
				}

				if (Settings::BoundingBox) {
					ImGui::GetForegroundDrawList()->AddRect(ImVec2(BoxX - 1, BoxY - 1), ImVec2(BoxX + BoxWidth + 1, BoxY + BoxHeight + 1), ImColor(0, 0, 0, 255), 0.0f, 0, BoxThickness);
					ImGui::GetForegroundDrawList()->AddRect(ImVec2(BoxX + 1, BoxY + 1), ImVec2(BoxX + BoxWidth - 1, BoxY + BoxHeight - 1), ImColor(0, 0, 0, 255), 0.0f, 0, BoxThickness);
					ImGui::GetForegroundDrawList()->AddRect(ImVec2(BoxX, BoxY), ImVec2(BoxX + BoxWidth, BoxY + BoxHeight), EspColor, 0.0f, 0, BoxThickness);
				}

				if (Settings::HealthBar) {
					float& DisplayedHealth = HealthCache[reinterpret_cast<uintptr_t>(Player.Player)];
					float CurrentHealth = static_cast<float>(Player.Health);

					DisplayedHealth += (CurrentHealth - DisplayedHealth) * ImGui::GetIO().DeltaTime * 10.0f;
					if (std::abs(DisplayedHealth - CurrentHealth) < 0.5f) DisplayedHealth = CurrentHealth;

					ImColor HBColor = (DisplayedHealth < 25.0f) ? ImColor(255, 0, 0) : (DisplayedHealth < 65.0f) ? ImColor(255, 165, 0) : ImColor(0, 255, 0);

					float HealthHeight = BoxHeight * (DisplayedHealth / 100.0f);
					ImVec2 HBMin(BoxX - 6, BoxY - 1);
					ImVec2 HBMax(BoxX - 2, BoxY + BoxHeight + 1);
					ImVec2 HBFillMin(BoxX - 5, BoxY + BoxHeight - HealthHeight);
					ImVec2 HBFillMax(BoxX - 3, BoxY + BoxHeight);

					ImGui::GetForegroundDrawList()->AddRect(HBMin, HBMax, ImColor(0, 0, 0, 255), 0, 0, 1.0f);
					ImGui::GetForegroundDrawList()->AddRectFilled(HBMin, HBMax, ImColor(0, 0, 0, 100));
					ImGui::GetForegroundDrawList()->AddRectFilled(HBFillMin, HBFillMax, HBColor);

					if (CurrentHealth < 100) {
						std::string Text = std::to_string(static_cast<int>(CurrentHealth));
						ImVec2 textSize = ImGui::CalcTextSize(Text.c_str());
						float textX = BoxX - 10;
						float textY = HBFillMin.y;

						ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(textX - 1, textY), ImColor(0, 0, 0), Text.c_str());
						ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(textX + 1, textY), ImColor(0, 0, 0), Text.c_str());
						ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(textX, textY - 1), ImColor(0, 0, 0), Text.c_str());
						ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(textX, textY + 1), ImColor(0, 0, 0), Text.c_str());
						ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(textX, textY), ImColor(255, 255, 255), Text.c_str());
					}
				}

				if (Settings::Username) {
					std::string Username = Player.Name;
					ImVec2 TextSize = GetScaledTextSize(Username.c_str(), ImGui::GetFont(), ScaledFontSize);

					float UsernameX = BoxX + BoxWidth * 0.5f - TextSize.x * 0.5f;
					float UsernameY = BoxY - 7.0f - TextSize.y * 0.5f;

					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(UsernameX - 1, UsernameY), ImColor(0, 0, 0, 255), Username.c_str());
					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(UsernameX + 1, UsernameY), ImColor(0, 0, 0, 255), Username.c_str());
					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(UsernameX, UsernameY - 1), ImColor(0, 0, 0, 255), Username.c_str());
					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(UsernameX, UsernameY + 1), ImColor(0, 0, 0, 255), Username.c_str());
					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(UsernameX, UsernameY), ImColor(255, 255, 255, 255), Username.c_str());
				}

				if (Settings::Distance) {
					std::string Dist = std::to_string(static_cast<int>(Distance / 40.0f)) + X("m");
					ImVec2 TextSize = GetScaledTextSize(Dist.c_str(), ImGui::GetFont(), ScaledFontSize);

					float DistX = BoxX + BoxWidth * 0.5f - TextSize.x * 0.5f;
					float DistY = BoxY + BoxHeight + 7.0f - TextSize.y * 0.5f;

					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(DistX - 1, DistY), ImColor(0, 0, 0, 255), Dist.c_str());
					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(DistX + 1, DistY), ImColor(0, 0, 0, 255), Dist.c_str());
					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(DistX, DistY - 1), ImColor(0, 0, 0, 255), Dist.c_str());
					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(DistX, DistY + 1), ImColor(0, 0, 0, 255), Dist.c_str());
					ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), ScaledFontSize, ImVec2(DistX, DistY), ImColor(255, 255, 255, 255), Dist.c_str());
				}
			}
		}
	}
}