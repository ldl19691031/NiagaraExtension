#include "NiagaraSimpleCustomRenderer.h"
#include "NiagaraRenderer.h"
#include "NiagaraEmitterInstance.h"
#include "NiagaraSystemInstanceController.h"

class FNiagaraSimpleCustomRenderer : public FNiagaraRenderer
{
public:
	FNiagaraSimpleCustomRenderer(ERHIFeatureLevel::Type InFeatureLevel, const UNiagaraRendererProperties* InProps, const FNiagaraEmitterInstance* Emitter);
	virtual ~FNiagaraSimpleCustomRenderer() override;
};

void UNiagaraSimpleCustomRendererProperties::InitCDOPropertiesAfterModuleStartup()
{

}

FNiagaraRenderer* UNiagaraSimpleCustomRendererProperties::CreateEmitterRenderer(ERHIFeatureLevel::Type FeatureLevel, const FNiagaraEmitterInstance* Emitter, const FNiagaraSystemInstanceController& InController)
{
	return nullptr;
}

FNiagaraSimpleCustomRenderer::FNiagaraSimpleCustomRenderer(ERHIFeatureLevel::Type InFeatureLevel, const UNiagaraRendererProperties* InProps, const FNiagaraEmitterInstance* Emitter)
	: FNiagaraRenderer(FeatureLevel, InProps, Emitter)
{
}

FNiagaraSimpleCustomRenderer::~FNiagaraSimpleCustomRenderer()
{
}
