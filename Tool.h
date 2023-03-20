#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

UCLASS()
class UTool : public UObject
{
    GENERATED_BODY()

    public:

    template <typename ObjClass>

    static FVector ConvertLocationToActorSpace( FVector Location, AActor* Reference, AActor* Target );

    static FRotator ConvertRotationToActorSpace( FRotator Rotation, AActor* Reference, AActor* Target );

    UFUNCTION(BlueprintCallable,Category="Tools")
    static bool IsPointInsideBox( FVector Point, UBoxComponent* Box );

    UFUNCTION(BlueprintCallable,Category="Tools")
    static ObjClass* LoadObjFromPath(const FName& Path)
    {
        if (Path == NAME_None) return nullptr;

        return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), nullptr, *Path.ToString()));
    }

    UFUNCTION(BlueprintCallable,Category="Tools")
    static UMaterial* LoadMaterialFromPath(const FName& Path)
    {
        if (Path == NAME_None) return nullptr;

        return LoadObjFromPath<UMaterial>(Path);
    }

    UFUNCTION(BlueprintCallable,Category="Tools")
    static UObject* LoadObjectFromAssetPath(TSubclassOf<UObject> ObjectClass, FName Path, bool& IsValid);


    UFUNCTION(BlueprintPure,Category="Tools")
    static FName GetObjectPath(UObject* Obj);
    
    static int32 GetVertices(const USkeletalMesh& Mesh, const int32 LODIndex, 
	TArray<FVector3f>& OutPositions, TArray<FVector3f>& OutNormals);
	
    static int32 FindClosestVertex(const FVector3f& Point, const TArray<FVector3f>& Vertices);
    
    TMap<UClass*, FString> PrefixMap =
	{
		{UBlueprint::StaticClass(), TEXT("BP_")},
		{UStaticMesh::StaticClass(),TEXT("SM_")},
		{UMaterial::StaticClass(), TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
		{UParticleSystem::StaticClass(), TEXT("PS_")},
		{USoundCue::StaticClass(), TEXT("SC_")},
		{USoundWave::StaticClass(), TEXT("SW_")},
		{UTexture::StaticClass(), TEXT("T_")},
		{UTexture2D::StaticClass(), TEXT("T_")},
		{UUserWidget::StaticClass(), TEXT("WBP_")},
		{USkeletalMeshComponent::StaticClass(), TEXT("SK_")},
		{UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")}
	};
};
