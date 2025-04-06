#pragma once
#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraRendererProperties.h"
#include "NiagaraSimpleCustomRenderer.generated.h"

UCLASS(editinlinenew, MinimalAPI, meta = (DisplayName = "Simple Custom Renderer"))
class UNiagaraSimpleCustomRendererProperties : public UNiagaraRendererProperties
{
	GENERATED_BODY()
public:
	static void InitCDOPropertiesAfterModuleStartup();
	//~ UNiagaraRendererProperties interface
	virtual FNiagaraRenderer* CreateEmitterRenderer(ERHIFeatureLevel::Type FeatureLevel, const FNiagaraEmitterInstance* Emitter, const FNiagaraSystemInstanceController& InController) override;
	//~ UNiagaraRendererProperties interface
};

