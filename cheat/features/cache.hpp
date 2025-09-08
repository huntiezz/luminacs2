#pragma once

class PlayerCache {
public:
	PlayerCache(C_BaseEntity* a1, C_CSPlayerPawn* plyr, int hp, const char* nm, Vector_t pos, Matrix2x4_t* bd, int tnum) : Entity(a1), Player(plyr), Health(hp), Name(nm), Origin(pos), BoneData(bd), TeamNum(tnum) {}

	C_BaseEntity* Entity;
	C_CSPlayerPawn* Player;
	int Health;
	const char* Name;
	Vector_t Origin;
	Matrix2x4_t* BoneData;
	int TeamNum;
};

class LocalPlayerCache {
public:
	C_CSPlayerPawn* Player;
	Vector_t Origin;
	int TeamNum;

	void Update(C_CSPlayerPawn* plyr, Vector_t pos, int tnum) {
		Origin = pos;
		Player = plyr;
		TeamNum = tnum;
	}

	void Reset() {
		Player = nullptr;
		Origin = Vector_t();
		TeamNum = 0;
	}
};

std::vector<PlayerCache> CachedPlayers;
LocalPlayerCache CachedLocalPlayer;

namespace Cache {
	void Run() {
		if (Interface::Source2EngineToClient->IsInGame() && Interface::Source2EngineToClient->IsConnected()) {
			CachedPlayers.clear();

			static uint64_t LocalPlayerIdx = 0;

			for (int i = 1; i <= 64; i++) {
				auto Entity = Interface::GameResourceServiceClient->Instance->Get(i);
				if (!Entity) continue;

				CCSPlayerController* Controller = reinterpret_cast<CCSPlayerController*>(Entity);
				if (!Controller) continue;
				if (!Controller->m_hPawn().valid()) continue;
				if (Controller->m_bIsLocalPlayerController()) {
					LocalPlayerIdx = i;
					continue;
				}

				auto Player = Interface::GameResourceServiceClient->Instance->Get<C_CSPlayerPawn>(Controller->m_hPawn().index());
				if (!Player) continue;
				if (Player->m_iHealth() <= 0) continue;
				if (Player->m_pGameSceneNode()->m_bDormant()) continue;

				auto BoneData = Player->m_pGameSceneNode()->GetSkeletonInstance()->BoneCache;
				if (!BoneData) continue;

				auto LocalEntity = Interface::GameResourceServiceClient->Instance->Get(LocalPlayerIdx);
				if (!LocalEntity) continue;

				auto LocalController = reinterpret_cast<CCSPlayerController*>(LocalEntity);
				if (!LocalController) continue;
				if (!LocalController->m_hPawn().valid()) continue;

				auto LocalPlayer = Interface::GameResourceServiceClient->Instance->Get<C_CSPlayerPawn>(LocalController->m_hPawn().index());
				if (!LocalPlayer) {
					CachedLocalPlayer.Reset();
					continue;
				}
				if (LocalPlayer->m_iHealth() <= 0) {
					CachedLocalPlayer.Reset();
					continue;
				}

				CachedPlayers.emplace_back(Entity, Player, Player->m_iHealth(), Controller->m_sSanitizedPlayerName(), Player->m_vOldOrigin(), BoneData, Player->m_iTeamNum());
				CachedLocalPlayer.Update(LocalPlayer, LocalPlayer->m_vOldOrigin(), LocalPlayer->m_iTeamNum());
			}
		}
	}
}