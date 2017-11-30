#include "Utils.h"

#define UWORLD 0x3A4F1A8
#define GNAMES 0x3951F90

int s_width		= 1600;	// 更新定义分辨率
int s_height	= 900;	// 更新定义分辨率
LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;
HWND hWnd;
HWND twnd;
MARGINS  margin = { 0,0,s_width,s_height };
LPD3DXFONT pFont;
ID3DXLine* d3dLine;

list<int> upper_part	= { Bones::neck_01,		Bones::Head,		Bones::forehead };
list<int> right_arm		= { Bones::neck_01,		Bones::upperarm_r,	Bones::lowerarm_r,	Bones::hand_r };
list<int> left_arm		= { Bones::neck_01,		Bones::upperarm_l,	Bones::lowerarm_l,	Bones::hand_l };
list<int> spine			= { Bones::neck_01,		Bones::spine_01,	Bones::spine_02,	Bones::pelvis };
list<int> lower_right	= { Bones::pelvis,		Bones::thigh_r,		Bones::calf_r,		Bones::foot_r };
list<int> lower_left	= { Bones::pelvis,		Bones::thigh_l,		Bones::calf_l,		Bones::foot_l };

list<list<int>> skeleton = { upper_part, right_arm, left_arm, spine, lower_right, lower_left };

CMem* mem = nullptr;

int ActorIds[4];
int uaz[3];
int dacia[4];
int motorbike[5];
int buggy[3];
int boat = 0;
int itemtype[2];

D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch	= (rot.x * float(M_PI) / 180.f);
	float radYaw	= (rot.y * float(M_PI) / 180.f);
	float radRoll	= (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

Vector3 WorldToScreen(Vector3 WorldLocation, FCameraCacheEntry CameraCacheL)
{
	Vector3 Screenlocation = Vector3(0, 0, 0);

	auto POV = CameraCacheL.POV;

	Vector3 Rotation = POV.Rotation; // FRotator

	D3DMATRIX tempMatrix = Matrix(Rotation); // Matrix

	Vector3 vAxisX, vAxisY, vAxisZ;

	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - POV.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	float FovAngle = POV.FOV;
	float ScreenCenterX = s_width / 2.0f;
	float ScreenCenterY = s_height / 2.0f;

	Screenlocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;

	return Screenlocation;
}

DWORD_PTR GetUWorldPtr()
{
	if (!mem)
		return NULL;

	global::pUWorld				= mem->RPM<DWORD_PTR>(mem->GetBase() + UWORLD, 8);
	global::pGameInstance		= mem->RPM<DWORD_PTR>(global::pUWorld + 0x140, 8);
	global::pLocalPlayerArray	= mem->RPM<DWORD_PTR>(global::pGameInstance + 0x38, 8);
	global::pLocalPlayer		= mem->RPM<DWORD_PTR>(global::pLocalPlayerArray + 0x0, 8);
	global::pViewportClient		= mem->RPM<DWORD_PTR>(global::pLocalPlayer + 0x58, 8);

	return mem->RPM<DWORD_PTR>(global::pViewportClient + 0x80, 8);
}

void UpdateAddresses()
{
	if (!mem)
		return;

	global::pUWorld				= GetUWorldPtr();
	global::pGameInstance		= mem->RPM<DWORD_PTR>(global::pUWorld + 0x140, 8);
	global::pLocalPlayerArray	= mem->RPM<DWORD_PTR>(global::pGameInstance + 0x38, 8);
	global::pLocalPlayer		= mem->RPM<DWORD_PTR>(global::pLocalPlayerArray + 0x0, 8);
	global::cameracache			= GetCameraCache();
}

Vector3 GetLocalPlayerPos()
{
	Vector3 pos;
	if (!mem)
		return pos;
	pos = mem->RPM<Vector3>(global::pLocalPlayer + 0x70, 0xC);

	return pos;
}

Vector3 GetActorPos(DWORD_PTR entity)
{
	Vector3 pos;
	if (!mem)
		return pos;
	auto rootcomp = mem->RPM<DWORD_PTR>(entity + 0x180, 8);
	pos = mem->RPM<Vector3>((DWORD_PTR)rootcomp + 0x174, 0xC);

	return pos;
}

float GetLocalPlayerHealth()
{
	if (!mem)
		return 0.f;
	auto playercontroller = mem->RPM<DWORD_PTR>(global::pLocalPlayer + 0x30, 8);
	auto pawn = mem->RPM<DWORD_PTR>(playercontroller + 0x418, 8);
	return mem->RPM<float>(pawn + 0x1068, 0x4);
}

float GetActorHealth(DWORD_PTR pAActor)
{
	if (!mem)
		return 0.f;
	return mem->RPM<float>(pAActor + 0x1068, 0x4);
}

DWORD_PTR GetEntityList()
{
	if (!mem)
		return 0;
	return mem->RPM<DWORD_PTR>(mem->RPM<DWORD_PTR>(global::pUWorld + 0x30, 0x8) + 0xA0, 0x8);
}

FCameraCacheEntry GetCameraCache()
{
	FCameraCacheEntry cce;
	if (!mem)
		return cce;
	auto playercontroller = mem->RPM<DWORD_PTR>(global::pLocalPlayer + 0x30, 8);
	cce = mem->RPM<FCameraCacheEntry>(mem->RPM<DWORD_PTR>(playercontroller + 0x438, 0x8) + 0x410, sizeof(FCameraCacheEntry));
	return cce;
}


std::string GetNameFromId(int ID)
{
	DWORD_PTR fNamePtr = mem->RPM<DWORD_PTR>(mem->RPM<DWORD_PTR>(mem->GetBase() + GNAMES, 0x8) + int(ID / 0x4000) * 8, 8);
	DWORD_PTR fName = mem->RPM<DWORD_PTR>(fNamePtr + 8 * int(ID % 0x4000), 8);
	char name[64] = { NULL };
	if (ReadProcessMemory(mem->GetHandle(), (LPVOID)(fName + 16), name, sizeof(name) - 2, NULL) != 0)
		return std::string(name);
	return std::string("NULL");
}

void CacheNames()
{
	string name = "";
	for (int i = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0; i < 200000; i++)
	{
		if (c1 == 4 && c2 == 3 && c3 == 4 && c4 == 5 && c5 == 3 && boat != 0 && c6 == 2)
		{
			cout << "IDs retrieved" << endl;
			return;
		}
		name = GetNameFromId(i);
		if (name == "PlayerFemale_A" || name == "PlayerFemale_A_C" || name == "PlayerMale_A" || name == "PlayerMale_A_C")
		{
			cout << name << " " << i << endl;
			ActorIds[c1++] = i;
		}
		if (name == "Uaz_A_01_C" || name == "Uaz_B_01_C" || name == "Uaz_C_01_C")
		{
			uaz[c2++] = i;
			cout << name << " " << i << endl;
		}
		if (name == "Dacia_A_01_C" || name == "Dacia_A_02_C" || name == "Dacia_A_03_C" || name == "Dacia_A_04_C")
		{
			dacia[c3++] = i;
			cout << name << " " << i << endl;
		}
		if (name == "ABP_Motorbike_03_C" || name == "ABP_Motorbike_04_C" || name == "BP_Motorbike_03_C" || name == "BP_Motorbike_04_C" || name == "ABP_Motorbike_03_Sidecart_C")
		{
			cout << name << " " << i << endl;
			motorbike[c4++] = i;
		}
		if (name == "Buggy_A_01_C" || name == "Buggy_A_02_C" || name == "Buggy_A_03_C")
		{
			cout << name << " " << i << endl;
			buggy[c5++] = i;
		}
		if (name == "Boat_PG117_C")
		{
			cout << name << " " << i << endl;
			boat = i;
		}
		if (name == "DroppedItemInteractionComponent" || name == "DroppedItemGroup")
		{
			cout << name << " " << i << endl;
			itemtype[c6++] = i;
		}
	}
}

int isUaz(int id)
{
	for (int i = 0; i < 3; i++)
		if (id == uaz[i])
			return 1;
	return 0;
}

int isDacia(int id)
{
	for (int i = 0; i < 4; i++)
		if (id == dacia[i])
			return 1;
	return 0;
}

int isBuggy(int id)
{
	for (int i = 0; i < 3; i++)
		if (id == buggy[i])
			return 1;
	return 0;
}

int isBike(int id)
{
	for (int i = 0; i < 5; i++)
		if (id == motorbike[i])
			return 1;
	return 0;
}

int isBoat(int id)
{
	return id == boat;
}

void DrawVehicle(DWORD_PTR entity, Vector3 local, const char* txt)
{
	Vector3 pos = GetActorPos(entity);
	Vector3 spos = WorldToScreen(pos, global::cameracache);
	DrawString((int)spos.x, (int)spos.y, D3DCOLOR_ARGB(255, 160, 32, 240), pFont, txt, local.Distance(pos) / 100.f);
}

D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}


