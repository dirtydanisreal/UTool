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
};