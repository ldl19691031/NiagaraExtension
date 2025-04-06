#include "NiagaraSimpleCustomRenderer.h"
#include "NiagaraRenderer.h"
#include "NiagaraEmitterInstance.h"
#include "NiagaraSystemInstanceController.h"
#include "ProceduralMeshComponent.h"

class FNiagaraSimpleCustomRenderer : public FNiagaraRenderer
{
public:
	FNiagaraSimpleCustomRenderer(ERHIFeatureLevel::Type InFeatureLevel, const UNiagaraRendererProperties* InProps, const FNiagaraEmitterInstance* Emitter);
	virtual ~FNiagaraSimpleCustomRenderer() override;

	virtual void Initialize(const UNiagaraRendererProperties* InProps, const FNiagaraEmitterInstance* Emitter, const FNiagaraSystemInstanceController& InController) override;
	virtual void PostSystemTick_GameThread(const UNiagaraRendererProperties* InProperties, const FNiagaraEmitterInstance* Emitter) override;
	virtual void OnSystemComplete_GameThread(const UNiagaraRendererProperties* InProperties, const FNiagaraEmitterInstance* Emitter) override;
protected:
	TWeakObjectPtr<UProceduralMeshComponent> MeshComponent;
	TWeakObjectPtr<AActor> SpawnedComponentOwnerActor;

};

void UNiagaraSimpleCustomRendererProperties::InitCDOPropertiesAfterModuleStartup()
{

}

FNiagaraRenderer* UNiagaraSimpleCustomRendererProperties::CreateEmitterRenderer(ERHIFeatureLevel::Type FeatureLevel, const FNiagaraEmitterInstance* Emitter, const FNiagaraSystemInstanceController& InController)
{
	FNiagaraRenderer* NewRenderer = new FNiagaraSimpleCustomRenderer(FeatureLevel, this, Emitter);
	NewRenderer->Initialize(this, Emitter, InController);
	return NewRenderer;
}

FNiagaraSimpleCustomRenderer::FNiagaraSimpleCustomRenderer(ERHIFeatureLevel::Type InFeatureLevel, const UNiagaraRendererProperties* InProps, const FNiagaraEmitterInstance* Emitter)
	: FNiagaraRenderer(FeatureLevel, InProps, Emitter)
{
}

FNiagaraSimpleCustomRenderer::~FNiagaraSimpleCustomRenderer()
{
}

void FNiagaraSimpleCustomRenderer::Initialize(const UNiagaraRendererProperties* InProps, const FNiagaraEmitterInstance* Emitter, const FNiagaraSystemInstanceController& InController)
{
	FNiagaraRenderer::Initialize(InProps, Emitter, InController);
}

void FNiagaraSimpleCustomRenderer::PostSystemTick_GameThread(const UNiagaraRendererProperties* InProperties, const FNiagaraEmitterInstance* Emitter)
{
	FNiagaraRenderer::PostSystemTick_GameThread(InProperties, Emitter);
	FNiagaraSystemInstance* SystemInstance = Emitter->GetParentSystemInstance();
	if (SystemInstance == nullptr)
	{
		return;
	}
	USceneComponent* AttachComponent = SystemInstance->GetAttachComponent();
	if (!AttachComponent)
	{
		// we can't attach the components anywhere, so just bail
		return;
	}
	if (SpawnedComponentOwnerActor.IsValid() == false)
	{
		SpawnedComponentOwnerActor = AttachComponent->GetOwner();
		if (SpawnedComponentOwnerActor == nullptr)
		{
			// NOTE: This can happen with spawned systems
			SpawnedComponentOwnerActor = AttachComponent->GetWorld()->SpawnActor<AActor>();
			SpawnedComponentOwnerActor->SetFlags(RF_Transient);
		}
	}
	if (MeshComponent.IsValid() == false)
	{
		AActor* owner = SpawnedComponentOwnerActor.Get();
		MeshComponent = NewObject<UProceduralMeshComponent>(owner, UProceduralMeshComponent::StaticClass(), TEXT("CustomMesh"));
		if (MeshComponent.IsValid())
		{
			MeshComponent->RegisterComponent();
			MeshComponent->AttachToComponent(owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			MeshComponent->SetVisibility(true);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//MeshComponent->SetMaterial(0, InProps->GetMaterial());
			//Add one triangle to Mesh Component
			TArray<FVector> Vertices;
			TArray<int32> Triangles;
			TArray<FVector> Normals;
			TArray<FVector2D> UV0;
			TArray<FColor> VertexColors;
			TArray<FProcMeshTangent> Tangents;
			Vertices.Add(FVector(100, 0, 0));
			Vertices.Add(FVector(-100, 0, 0));
			Vertices.Add(FVector(0, 0, 100));
			Triangles.Add(0);
			Triangles.Add(1);
			Triangles.Add(2);
			Normals.Add(FVector(0, 1, 0));
			UV0.Add(FVector2D(0, 0));
			UV0.Add(FVector2D(0, 1));
			UV0.Add(FVector2D(1, 0));
			VertexColors.Add(FColor::Red);
			Tangents.Add(FProcMeshTangent(0, 1, 0));
			MeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
			//MeshComponent->SetMaterial(0, InProps->GetMaterial());
		}
	}
}

void FNiagaraSimpleCustomRenderer::OnSystemComplete_GameThread(const UNiagaraRendererProperties* InProperties, const FNiagaraEmitterInstance* Emitter)
{
	FNiagaraRenderer::OnSystemComplete_GameThread(InProperties, Emitter);
	if (MeshComponent.IsValid())
	{
		MeshComponent->DestroyComponent();
	}
}
