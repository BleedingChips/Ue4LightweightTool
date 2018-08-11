#pragma once
#include "Engine.h"
extern ENGINE_API float GAverageMS;
#ifdef WITH_EDITORONLY_DATA
void FilterStaticMesh(const FString& string) noexcept;
void FilterActorClassName(const FString& string) noexcept;
void FilterActorDisplayName(const FString& string) noexcept;
void FilterActorIDName(const FString& string) noexcept;
void StoreSelectedActor() noexcept;
void RestoreSelectedActor() noexcept;
#endif
