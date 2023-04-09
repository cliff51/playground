// Copyright Kellan Mythen 2021. All rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OpenAIDefinitions.h"
#include "HttpModule.h"
#include "OpenAICallGPT.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnGptResponseRecievedPin, const TArray<FCompletion>&, completions, const FString&, errorMessage, const FCompletionInfo&, completionInfo, bool, Success, const FString, test);

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGptResponseLoadingPin, const FString, Loadingcompletions, bool, Success);


/**
 * 
 */

USTRUCT(BlueprintType)
struct FHttpGPTMessage_1
{
	GENERATED_BODY()


		UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Role = "user";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Content;

};

UCLASS()
class OPENAIAPI_API UOpenAICallGPT : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UOpenAICallGPT();
	~UOpenAICallGPT();

	EOAEngineType engine = EOAEngineType::TEXT_DAVINCI_002;
	FString prompt = "";
	FGPT3Settings settings;


	FString AddData_2(FString Name, FString Value);
	FHttpGPTMessage_1 ParseCompletionInfo_2(const FJsonObject&);


	UPROPERTY(BlueprintAssignable, Category = "OpenAI")
		FOnGptResponseRecievedPin Finished;


	//UPROPERTY(BlueprintAssignable, Category = "OpenAI")
	//	FOnGptResponseLoadingPin Loading;

	TArray<FString> GetDeltasFromContent(const FString& Content) const;

	void DeserializeStreamedResponse(const TArray<FString>& Deltas);
	void DeserializeSingleResponse(const FString& Content);



private:
	OpenAIValueMapping mapping;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "OpenAI")
		static UOpenAICallGPT* OpenAICallGPT(EOAEngineType engine, FString prompt, FGPT3Settings settings);

	virtual void Activate() override;


	void OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
};