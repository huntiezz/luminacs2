#pragma once
#include <Windows.h>
#include <array>
#include "virtual.hpp"

ViewMatrix VM;

class CEngineClient {
public:
	bool IsInGame() {
		return Virtual::Call<36U, bool>(this);
	}

	bool IsConnected() {
		return Virtual::Call<37U, bool>(this);
	}
};

class CGameEntitySystem {
public:
	template <typename T = C_BaseEntity>
	T* Get(int i) {
		return reinterpret_cast<T*>(this->GetEntityByIndex(i));
	}

private:
	void* GetEntityByIndex(int i) {
		int64_t v2 = reinterpret_cast<int64_t>(this) + 16;

		if ((unsigned int)i <= 0x7FFE) {
			int v3 = i >> 9;
			if ((unsigned int)v3 <= 0x3F) {
				uint64_t v4 = *reinterpret_cast<uint64_t*>(v2 + 8LL * v3);
				if (v4 != 0) {
					uint64_t v5 = v4 + 120LL * (i & 0x1FF);
					if (v5 != 0) {
						uint32_t StoredIndex = *reinterpret_cast<uint32_t*>(v5 + 16) & 0x7FFF;
						if (StoredIndex == (uint32_t)i) {
							return *reinterpret_cast<void**>(v4 + 120LL * (i & 0x1FF));
						}
					}
				}
			}
		}

		return nullptr;
	}
};

class IGameResourceService {
public:
	MEM_PAD(0x58);
	CGameEntitySystem* Instance;
};

class CCollisionProperty {
public:
	SCHEMA_ADD_OFFSET(Vector_t, m_vecMins, 0x40);
	SCHEMA_ADD_OFFSET(Vector_t, m_vecMaxs, 0x4C);
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
	SCHEMA_ADD_OFFSET(int, m_iHealth, 0x34C);
	SCHEMA_ADD_OFFSET(CCollisionProperty*, m_pCollision, 0x340);
	SCHEMA_ADD_OFFSET(CGameSceneNode*, m_pGameSceneNode, 0x330);
	SCHEMA_ADD_OFFSET(int, m_iTeamNum, 0x3EB);
};

class CCSPlayerController {
public:
	CCSPlayerController(uintptr_t Address) : Address(Address) {};

	SCHEMA_ADD_OFFSET(bool, m_bIsLocalPlayerController, 0x778);
	SCHEMA_ADD_OFFSET(CBaseHandle, m_hPawn, 0x6B4);
	SCHEMA_ADD_OFFSET(const char*, m_sSanitizedPlayerName, 0x850);

private:
	uintptr_t Address;
};

class C_CSPlayerPawn : public C_BaseEntity {
public:
	C_CSPlayerPawn(uintptr_t Address) : Address(Address) {};

	SCHEMA_ADD_OFFSET(Vector_t, m_vOldOrigin, 0x15B0);
	SCHEMA_ADD_OFFSET(Vector_t, m_vecViewOffset, 0xD98);

private:
	uintptr_t Address;
};

class C_CSPlayerPawnBase {
public:
	SCHEMA_ADD_OFFSET(Vector_t, m_vecLastClipCameraPos, 0x1604);
};

struct Ray_t {
	Vector_t vecStart;
	Vector_t vecEnd;
	Vector_t vecMins;
	Vector_t vecMaxs;
	MEM_PAD(0x4);
	uint8_t UnkType;
};

struct SurfaceData_t {
	MEM_PAD(0x8);
	float m_flPenetrationModifier;
	float m_flDamageModifier;
	MEM_PAD(0x4);
	int m_iMaterial;
};

struct Color {
	uint8_t r = 0U, g = 0U, b = 0U, a = 0U;
};

