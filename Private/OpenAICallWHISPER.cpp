// Copyright Kellan Mythen 2021. All rights Reserved.

#include "OpenAICallWHISPER.h"
#include "OpenAICallGPT.h"
#include "OpenAIUtils.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "OpenAIParser.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Interfaces/IHttpRequest.h"

void UOpenAICallWHISPER::InActivate()
{
}

UOpenAICallWHISPER* UOpenAICallWHISPER::OpenAICallWhisper(FString promptInput)
{
	UOpenAICallWHISPER* BPNode = NewObject<UOpenAICallWHISPER>();
	BPNode->FilePath = promptInput;

	return BPNode;
}

FString UOpenAICallWHISPER::AddData(FString Name, FString Value) {
	return 
		 BoundaryBegin/* + TEXT("\r\n")*/
		+ FString(TEXT("Content-Disposition: form-data; name=\""))
		+ Name
		+ FString(TEXT("\"\r\n\r\n"))
		+ Value;
}

void UOpenAICallWHISPER::Activate()
{
	TSharedRef<IHttpRequest> httpRequest = FHttpModule::Get().CreateRequest();

	FString url = FString::Printf(TEXT("https://api.openai.com/v1/audio/transcriptions"));

	httpRequest->SetVerb(TEXT("POST"));
	BoundaryLabel = FString(TEXT("e543322540af456f9a3773049ca02529-")) + FString::FromInt(FMath::Rand());
	BoundaryBegin = FString(TEXT("--")) + BoundaryLabel + FString(TEXT("\r\n"));
	BoundaryEnd = FString(TEXT("\r\n--")) + BoundaryLabel + FString(TEXT("--\r\n"));
	FilePath = FString(("C:/workspace"));

	FString FileName = FPaths::GetCleanFilename(FilePath);

	TArray<uint8> UpFileRawData;
	FFileHelper::LoadFileToArray(UpFileRawData, *FilePath);

	FString FileBoundaryString = FString(TEXT("\r\n"))
		+ BoundaryBegin
		+ FString(TEXT("Content-Disposition: form-data; name=\"file\"; filename=\""))
		+ FileName + "\"\r\n"
		+ "Content-Type: audio/mpeg"
		+ FString(TEXT("\r\n\r\n"));

	httpRequest->SetURL(url);
	httpRequest->SetHeader(TEXT("Authorization"), TEXT("Bearer APIKEY"));

	httpRequest->SetHeader("Content-Type", "multipart/form-data;" " boundary=" + BoundaryLabel);
	
	const FString fileHeader2(TEXT("\r\n\r\n"));
	
	TArray<uint8> CombinedContent;
	CombinedContent.Append(FStringToUint8(FileBoundaryString));
	CombinedContent.Append(UpFileRawData);
	CombinedContent.Append(FStringToUint8(fileHeader2));
	CombinedContent.Append(FStringToUint8(AddData("model", "whisper-1")));
	CombinedContent.Append(FStringToUint8(BoundaryEnd));

	httpRequest->SetContent(CombinedContent);

	if (httpRequest->ProcessRequest())
	{
		httpRequest->OnProcessRequestComplete().BindUObject(this, &UOpenAICallWHISPER::OnResponse);
	}
	else
	{
		Finished.Broadcast({}, ("Error sending request"), {}, false);
	}
}

void UOpenAICallWHISPER::OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	if (!WasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error processing request. \n%s \n%s"), *Response->GetContentAsString(), *Response->GetURL());
		if (Finished.IsBound())
		{
			Finished.Broadcast({}, *Response->GetContentAsString(), {}, false);
		}

		return;
	}

	TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if (FJsonSerializer::Deserialize(reader, JsonObject))
	{
		bool err = JsonObject->HasField("error");

		if (err)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Response->GetContentAsString());
			Finished.Broadcast({}, TEXT("Api error"), {}, false);
			return;
		}

		OpenAIParser parser(settings);
		TArray<FCompletion> _out;
		FCompletionInfo _info = parser.ParseCompletionInfo(*JsonObject);

		auto CompletionsObject2 = JsonObject->GetStringField(TEXT("text"));

		FCompletion res = {};

		Finished.Broadcast(CompletionsObject2, "", _info, true);

		res.message_whisper = CompletionsObject2;
	}
}

TArray<uint8> UOpenAICallWHISPER::FStringToUint8(const FString& InString)
{
	TArray<uint8> OutBytes;

	if (InString.Len() > 0)
	{
		FTCHARToUTF8 Converted(*InString); // Convert to UTF8
		OutBytes.Append(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
	}

	return OutBytes;
}

