#include "NiagaraSimpleCustomRenderer.h"
#include "NiagaraRenderer.h"
#include "NiagaraEmitterInstance.h"
#include "NiagaraSystemInstanceController.h"
#include "ProceduralMeshComponent.h"
#include "NiagaraConstants.h"
#include "NiagaraEmitter.h"
#include "NiagaraModule.h"
#include "NiagaraParameterStore.h"

TArray<TWeakObjectPtr<UNiagaraSimpleCustomRendererProperties>> UNiagaraSimpleCustomRendererProperties::DeferredInitProperties;

class FNiagaraSimpleCustomRenderer : public FNiagaraRenderer
{
public:
	FNiagaraSimpleCustomRenderer(ERHIFeatureLevel::Type InFeatureLevel, const UNiagaraRendererProperties* InProps, const FNiagaraEmitterInstance* Emitter);
	virtual ~FNiagaraSimpleCustomRenderer() override;

	virtual void Initialize(const UNiagaraRendererProperties* InProps, const FNiagaraEmitterInstance* Emitter, const FNiagaraSystemInstanceController& InController) override;
	virtual void PostSystemTick_GameThread(const UNiagaraRendererProperties* InProperties, const FNiagaraEmitterInstance* Emitter) override;
	void CreateAndInitMeshComponent();
	virtual void OnSystemComplete_GameThread(const UNiagaraRendererProperties* InProperties, const FNiagaraEmitterInstance* Emitter) override;
	virtual void DestroyRenderState_Concurrent();
protected:
	TWeakObjectPtr<UProceduralMeshComponent> MeshComponent;
	TWeakObjectPtr<AActor> SpawnedComponentOwnerActor;


	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
};

void UNiagaraSimpleCustomRendererProperties::InitCDOPropertiesAfterModuleStartup()
{
	UNiagaraSimpleCustomRendererProperties* CDO = CastChecked<UNiagaraSimpleCustomRendererProperties>(StaticClass()->GetDefaultObject());
	CDO->MeshVertexPositionBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_POSITION);

	for (TWeakObjectPtr<UNiagaraSimpleCustomRendererProperties>& WeakRendererProperties : DeferredInitProperties)
	{
		if (WeakRendererProperties.IsValid())
		{
			WeakRendererProperties->InitBindings();
		}
	}
}

FNiagaraRenderer* UNiagaraSimpleCustomRendererProperties::CreateEmitterRenderer(ERHIFeatureLevel::Type FeatureLevel, const FNiagaraEmitterInstance* Emitter, const FNiagaraSystemInstanceController& InController)
{
	FNiagaraRenderer* NewRenderer = new FNiagaraSimpleCustomRenderer(FeatureLevel, this, Emitter);
	NewRenderer->Initialize(this, Emitter, InController);
	return NewRenderer;
}

void UNiagaraSimpleCustomRendererProperties::GetUsedMaterials(const FNiagaraEmitterInstance* InEmitter, TArray<UMaterialInterface*>& OutMaterials) const { 
	if (Material)
	{
		OutMaterials.Add(Material);
	}
}

void UNiagaraSimpleCustomRendererProperties::PostInitProperties()
{
	Super::PostInitProperties();
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		if (FModuleManager::Get().IsModuleLoaded("Niagara") == false)
		{
			DeferredInitProperties.Add(this);
			return;
		}
		else
		{
			InitBindings();
		}
	}
}

void UNiagaraSimpleCustomRendererProperties::InitBindings()
{
	if (MeshVertexPositionBinding.IsValid() == false)
	{
		MeshVertexPositionBinding = FNiagaraConstants::GetAttributeDefaultBinding(SYS_PARAM_PARTICLES_POSITION);
	}
}

FNiagaraSimpleCustomRenderer::FNiagaraSimpleCustomRenderer(ERHIFeatureLevel::Type InFeatureLevel, const UNiagaraRendererProperties* InProps, const FNiagaraEmitterInstance* Emitter)
	: FNiagaraRenderer(FeatureLevel, InProps, Emitter)
{
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
	const UNiagaraSimpleCustomRendererProperties* Props = CastChecked<UNiagaraSimpleCustomRendererProperties>(InProperties);

	if (MeshComponent.IsValid() == false)
	{
		CreateAndInitMeshComponent();
	}

	if (MeshComponent.IsValid())
	{
		if (Props->Material)
		{
			MeshComponent->SetMaterial(0, Props->Material);
		}
	}

	if (MeshComponent.IsValid())
	{
		
		const FNiagaraDataSet& Data = Emitter->GetParticleData();
		const FNiagaraDataBuffer& ParticleData = Data.GetCurrentDataChecked();
		FNiagaraDataSetAccessor<FNiagaraPosition> MeshVertexPositionAccessor(Data, Props->MeshVertexPositionBinding.GetDataSetBindableVariable().GetName());
		FNiagaraDataSetReaderFloat<FNiagaraPosition> PositionReader = MeshVertexPositionAccessor.GetReader(Data);

		if (PositionReader.IsValid())
		{
			const int32 ParticleCount = ParticleData.GetNumInstances();
			TArray<FVector> NewVertices = Vertices;

			for (int32 i = 0; i < FMath::Min(ParticleCount,3); ++i)
			{
				const FVector Position = FVector( PositionReader.GetSafe(i, FNiagaraPosition::ZeroVector));
				NewVertices[i] = Position;
				// Add other mesh data as needed
			}
			MeshComponent->UpdateMeshSection(0, NewVertices, Normals, UV0, VertexColors, Tangents);
		}

	}
}

void FNiagaraSimpleCustomRenderer::CreateAndInitMeshComponent()
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
		MeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
		//MeshComponent->SetMaterial(0, InProps->GetMaterial());
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

void FNiagaraSimpleCustomRenderer::DestroyRenderState_Concurrent()
{
	AsyncTask(
		ENamedThreads::GameThread,
		[Mesh_GT = MoveTemp(MeshComponent), Owner_GT = MoveTemp(SpawnedComponentOwnerActor)]()
	{
		if (Mesh_GT.IsValid())
		{
			Mesh_GT->DestroyComponent();
		}
		if (Owner_GT.IsValid())
		{
			Owner_GT->Destroy();
		}
	}
	);
	SpawnedComponentOwnerActor.Reset();
	MeshComponent.Reset();
}
