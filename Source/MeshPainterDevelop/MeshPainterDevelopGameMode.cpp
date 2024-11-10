// Copyright Epic Games, Inc. All Rights Reserved.

#include "MeshPainterDevelopGameMode.h"
#include "MeshPainterDevelopCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMeshPainterDevelopGameMode::AMeshPainterDevelopGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