FTransform GetBoneIndex(DWORD_PTR mesh, int index)
{
	DWORD_PTR bonearray = mem->RPM<DWORD_PTR>(mesh + 0x790, 0x8);

	return mem->RPM<FTransform>(bonearray + (index * 0x30), sizeof(FTransform));
}

Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id)
{
	FTransform bone = GetBoneIndex(mesh, id);

	FTransform ComponentToWorld = mem->RPM<FTransform>(mesh + 0x190, sizeof(FTransform));
	D3DMATRIX Matrix;
	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());
	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}

void DrawSkeleton(DWORD_PTR mesh)
{
	Vector3 neckpos = GetBoneWithRotation(mesh, Bones::neck_01);
	Vector3 pelvispos = GetBoneWithRotation(mesh, Bones::pelvis);
	Vector3 previous(0, 0, 0);
	Vector3 current, p1, c1;
	for (auto a : skeleton)
	{
		previous = Vector3(0, 0, 0);
		for (int bone : a)
		{
			current = bone == Bones::neck_01 ? neckpos : (bone == Bones::pelvis ? pelvispos : GetBoneWithRotation(mesh, bone));
			if (previous.x == 0.f)
			{
				previous = current;
				continue;
			}
			p1 = WorldToScreen(previous, global::cameracache);
			c1 = WorldToScreen(current, global::cameracache);
			DrawLine(p1.x, p1.y, c1.x, c1.y, D3DCOLOR_ARGB(255, 153, 249, 9));
			previous = current;
		}
	}
}