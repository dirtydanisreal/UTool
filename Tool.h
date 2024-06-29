#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
class UMaterial;
class UMaterialFunction;
class UMaterialExpression;
class UMaterialExpressionMakeMaterialAttributes;
struct FExpressionInput;
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
    static void ResizeRenderTarget(UTextureRenderTarget2D* render_target, float size_x, float size_y);

    UFUNCTION(BlueprintCallable,Category="Tools")
    static UObject* LoadObjectFromAssetPath(TSubclassOf<UObject> ObjectClass, FName Path, bool& IsValid);

    UFUNCTION(BlueprintCallable,Category="Tools")
    static FMatrix GetCameraProjectionMatrix() const;

    UFUNCTION(BlueprintPure,Category="Tools")
    static FName GetObjectPath(UObject* Obj);
    
    UFUNCTION(BlueprintCallable,Category="Tools")
    static int32 GetVertices(const USkeletalMesh& Mesh, const int32 LODIndex, 
	TArray<FVector3f>& OutPositions, TArray<FVector3f>& OutNormals);
	
    UFUNCTION(BlueprintCallable,Category="Tools")	
    static int32 FindClosestVertex(const FVector3f& Point, const TArray<FVector3f>& Vertices);
    
    UFUNCTION(BlueprintCallable, Category = "Tools")
    static void ConvertMaterialToMaterialAttributes(UMaterial* Material);
    
    UFUNCTION(BlueprintCallable, Category = "Tools")
    static void InsertMaterialFunctionBeforeResult(
		UMaterial* Material,
		UMaterialFunction* MaterialFunction,
		bool bOnlyAddIfNotPresent = true);
		
    UFUNCTION(BlueprintCallable, Category = "Open Unreal Utilities|Editor|Material Editing")
    static void OpenMaterialEditorAndJumpToExpression(UMaterialExpression* MaterialExpression);
    
    UFUNCTION(BlueprintCallable,Category="Tools")
    static void CopyMaterialAttributeConnections(
		UMaterial* SourceMaterial,
		UMaterialExpressionMakeMaterialAttributes* TargetMakeMaterialAttributes);
		
    UFUNCTION(BlueprintPure, Category = "Fabulous Utility|Fu Controller Utility", Meta = (DefaultToSelf = "Actor"))
    static APlayerController* GetPlayerControllerFromActor(AActor* Actor);
    
    inline APlayerController* UFuControllerUtility::GetPlayerControllerFromActor(AActor* Actor)
{
	return Cast<APlayerController>(GetControllerFromActor(Actor));
}
    UFUNCTION(BlueprintCallable,Category="Tools")
    static ACharacter* GetCharacter() const;


    UFUNCTION(BlueprintCallable, Category="LyraExt")
    static bool ChangeMeshMaterials(TArray<UStaticMesh*> Mesh, UMaterialInterface* Material);
    
    UFUNCTION(BlueprintCallable,Category="Tools")
    static UClass* FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures = true);

template <typename DesiredClass>
static TSubclassOf<DesiredClass> FindClassByShortName(const FString& SearchToken, bool bLogFailures = true)
{
	return FindClassByShortName(SearchToken, DesiredClass::StaticClass(), bLogFailures);
}
};
