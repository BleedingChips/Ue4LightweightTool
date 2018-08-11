#pragma once
#include "Engine.h"
#ifdef WITH_EDITORONLY_DATA
#include <Editor.h>
void CopySelectionActorToClioboard(const FString& replace);
void MoveToPosition(const FString& replace);
#endif
void CopyCurrentPosition(UWorld*);