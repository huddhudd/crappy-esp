#pragma once
#include "crappy.h"

void UpdateAddresses();
void CacheNames();
DWORD_PTR GetEntityList();
Vector3 GetLocalPlayerPos();
Vector3 GetActorPos(DWORD_PTR entity);
float GetActorHealth(DWORD_PTR pAActor);
int isUaz(int id);
int isDacia(int id);
int isBuggy(int id);
int isBike(int id);
int isBoat(int id);
void DrawSkeleton(DWORD_PTR mesh);
void DrawVehicle(DWORD_PTR entity, Vector3 local, const char* txt);
Vector3 WorldToScreen(Vector3 WorldLocation, FCameraCacheEntry CameraCacheL);
FCameraCacheEntry GetCameraCache();