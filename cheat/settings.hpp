#pragma once
#include <vector>
#include <string>
#include <utility>

namespace Settings {
	inline bool Aimbot = false;
	inline bool FOVCircle = false;
	inline bool AimIgnoreTeam = false;
	inline bool VisibleOnly = false;
	inline int FOV = 5;
	inline int Smoothing = 5;
	inline int Keybind = 642;
	inline int CurrentBoneId = 4;
	static std::vector<std::pair<std::string, int>> BoneIdNames = {
		{ "Head", 6 },
		{ "Neck", 5 },
		{ "Chest", 4 },
		{ "Hip", 0 },
	};

	inline bool Esp = false;
	inline bool BoundingBox = false;
	inline bool HealthBar = false;
	inline bool Username = false;
	inline bool Distance = false;
	inline bool EspIgnoreTeam = false;

	ImColor FOVCircleColor = IM_COL32(255, 255, 255, 255);
	ImColor EspColorVis = IM_COL32(255, 255, 255, 255);
	ImColor EspColorInvis = IM_COL32(255, 0, 0, 255);
}