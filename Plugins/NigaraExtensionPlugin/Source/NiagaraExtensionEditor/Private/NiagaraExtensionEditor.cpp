// Copyright Epic Games, Inc. All Rights Reserved.

#include "NiagaraExtensionEditor.h"
#include "NiagaraEditorModule.h"
#include "NiagaraSimpleCustomRenderer.h"

#define LOCTEXT_NAMESPACE "FNiagaraExtensionEditorModule"

void FNiagaraExtensionEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FNiagaraEditorModule& NiagaraEditorModule = FNiagaraEditorModule::Get();
	NiagaraEditorModule.RegisterRendererCreationInfo(FNiagaraRendererCreationInfo(
		UNiagaraSimpleCustomRendererProperties::StaticClass()->GetDisplayNameText(),
		FText::FromString(UNiagaraSimpleCustomRendererProperties::StaticClass()->GetDescription()),
		UNiagaraSimpleCustomRendererProperties::StaticClass()->GetClassPathName(),
		FNiagaraRendererCreationInfo::FRendererFactory::CreateLambda([](UObject* OuterEmitter)
		{
		UNiagaraSimpleCustomRendererProperties* NewRenderer = NewObject<UNiagaraSimpleCustomRendererProperties>(OuterEmitter, NAME_None, RF_Transactional);
		return NewRenderer;
	})));
}

void FNiagaraExtensionEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNiagaraExtensionEditorModule, NiagaraExtensionEditor)