struct TraceHitboxData_t {
	const char* m_name;
	const char* m_sSurfaceProperty;
	const char* m_sBoneName;
	Vector_t m_vecMinBounds;
	Vector_t m_vecMaxBounds;
	float m_flShapeRadius;
	uint32_t m_nBoneNameHash;
	int32_t m_nHitGroup;
	uint8_t m_nShapeType;
	bool m_bTranslationOnly;
	uint32_t m_CRC;
	Color m_cRenderColor;
	uint16_t m_nHitboxId;
	MEM_PAD(0x22);
};

struct GameTrace_t {
	GameTrace_t() = default;

	void* m_pSurface;
	C_CSPlayerPawn* m_pHitEntity;
	TraceHitboxData_t* m_pHitboxData;
	MEM_PAD(0x38);
	uint32_t m_uContents;
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
	int64_t m_uTraceMask;
	std::array< int64_t, 2 > m_v1;
	std::array< int32_t, 4 > m_arrSkipHandles;
	std::array< int16_t, 2 > m_arrCollisions;
	int16_t m_v2;
	uint8_t m_v3;
	uint8_t m_v4;
	uint8_t m_v5;

	TraceFilter_t() = default;
	TraceFilter_t(uint32_t uMask, C_CSPlayerPawn* pSkip1, C_CSPlayerPawn* pSkip2, int nLayer) {
		void(__fastcall * FnInitTraceFilter)(TraceFilter_t*, C_BaseEntity*, uint64_t, uint8_t, uint16_t);
		static auto InitTraceFilter = reinterpret_cast<decltype(FnInitTraceFilter)>(Memory::FindPattern(X("client.dll"), X("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24")));

		InitTraceFilter(this, pSkip1, uMask, 4U, nLayer);
	}
};

class CGameTraceManager {
public:
	bool TraceShape(Ray_t* Ray, Vector_t Start, Vector_t End, TraceFilter_t* Filter, GameTrace_t* GameTrace) {
		using FnTraceShape = bool(__fastcall*)(void*, void*, const Vector_t&, const Vector_t&, void*, void*);
		static auto OriginalTraceShape = reinterpret_cast<FnTraceShape>(Memory::FindPattern(X("client.dll"), X("48 89 5C 24 ? 48 89 4C 24 ? 55 57")));

		return OriginalTraceShape(this, Ray, Start, End, Filter, GameTrace);
	}
};

class CCSGOInput {
public:
	QAngle_t GetViewAngles() {
		int64_t(__fastcall * FnGetViewAngles)(CCSGOInput*, int32_t);
		static auto GetViewAngles = reinterpret_cast<decltype(FnGetViewAngles)>(Memory::FindPattern(X("client.dll"), X("4C 8B C1 85 D2 74 ? 48 8D 05")));

		return *reinterpret_cast<QAngle_t*>(GetViewAngles(this, 0));
	}

	void SetViewAngle(QAngle_t& Angle) {
		int64_t(__fastcall * FnSetViewAngle)(void*, int32_t, QAngle_t&);
		static auto SetViewAngle = reinterpret_cast<decltype(FnSetViewAngle)>(Memory::FindPattern(X("client.dll"), X("85 D2 75 ? 48 63 81")));

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
	CGameTraceManager* GameTraceManager = *reinterpret_cast<CGameTraceManager**>(Memory::GetAbsoluteAddress(Memory::FindPattern(X("client.dll"), X("48 8B 0D ? ? ? ? 48 8D 55 ? 66 89 44 24")), 3, 0));
	CCSGOInput* CSGOInput = *reinterpret_cast<CCSGOInput**>(Memory::GetRelativeAddress(Memory::FindPattern(X("client.dll"), X("48 8B 0D ? ? ? ? 4C 8B C6 8B 10 E8")), 0x3, 0x7));
}

namespace Stuff {
	void Initialize() {
		VM.viewMatrix = (viewmatrix_t*)Memory::GetAbsoluteAddress(Memory::FindPattern(X("client.dll"), X("48 8D 0D ? ? ? ? 48 C1 E0 06")), 3, 0);
	}
}
