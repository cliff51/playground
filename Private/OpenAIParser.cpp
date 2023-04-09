// Copyright Kellan Mythen 2021. All rights Reserved.
#include "OpenAIParser.h"

OpenAIParser::OpenAIParser(const FGPT3Settings& promptSettings)
	: settings(promptSettings)
{
}

OpenAIParser::~OpenAIParser()
{
}

FCompletion OpenAIParser::ParseCompletion(const FJsonObject& json)
{
	FCompletion res = {};
	res.index = json.GetIntegerField(TEXT("index"));
	res.finishReason = json.GetStringField(TEXT("finish_reason"));
	
	if (json.HasField("message"))
	{
		FMessageContent value;

		auto LogProbsObject = json.GetObjectField(TEXT("message"));
		auto TokensLogprobsObject = LogProbsObject.Get();
		auto ContentField = TokensLogprobsObject->GetStringField(TEXT("content"));

		res.message = ContentField;
		res.finishReason = ContentField;
	}
	if(json.HasField("delta"))
	{
			FMessageContent value;

			auto LogProbsObject = json.GetObjectField(TEXT("delta"));
			auto TokensLogprobsObject = LogProbsObject.Get();
			auto ContentField = TokensLogprobsObject->GetStringField(TEXT("content"));

			res.message = ContentField;
			res.finishReason = ContentField;
	}

	return res;
}
FCompletionInfo OpenAIParser::ParseCompletionInfo(const FJsonObject& json)
{
	FCompletionInfo res = {};

	res.id = json.GetStringField("id");
	res.object = json.GetStringField("object");
	res.created = FDateTime::FromUnixTimestamp(json.GetNumberField("created"));

	return res;
}

FString OpenAIParser::ParseGeneratedImage(FJsonObject& json)
{
	FString res = "";
	res = json.GetStringField(TEXT("url"));

	return res;
}
