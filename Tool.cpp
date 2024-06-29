#include "Tool.h"

#include <GameFramework/Actor.h>
#include <Components/BoxComponent.h>
#include "StaticMeshResources.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "RawMesh.h"
#include "MeshUtilities.h"
#include "AssetRegistryModule.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Materials/MaterialInterface.h"
#include "Math/Vector.h"
#include "Math/NumericLimits.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MeshDescription.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MaterialEditingLibrary.h"
#include "Engine/StaticMesh.h"
#include "EditorSupportDelegates.h"
#include "Engine/Engine.h"
#include "AssetRegistryModule.h"

FVector UTool::ConvertLocationToActorSpace( FVector Location, AActor* Reference, AActor* Target )
{
    if( Reference == nullptr || Target == nullptr )
    {
        return FVector::ZeroVector;
    }

    FVector Direction       = Location - Reference->GetActorLocation();
    FVector TargetLocation  = Target->GetActorLocation();

    FVector Dots;
    Dots.X  = FVector::DotProduct( Direction, Reference->GetActorForwardVector() );
    Dots.Y  = FVector::DotProduct( Direction, Reference->GetActorRightVector() );
    Dots.Z  = FVector::DotProduct( Direction, Reference->GetActorUpVector() );

    FVector NewDirection    = Dots.X * Target->GetActorForwardVector()
                            + Dots.Y * Target->GetActorRightVector()
                            + Dots.Z * Target->GetActorUpVector();

    return TargetLocation + NewDirection;
}

void UTool::ResizeRenderTarget(UTextureRenderTarget2D* render_target, float size_x, float size_y)
{
                if (render_target == nullptr)
                                return;
                render_target->ResizeTarget(size_x, size_y);
}


FRotator UTool::ConvertRotationToActorSpace( FRotator Rotation, AActor* Reference, AActor* Target )
{
    if( Reference == nullptr || Target == nullptr )
    {
        return FRotator::ZeroRotator;
    }

    FTransform SourceTransform  = Reference->GetActorTransform();
    FTransform TargetTransform  = Target->GetActorTransform();
    FQuat QuatRotation          = FQuat( Rotation );

    FQuat LocalQuat             = SourceTransform.GetRotation().Inverse() * QuatRotation;
    FQuat NewWorldQuat          = TargetTransform.GetRotation() * LocalQuat;

    return NewWorldQuat.Rotator();
}

bool UTool::IsPointInsideBox( FVector Point, UBoxComponent* Box )
{
    if( Box != nullptr )
    {
        // From :
        // https://stackoverflow.com/questions/52673935/check-if-3d-point-inside-a-box/52674010

        FVector Center      = Box->GetComponentLocation();
        FVector Half        = Box->GetScaledBoxExtent();
        FVector DirectionX  = Box->GetForwardVector();
        FVector DirectionY  = Box->GetRightVector();
        FVector DirectionZ  = Box->GetUpVector();

        FVector Direction   = Point - Center;

        bool IsInside = FMath::Abs( FVector::DotProduct( Direction, DirectionX ) ) <= Half.X &&
                        FMath::Abs( FVector::DotProduct( Direction, DirectionY ) ) <= Half.Y &&
                        FMath::Abs( FVector::DotProduct( Direction, DirectionZ ) ) <= Half.Z;

        return IsInside;
    }
    else
    {
        return false;
    }
}

UObject* UTool::LoadObjectFromAssetPath(TSubclassOf<UObject> ObjectClass,FName Path,bool& IsValid)
{
    IsValid = false;
	
	if(Path == NAME_None) return NULL;
	//~~~~~~~~~~~~~~~~~~~~~
	
	UObject* LoadedObj = StaticLoadObject( ObjectClass, NULL,*Path.ToString());
	 
	IsValid = LoadedObj != nullptr;
	
	return LoadedObj;
}

FName UTool::GetObjectPath(UObject* Obj)
{
    if(!Obj) return NAME_None;
	if(!Obj->IsValidLowLevel()) return NAME_None;
	//~~~~~~~~~~~~~~~~~~~~~~~~~
	
	FStringAssetReference ThePath = FStringAssetReference(Obj);
		
	if(!ThePath.IsValid()) return "";
	
	//The Class FString Name For This Object
	FString str=Obj->GetClass()->GetDescription();
	
	//Remove spaces in Material Instance Constant class description!
	str = str.Replace(TEXT(" "),TEXT(""));
	
	str += "'";
	str += ThePath.ToString();
	str += "'";
	
	return FName(*str);
}

