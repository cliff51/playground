// Copyright Kellan Mythen 2021. All rights Reserved.


#include "OpenAICallGPT.h"
#include "OpenAIUtils.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "OpenAIParser.h"


UOpenAICallGPT::UOpenAICallGPT()
{
}

UOpenAICallGPT::~UOpenAICallGPT()
{
}

UOpenAICallGPT* UOpenAICallGPT::OpenAICallGPT(EOAEngineType engineInput, FString promptInput, FGPT3Settings settingsInput)
{
	UOpenAICallGPT* BPNode = NewObject<UOpenAICallGPT>();
	BPNode->engine = engineInput;
	BPNode->prompt = promptInput;
	BPNode->settings = settingsInput;
	return BPNode;
}

void UOpenAICallGPT::Activate()
{

	FString prompte;

	FString _apiKey = FString(TEXT("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"));


	auto HttpRequest = FHttpModule::Get().CreateRequest();

	FString apiMethod = "gpt-3.5-turbo";

	FString tempPrompt = settings.startSequence + prompte + settings.injectStartText;
	FString tempHeader = "Bearer ";
	tempHeader += _apiKey;

	FString url = FString::Printf(TEXT("https://api.openai.com/v1/chat/completions"));
	HttpRequest->SetURL(url);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Authorization"), tempHeader);

	TSharedPtr<FJsonObject> root = MakeShareable(new FJsonObject());

	root->SetStringField(TEXT("model"), *apiMethod);
	TArray<TSharedPtr<FJsonValue>> myfriends;

	TSharedPtr<FJsonObject> mydog = MakeShareable(new FJsonObject());
	mydog->SetStringField("role", "user");
	mydog->SetStringField("content", prompt);

	TSharedPtr<FJsonValueObject> c = MakeShareable(new FJsonValueObject(mydog));
	myfriends.Add(c);
	root->SetArrayField("messages", myfriends);

	root->SetBoolField("stream", true);

	FString _payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&_payload);
	FJsonSerializer::Serialize(root.ToSharedRef(), Writer);

	// commit request
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetContentAsString(_payload);

	if (HttpRequest->ProcessRequest())
	{
		HttpRequest->OnProcessRequestComplete().BindUObject(this, &UOpenAICallGPT::OnResponse);
	}
	else
	{
		Finished.Broadcast({}, ("Error sending request"), {}, false, {});
	}

}

TArray<FString> UOpenAICallGPT::GetDeltasFromContent(const FString& Content) const
{
	TArray<FString> Deltas;
	Content.ParseIntoArray(Deltas, TEXT("data: "));

	if (Deltas.Top().Contains("[done]", ESearchCase::IgnoreCase))
	{
		Deltas.Pop();
	}
	else
	{
		Deltas.Add(Content);
	}
	return Deltas;
}


void UOpenAICallGPT::OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	if (!WasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error processing request. \n%s \n%s"), *Response->GetContentAsString(), *Response->GetURL());
		if (Finished.IsBound())
		{
			Finished.Broadcast({}, *Response->GetContentAsString(), {}, false, {});
		}

		return;
	}

	TSharedPtr<FJsonObject> responseObject = MakeShareable(new FJsonObject);

	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	TArray<FString> Deltas = GetDeltasFromContent(Response->GetContentAsString());
	DeserializeStreamedResponse(Deltas);

	if (FJsonSerializer::Deserialize(reader, responseObject))
	{
		bool err = responseObject->HasField("error");

		if (err)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Response->GetContentAsString());
			Finished.Broadcast({}, TEXT("Api error"), {}, false, {});
			return;
		}

		OpenAIParser parser(settings);
		TArray<FCompletion> _out;
		FCompletionInfo _info = parser.ParseCompletionInfo(*responseObject);

		auto CompletionsObject = responseObject->GetArrayField(TEXT("choices"));

		GetDeltasFromContent(Response->GetContentAsString());

		for (auto& elem : CompletionsObject)
		{
			_out.Add(parser.ParseCompletion(*elem->AsObject()));

		}
		GetDeltasFromContent(Response->GetContentAsString());
		Finished.Broadcast(_out, "", _info, true, {});

	}
}
FString UOpenAICallGPT::AddData_2(FString Name, FString Value) {
	return Name
		+ ":"
		+ Value;
}

FHttpGPTMessage_1 UOpenAICallGPT::ParseCompletionInfo_2(const FJsonObject& json)
{
	FHttpGPTMessage_1 res = {};

	res.Content = json.GetStringField("object");


	return res;
}

void UOpenAICallGPT::DeserializeStreamedResponse(const TArray<FString>& Deltas)
{
	for (const FString& Delta : Deltas)
	{
		DeserializeSingleResponse(Delta);
		UE_LOG(LogTemp, Log, TEXT("%s"),*Delta);
	}
}

void UOpenAICallGPT::DeserializeSingleResponse(const FString& Content)
{
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	TSharedPtr<FJsonObject> JsonResponse = MakeShareable(new FJsonObject);
	FJsonSerializer::Deserialize(Reader, JsonResponse);

	const TArray<TSharedPtr<FJsonValue>> ChoicesArr = JsonResponse->GetArrayField("choices");

	for (auto Iterator = ChoicesArr.CreateConstIterator(); Iterator; ++Iterator)
	{
		const TSharedPtr<FJsonObject> ChoiceObj = (*Iterator)->AsObject();
		const int32 ChoiceIndex = ChoiceObj->GetIntegerField("index");
		const TSharedPtr<FJsonObject>* DeltaObj;
		if (ChoiceObj->TryGetObjectField("delta", DeltaObj))
		{
			FString ContentStr;
			if ((*DeltaObj)->TryGetStringField("content", ContentStr))
			{
				Finished.Broadcast({}, {}, {}, true, ContentStr);

			}
		}
	}
}
