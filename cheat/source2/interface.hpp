#pragma once
#include <Windows.h>
#include <array>
#include "virtual.hpp"

inline void* (__fastcall* OriginalGetBaseEntity)(void*, int);

ViewMatrix VM;

class CEngineClient {
public:
	bool IsInGame() {
		return Virtual::Call<35U, bool>(this);
	}

	bool IsConnected() {
		return Virtual::Call<36U, bool>(this);
	}
};

class CGameEntitySystem {
public:
	template <typename T = C_BaseEntity>
	T* Get(int nIndex) {
		return reinterpret_cast<T*>(this->GetEntityByIndex(nIndex));
	}

private:
	void* GetEntityByIndex(int nIndex) {
		return OriginalGetBaseEntity(this, nIndex);
	}
};

class IGameResourceService {
public:
	MEM_PAD(0x58);
	CGameEntitySystem* Instance;
};

class CCollisionProperty {
public:
	std::uint16_t CollisionMask() {
		return *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uintptr_t>(this) + 0x38);
	}

	SCHEMA_ADD_OFFSET(Vector_t, m_vecMins, 0x40);
	SCHEMA_ADD_OFFSET(Vector_t, m_vecMaxs, 0x4C);
	SCHEMA_ADD_OFFSET(std::uint8_t, m_usSolidFlags, 0x5A);
};

class CGameSceneNode {
public:
	SCHEMA_ADD_OFFSET(bool, m_bDormant, 0xEF);
};

struct CBoneData {
	Vector_t Position;
	MEM_PAD(0x14);
};
class CSkeletonInstance : public CGameSceneNode {
public:
	CBoneData* GetBoneData() {
		return *(CBoneData**)((DWORD64)this + 0x170 + 0x80);
	}
};

class C_BaseEntity {
public:
	SCHEMA_ADD_OFFSET(int, m_iHealth, 0x344);
	SCHEMA_ADD_OFFSET(CCollisionProperty*, m_pCollision, 0x338);
	SCHEMA_ADD_OFFSET(CGameSceneNode*, m_pGameSceneNode, 0x328);
	SCHEMA_ADD_OFFSET(CBaseHandle, m_hOwnerEntity, 0x440);
	SCHEMA_ADD_OFFSET(int, m_iTeamNum, 0x3E3);
};

class CCSPlayerController {
public:
	CCSPlayerController(uintptr_t Address) : Address(Address) {};

	SCHEMA_ADD_OFFSET(bool, IsLocalPlayer, 0x6F0);
	SCHEMA_ADD_OFFSET(CBaseHandle, m_hPawn, 0x62C);
	SCHEMA_ADD_OFFSET(const char*, m_sSanitizedPlayerName, 0x778);

private:
	uintptr_t Address;
};

class C_CSPlayerPawn : public C_BaseEntity {
public:
	C_CSPlayerPawn(uintptr_t Address) : Address(Address) {};

	SCHEMA_ADD_OFFSET(Vector_t, m_vOldOrigin, 0x1324);
	SCHEMA_ADD_OFFSET(Vector_t, m_vecViewOffset, 0xCB0);

	std::uint32_t GetOwnerHandleIndex() {
		std::uint32_t Result = -1;

		if (this && this->m_pCollision() && !(this->m_pCollision()->m_usSolidFlags()) & 4) {
			Result = this->m_hOwnerEntity().index();
		}

		return Result;
	}

	std::uint16_t GetCollisionMask() {
		if (this && this->m_pCollision()) {
			return this->m_pCollision()->CollisionMask();
		}
	}

private:
	uintptr_t Address;
};

class C_CSPlayerPawnBase {
public:
	SCHEMA_ADD_OFFSET(Vector_t, m_vecLastClipCameraPos, 0x1384);
};

struct Ray_t {
	Vector_t vecStart;
	Vector_t vecEnd;
	Vector_t vecMins;
	Vector_t vecMaxs;
	MEM_PAD(0x4);
	std::uint8_t UnkType;
};

struct SurfaceData_t {
	MEM_PAD(0x8)
		float m_flPenetrationModifier;
	float m_flDamageModifier;
	MEM_PAD(0x4)
		int m_iMaterial;
};

struct TraceHitboxData_t {
	MEM_PAD(0x38);
	int m_nHitGroup;
	MEM_PAD(0x4);
	int m_nHitboxId;
};

struct GameTrace_t {
	GameTrace_t() = default;

	bool IsVisible() const {
		return (m_flFraction > 0.97f);
	}

	void* m_pSurface;
	C_CSPlayerPawn* m_pHitEntity;
	TraceHitboxData_t* m_pHitboxData;
	MEM_PAD(0x38);
	std::uint32_t m_uContents;
	MEM_PAD(0x24);
	Vector_t m_vecStartPos;
	Vector_t m_vecEndPos;
	Vector_t m_vecNormal;
	Vector_t m_vecPosition;
	MEM_PAD(0x4);
	float m_flFraction;
	MEM_PAD(0x6);
	bool m_bAllSolid;
	MEM_PAD(0x4D)
};