bool UTool::GetStaticMeshVertexLocations(UStaticMeshComponent* Comp, TArray<FVector>& VertexPositions)
{
    VertexPositions.Empty();
	 
	if(!Comp) 						
	{
		return false;
	}
	
	if(!Comp->IsValidLowLevel()) 
	{
		return false;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~
	
	//Component Transform
	FTransform RV_Transform = Comp->GetComponentTransform(); 
	
	//Body Setup valid?
	UBodySetup* BodySetup = Comp->GetBodySetup();
	
	if(!BodySetup || !BodySetup->IsValidLowLevel())
	{
		return false;
	}  
	
	for(PxTriangleMesh* EachTriMesh : BodySetup->TriMeshes)
	{
		if (!EachTriMesh)
		{
			return false;
		}
		//~~~~~~~~~~~~~~~~

		//Number of vertices
		PxU32 VertexCount = EachTriMesh->getNbVertices();

		//Vertex array
		const PxVec3* Vertices = EachTriMesh->getVertices();

		//For each vertex, transform the position to match the component Transform 
		for (PxU32 v = 0; v < VertexCount; v++)
		{
			VertexPositions.Add(RV_Transform.TransformPosition(P2UVector(Vertices[v])));
		}
	}
	return true;
	
	/*
	//See this wiki for more info on getting triangles
	//		https://wiki.unrealengine.com/Accessing_mesh_triangles_and_vertex_positions_in_build
	*/
}

int32 UTool::GetVertices(const USkeletalMesh& Mesh, const int32 LODIndex, 
	TArray<FVector3f>& OutPositions, TArray<FVector3f>& OutNormals)
{
	OutPositions.Reset();
	OutNormals.Reset();
	
	const FSkeletalMeshRenderData* RenderData = Mesh.GetResourceForRendering();
	check(RenderData);

	if (!RenderData->LODRenderData.IsValidIndex(LODIndex))
	{
		return INDEX_NONE;
	}

	// Get LOD Data
	const FSkeletalMeshLODRenderData& LODRenderData = RenderData->LODRenderData[LODIndex];

	// Get Total Num of Vertices (for all sections)
	const int32 NumVertices = LODRenderData.GetNumVertices();
	OutPositions.SetNumUninitialized(NumVertices);
	OutNormals.SetNumUninitialized(NumVertices);
	
	for (int32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
	{
		OutPositions[VertexIndex] = LODRenderData.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
		OutNormals[VertexIndex] = LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);
	};

	return NumVertices;
}

int32 UTool::FindClosestVertex(const FVector3f& Point, const TArray<FVector3f>& Vertices)
{
	float MinDistance = TNumericLimits<float>::Max();
	int32 ClosestIndex = INDEX_NONE;

	// Find Closest SkeletalMesh Vertex.
	for (int32 VertexIndex = 0; VertexIndex < Vertices.Num(); ++VertexIndex)
	{
		const float Distance = FVector3f::Dist(Point, Vertices[VertexIndex]);
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			ClosestIndex = VertexIndex;
		}
	}

	return ClosestIndex;
}

bool UTool::ChangeMeshMaterials(TArray<UStaticMesh*> Mesh, UMaterialInterface* Material)
{

	for (int i = 0; i < Mesh.Num(); i++)
	{
		Mesh[i]->Modify();
		TArray<FStaticMaterial>& Mats = Mesh[i]->GetStaticMaterials();
		for (int j = 0; j < Mats.Num(); j++)
		{
			Mats[j].MaterialInterface = Material;

		}
		Mesh[i]->PostEditChange();
	}
	return true;
}

UClass* UTool::FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass)
{
	FString TargetName = TargetNameRaw;
	TargetName.RemoveFromEnd(TEXT("_C"), ESearchCase::CaseSensitive);

	TArray<FAssetData> BlueprintList = ULyraDevelopmentStatics::GetAllBlueprints();
	for (const FAssetData& AssetData : BlueprintList)
	{
		if ((AssetData.AssetName.ToString() == TargetName) || (AssetData.ObjectPath.ToString() == TargetName))
		{
			if (UBlueprint* BP = Cast<UBlueprint>(AssetData.GetAsset()))
			{
				if (UClass* GeneratedClass = BP->GeneratedClass)
				{
					if (GeneratedClass->IsChildOf(DesiredBaseClass))
					{
						return GeneratedClass;
					}
				}
			}
		}
	}

	return nullptr;
}

UClass* UTool::FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures)
{
	check(DesiredBaseClass);

	FString TargetName = SearchToken;

	// Check native classes and loaded assets first before resorting to the asset registry
	bool bIsValidClassName = true;
	if (TargetName.IsEmpty() || TargetName.Contains(TEXT(" ")))
	{
		bIsValidClassName = false;
	}
	else if (!FPackageName::IsShortPackageName(TargetName))
	{
		if (TargetName.Contains(TEXT(".")))
		{
			// Convert type'path' to just path (will return the full string if it doesn't have ' in it)
			TargetName = FPackageName::ExportTextPathToObjectPath(TargetName);

			FString PackageName;
			FString ObjectName;
			TargetName.Split(TEXT("."), &PackageName, &ObjectName);

			const bool bIncludeReadOnlyRoots = true;
			FText Reason;
			if (!FPackageName::IsValidLongPackageName(PackageName, bIncludeReadOnlyRoots, &Reason))
			{
				bIsValidClassName = false;
			}
		}
		else
		{
			bIsValidClassName = false;
		}
	}

	UClass* ResultClass = nullptr;
	if (bIsValidClassName)
	{
		if (FPackageName::IsShortPackageName(TargetName))
		{
			ResultClass = FindObject<UClass>(ANY_PACKAGE, *TargetName);
		}
		else
		{
			ResultClass = FindObject<UClass>(nullptr, *TargetName);
		}
	}

	// If we still haven't found anything yet, try the asset registry for blueprints that match the requirements
	if (ResultClass == nullptr)
	{
		ResultClass = FindBlueprintClass(TargetName, DesiredBaseClass);
	}

	// Now validate the class (if we have one)
	if (ResultClass != nullptr)
	{
		if (!ResultClass->IsChildOf(DesiredBaseClass))
		{
			if (bLogFailures)
			{
				UE_LOG(LogConsoleResponse, Warning, TEXT("Found an asset %s but it wasn't of type %s"), *ResultClass->GetPathName(), *DesiredBaseClass->GetName());
			}
			ResultClass = nullptr;
		}
	}
	else
	{
		if (bLogFailures)
		{
			UE_LOG(LogConsoleResponse, Warning, TEXT("Failed to find class of type %s named %s"), *DesiredBaseClass->GetName(), *SearchToken);
		}
	}

	return ResultClass;
}

ACharacter* UTool::GetCharacter()
{
	return UGameplayStatics::GetPlayerCharacter(GetWorld(),0);	
}
