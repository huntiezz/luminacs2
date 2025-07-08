#pragma once

Vector_t GetEyePos(C_CSPlayerPawn* Entity) {
	if (!Entity) return {};

	Vector_t Origin = Entity->m_vOldOrigin();
	Vector_t ViewOffset = Entity->m_vecViewOffset();
	Vector_t Result = Origin + ViewOffset;

	if (!std::isfinite(Result.x) || !std::isfinite(Result.y) || !std::isfinite(Result.z)) return {};

	return Result;
}

namespace Aim {
	void Run() {
		if (Interface::Source2EngineToClient->IsInGame() && Interface::Source2EngineToClient->IsConnected()) {
			if (!Settings::Aimbot) return;
			if (!ImGui::IsKeyDown((ImGuiKey)Settings::Keybind)) return;
			if (!CachedLocalPlayer.Player) return;
			if (CachedPlayers.empty()) return;

			auto PawnBase = reinterpret_cast<C_CSPlayerPawnBase*>(CachedLocalPlayer.Player);
			QAngle_t* ViewAngle = reinterpret_cast<QAngle_t*>(reinterpret_cast<uintptr_t>(GetModuleHandleA(X("client.dll"))) + 0x1A774E0);
			QAngle_t BestAngle = {};
			float BestFov = Settings::FOV;
			bool FoundTarget = false;

			for (auto& Player : CachedPlayers) {
				if (!Player.Player) continue;
				if (Settings::AimIgnoreTeam && (Player.TeamNum == CachedLocalPlayer.TeamNum)) continue;

				Ray_t Ray = {};
				TraceFilter_t Filter(0x1C3003, CachedLocalPlayer.Player, nullptr, 4);
				GameTrace_t Trace = {};
				Interface::GameTraceManager->TraceShape(&Ray, GetEyePos(CachedLocalPlayer.Player), Player.BoneData[6].Position, &Filter, &Trace);
				if (Settings::VisibleOnly && (Trace.m_pHitEntity != Player.Player || !Trace.IsVisible())) continue;

				QAngle_t TempAngle = CalcAngles(PawnBase->m_vecLastClipCameraPos(), Player.BoneData[Settings::CurrentBoneId].Position);
				float TempFov = GetFov(*ViewAngle, TempAngle);

				if (TempFov < BestFov) {
					BestAngle = TempAngle;
					BestFov = TempFov;
					FoundTarget = true;
				}
			}

			if (FoundTarget) {
				*ViewAngle = SmoothAngle(*ViewAngle, BestAngle, Settings::Smoothing);
			}
		}
	}
}
