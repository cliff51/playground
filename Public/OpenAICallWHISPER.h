// Copyright Kellan Mythen 2021. All rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OpenAIDefinitions.h"
#include "HttpModule.h"
#include "OpenAICallWHISPER.generated.h"

class UOpenAICallGPT;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDalleResponseRecievedPin, const FString, completions, const FString&, errorMessage, const FCompletionInfo&, completionInfo, bool, Success);


/**
 *
 */
UCLASS()
class OPENAIAPI_API UOpenAICallWHISPER : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	FString FilePath = "";
	FGPT3Settings settings;

	UPROPERTY(BlueprintAssignable, Category = "OpenAI")
	FOnWHISPERResponseRecievedPin Finished;

	TArray<uint8> FStringToUint8(const FString& InString);

	FString AddData(FString Name, FString Value);

	FString BoundaryLabel = FString();
	FString BoundaryBegin = FString();
	FString BoundaryEnd = FString();

	static const UOpenAICallWHISPER* Get();
	UPROPERTY(EditAnywhere, Category = "Settings", Meta = (DisplayName = "API Key"))
	FString APIKey;

	void InActivate();

	UPROPERTY(EditAnywhere)
	UOpenAICallGPT* Test333;

private:
	OpenAIValueMapping mapping;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "OpenAI")
	static UOpenAICallDALLE* OpenAICallWhisper(FString prompt);

	virtual void Activate() override;
	void OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
};
