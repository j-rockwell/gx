#pragma once
#include "Alpha/Character/AAlphaBaseCharacter.h"
#include "AAlphaGenericCharacter.generated.h"

UCLASS(Config=Game)
class AAlphaGenericCharacter : public AAlphaBaseCharacter
{
	GENERATED_BODY()
	
public:
	AAlphaGenericCharacter();

protected:
	virtual void SpecialA1(const FInputActionValue& Value) override;
	virtual void SpecialA2(const FInputActionValue& Value) override;
};