struct TraceFilter_t {
	MEM_PAD(0x8);
	std::int64_t m_uTraceMask;
	std::array< std::int64_t, 2 > m_v1;
	std::array< std::int32_t, 4 > m_arrSkipHandles;
	std::array< std::int16_t, 2 > m_arrCollisions;
	std::int16_t m_v2;
	std::uint8_t m_v3;
	std::uint8_t m_v4;
	std::uint8_t m_v5;

	TraceFilter_t() = default;
	TraceFilter_t(std::uint32_t uMask, C_CSPlayerPawn* pSkip1, C_CSPlayerPawn* pSkip2, int nLayer) {
		m_uTraceMask = uMask;
		m_v1[0] = m_v1[1] = 0;
		m_v2 = 7;
		m_v3 = nLayer;
		m_v4 = 0x49;
		m_v5 = 0;

		if (pSkip1 != nullptr) {
			m_arrSkipHandles[0] = pSkip1->m_hOwnerEntity().index();
			m_arrSkipHandles[2] = pSkip1->GetOwnerHandleIndex();
			m_arrCollisions[0] = pSkip1->GetCollisionMask();
		}

		if (pSkip2 != nullptr) {
			m_arrSkipHandles[1] = pSkip2->m_hOwnerEntity().index();
			m_arrSkipHandles[3] = pSkip2->GetOwnerHandleIndex();
			m_arrCollisions[1] = pSkip2->GetCollisionMask();
		}
	}
};

class CGameTraceManager {
public:
	bool TraceShape(Ray_t* Ray, Vector_t Start, Vector_t End, TraceFilter_t* Filter, GameTrace_t* GameTrace) {
		using FnTraceShape = bool(__fastcall*)(void*, void*, const Vector_t&, const Vector_t&, void*, void*);
		static auto OriginalTraceShape = reinterpret_cast<FnTraceShape>(Memory::FindPattern(X("client.dll"), X("48 89 5C 24 20 48 89 4C 24 08 55 56 41")));

		return OriginalTraceShape(this, Ray, Start, End, Filter, GameTrace);
	}
};

class CCSGOInput {
public:
	QAngle_t GetViewAngles() {
		int64_t(__fastcall * FnGetViewAngles)(CCSGOInput*, int32_t);
		static auto GetViewAngles = reinterpret_cast<decltype(FnGetViewAngles)>(Memory::FindPattern(X("client.dll"), X("4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3")));

		return *reinterpret_cast<QAngle_t*>(GetViewAngles(this, 0));
	}

	void SetViewAngle(QAngle_t& Angle) {
		int64_t(__fastcall * FnSetViewAngle)(void*, int32_t, QAngle_t&);
		static auto SetViewAngle = reinterpret_cast<decltype(FnSetViewAngle)>(Memory::FindPattern(X("client.dll"), X("85 D2 75 3F 48")));

		SetViewAngle(this, 0, std::ref(Angle));
	}
};

namespace Interface {
	typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

	template <typename T>
	T* CreateInterface(const char* pModuleName, const char* pInterfaceName) {
		CreateInterfaceFn CreateInterface = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA(pModuleName), X("CreateInterface"));
		int ReturnCode = 0;

		return (T*)CreateInterface(pInterfaceName, &ReturnCode);
	}

	CEngineClient* Source2EngineToClient = CreateInterface<CEngineClient>(X("engine2.dll"), X("Source2EngineToClient001"));
	IGameResourceService* GameResourceServiceClient = CreateInterface<IGameResourceService>(X("engine2.dll"), X("GameResourceServiceClientV001"));
	void* Source2Client = CreateInterface<void*>(X("client.dll"), X("Source2Client002"));
	CGameTraceManager* GameTraceManager = *reinterpret_cast<CGameTraceManager**>(Memory::GetAbsoluteAddress(Memory::FindPattern(X("client.dll"), X("48 8B 0D ? ? ? ? 4C 8B C3 66 89 44 24")), 3, 0));
	CCSGOInput* CSGOInput = *reinterpret_cast<CCSGOInput**>(Memory::GetRelativeAddress(Memory::FindPattern(X("client.dll"), X("48 8B 0D ? ? ? ? 4C 8B C6 8B 10 E8")), 0x3, 0x7));
}

namespace Stuff {
	void Initialize() {
		OriginalGetBaseEntity = reinterpret_cast<decltype(OriginalGetBaseEntity)>(Memory::FindPattern(X("client.dll"), X("81 FA ? ? ? ? 77 ? 8B C2 C1 F8 ? 83 F8 ? 77 ? 48 98 48 8B 4C C1 ? 48 85 C9 74 ? 8B C2 25 ? ? ? ? 48 6B C0 ? 48 03 C8 74 ? 8B 41 ? 25 ? ? ? ? 3B C2 75 ? 48 8B 01")));
		VM.viewMatrix = (viewmatrix_t*)Memory::GetAbsoluteAddress(Memory::FindPattern(X("client.dll"), X("48 8D 0D ? ? ? ? 48 C1 E0 06")), 3, 0);
	}
}
