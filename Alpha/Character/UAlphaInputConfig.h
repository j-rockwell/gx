#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnhancedInput/Public/InputAction.h"
#include "UAlphaInputConfig.generated.h"

UCLASS()
class UAlphaInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UInputAction* InputMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UInputAction* InputLook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UInputAction* InputJump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UInputAction* InputSpecial1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UInputAction* InputSpecial2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UInputAction* InputUltimate;
};
