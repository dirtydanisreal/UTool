#include "Tool.h"

#include <GameFramework/Actor.h>
#include <Components/BoxComponent.h>
#include "StaticMeshResources.h"

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
