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

	virtual class FNiagaraBoundsCalculator* CreateBoundsCalculator() override { return nullptr; }
	virtual void GetUsedMaterials(const FNiagaraEmitterInstance* InEmitter, TArray<UMaterialInterface*>& OutMaterials) const override { return; }
	virtual void GetRendererWidgets(const FNiagaraEmitterInstance* InEmitter, TArray<TSharedPtr<SWidget>>& OutWidgets, TSharedPtr<FAssetThumbnailPool> InThumbnailPool) const override { return; }
	virtual void GetRendererTooltipWidgets(const FNiagaraEmitterInstance* InEmitter, TArray<TSharedPtr<SWidget>>& OutWidgets, TSharedPtr<FAssetThumbnailPool> InThumbnailPool) const override { return; }

	virtual bool IsSimTargetSupported(ENiagaraSimTarget InSimTarget) const { return InSimTarget == ENiagaraSimTarget::CPUSim; };
	virtual void PostInitProperties() override;
	//~ UNiagaraRendererProperties interface

	virtual bool NeedsSystemPostTick() const override { return true; }
	virtual bool NeedsSystemCompletion() const override { return true; }
public:
	UPROPERTY(EditAnywhere, Category = "Bindings")
	FNiagaraVariableAttributeBinding MeshVertexPositionBinding;

protected:
	void InitBindings();

	static TArray<TWeakObjectPtr<UNiagaraSimpleCustomRendererProperties> > DeferredInitProperties;
};

