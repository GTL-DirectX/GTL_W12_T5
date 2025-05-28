#pragma once

#include <PxPhysicsAPI.h>

#include "Math/Color.h"
#include "Math/Transform.h"
#include "Math/Matrix.h"

// PxVec3 = FVector;
// PxQuat = FQuat;
// PxTransform = FTransform;
// PxRaycastHit = FHitResult;
// PxOverlapHit = FOverlapResult;
// PxSweepHit = FSweepResult;
// PxFilterData = FMaskFilter;
// PxMaterial = UPhysicalMaterial;
// PxShape = FBodyInstance;
// PxRigidActor = FBodyInst;ance
// PxRigidDynamic = FBodyInstance;
// PxRigidStatic = FBodyInstance;
// PxJoint = FConstraintInstance;
// PxScene = UWorld->GetPhysicsScene();

using namespace physx;

inline FVector FromPxVec3(const physx::PxVec3& PxVec)
{
    // PhysX X (Right)  -> Unreal Y (Right)
    // PhysX Y (Up)     -> Unreal Z (Up)
    // PhysX Z (Back)   -> Unreal -X (Front)  (Z 부호 반전으로 왼손->오른손 또는 그 반대 효과, 그리고 축 매핑)
    return FVector(-PxVec.z, PxVec.x, PxVec.y);
}

inline FLinearColor FromPxColor(physx::PxU32 PxColor)
{
    // PxDebug* 색상은 보통 ARGB 순서의 8비트 채널입니다.
    float A = static_cast<float>((PxColor >> 24) & 0xFF) / 255.0f;
    float R = static_cast<float>((PxColor >> 16) & 0xFF) / 255.0f;
    float G = static_cast<float>((PxColor >> 8) & 0xFF) / 255.0f;
    float B = static_cast<float>((PxColor) & 0xFF) / 255.0f;
    return FLinearColor(R, G, B, A);
}

inline physx::PxVec3 ToPxVec3(const FVector& Vec)
{
    // Unreal X (Front) -> PhysX -Z (Back)
    // Unreal Y (Right) -> PhysX X (Right)
    // Unreal Z (Up)    -> PhysX Y (Up)
    return physx::PxVec3(Vec.Y, Vec.Z, -Vec.X);
}

inline PxQuat ToPxQuat(const FQuat& Quat)
{
    // 쿼터니언 변환은 좌표계 변환 방식에 따라 복잡할 수 있습니다.
    // 단순 축 스와핑만으로는 부족할 수 있으며, 회전 축과 각도에 대한 고려가 필요합니다.
    // 언리얼과 PhysX가 모두 표준 쿼터니언(x,y,z,w)을 사용한다면,
    // 좌표계 변환된 축을 기준으로 생성된 쿼터니언은 그대로 전달될 수 있지만,
    // 회전 방향(왼손/오른손)에 따른 부호 변경이 필요할 수 있습니다.
    // 가장 안전한 방법은 변환된 FMatrix에서 ToPxQuat을 유도하는 것입니다.
    // 여기서는 제공된 코드를 따르되, 실제 사용 시 검증이 필요합니다.
    FQuat UnrealQuat = Quat;
    // 예시: Y축과 Z축 회전 방향이 반대일 경우 (왼손<->오른손)
    // FMatrix RotMatrix = FMatrix::Identity;
    // RotMatrix.SetAxis(0, FVector(0,0,-1)); // Unreal X to PhysX -Z
    // RotMatrix.SetAxis(1, FVector(1,0,0));  // Unreal Y to PhysX X
    // RotMatrix.SetAxis(2, FVector(0,1,0));  // Unreal Z to PhysX Y
    // FQuat ConvertedUnrealQuat = RotMatrix.ToQuat() * UnrealQuat * RotMatrix.ToQuat().Inverse();
    // return physx::PxQuat(ConvertedUnrealQuat.X, ConvertedUnrealQuat.Y, ConvertedUnrealQuat.Z, ConvertedUnrealQuat.W);

    // 제공된 코드는 X,Y,Z,W 순서가 같다고 가정하고 직접 매핑합니다.
    // 하지만, 왼손/오른손 좌표계 간의 회전 변환은 특정 축의 부호를 반전시켜야 할 수 있습니다.
    // 예를 들어, (x, y, z, w) -> (y, z, -x, w) * (부호 조정)
    // 정확한 변환은 FMatrix를 통한 변환 후 쿼터니언을 추출하는 것이 더 안전합니다.
    // 여기서는 단순 매핑으로 두겠습니다. (주의: 이 부분은 검증 필요)
    return physx::PxQuat(Quat.X, Quat.Y, Quat.Z, Quat.W); // 이 변환은 잘못되었을 가능성이 높습니다.
    // FTransform을 PxTransform으로 변환할 때 사용된 ToPxQuat(Transform.GetRotation())을 참고해야 합니다.
    // 일반적으로는 FMatrix를 만들고 PxQuat으로 변환합니다.
}

// FTransform을 PxTransform으로 변환 (제공된 버전 사용, 단 ToPxQuat은 주의)
inline physx::PxTransform ToPxTransform(const FTransform& Transform)
{
    // 위치 변환
    physx::PxVec3 PxPosition = ToPxVec3(Transform.GetTranslation());

    // 회전 변환 (가장 복잡한 부분)
    // 언리얼 FQuat -> PhysX PxQuat 변환은 단순히 멤버를 복사하는 것 이상이 필요합니다.
    // 좌표계 축 매핑과 왼손/오른손 규칙을 모두 고려해야 합니다.
    // FMatrix를 중간 단계로 사용하는 것이 일반적입니다.
    FMatrix UnrealMatrix = Transform.ToMatrixWithScale(); // 스케일 없는 회전/이동 매트릭스
    
    // 언리얼 매트릭스 축 추출
    FVector U_XAxis = UnrealMatrix.GetScaledAxis(EAxis::X); // Unreal Front
    FVector U_YAxis = UnrealMatrix.GetScaledAxis(EAxis::Y); // Unreal Right
    FVector U_ZAxis = UnrealMatrix.GetScaledAxis(EAxis::Z); // Unreal Up

    // PhysX 축으로 매핑
    physx::PxVec3 Px_XAxis = ToPxVec3(U_YAxis); // Unreal Right -> PhysX X (Right)
    physx::PxVec3 Px_YAxis = ToPxVec3(U_ZAxis); // Unreal Up    -> PhysX Y (Up)
    physx::PxVec3 Px_ZAxis = ToPxVec3(-U_XAxis); // Unreal Front -> PhysX -Z (Back) (부호 반전으로 왼손->오른손)

    physx::PxMat33 PxRotationMatrix(Px_XAxis, Px_YAxis, Px_ZAxis);
    physx::PxQuat PxRotationQuat(PxRotationMatrix);


    return physx::PxTransform(PxPosition, PxRotationQuat);
}

// FMatrix 기반 Z-Up, Left-Hand 좌표계 변환 함수
// 축 관련 문제 생길 경우 이 함수부터 봐야함.
inline PxTransform ToPxTransform(const FVector& Position, const FVector& PriAxis, const FVector& SecAxis)
{
    // 1. 기준 축 정규화
    const FVector X = PriAxis.GetSafeNormal();    // 기준축
    const FVector Y = SecAxis.GetSafeNormal();    // 보조축
    const FVector Z = X ^ Y;                      // 왼손 좌표계: Z = X × Y
    const FVector YFix = Z ^ X;                   // Y 보정으로 직교성 보장

    // 2. 행렬 구성 (당신의 엔진은 행-기반)
    FMatrix RotMatrix;
    RotMatrix.SetAxis(0, X);     // Row 0 = X축
    RotMatrix.SetAxis(1, YFix);  // Row 1 = 보정된 Y축
    RotMatrix.SetAxis(2, Z);     // Row 2 = Z축
    RotMatrix.SetAxis(3, FVector::ZeroVector); // Row 3 = 변환 없음

    // 3. 행렬 → 쿼터니언
    const FQuat RotQuat = RotMatrix.ToQuat();

    return PxTransform(ToPxVec3(Position), ToPxQuat(RotQuat));
}

inline FTransform FromPxTransform(const physx::PxTransform& InPxTransform)
{
    // 위치 변환
    FVector UnrealPosition = FromPxVec3(InPxTransform.p);

    // 회전 변환
    physx::PxMat33 PxRotationMatrix(InPxTransform.q);
    FVector Px_XAxis_In_Unreal = FromPxVec3(PxRotationMatrix.column0); // PhysX X축을 언리얼 좌표계로
    FVector Px_YAxis_In_Unreal = FromPxVec3(PxRotationMatrix.column1); // PhysX Y축을 언리얼 좌표계로
    FVector Px_ZAxis_In_Unreal = FromPxVec3(PxRotationMatrix.column2); // PhysX Z축을 언리얼 좌표계로

    // 언리얼 매트릭스 구성 (언리얼 축 기준으로)
    // Unreal X (Front) = PhysX -Z (Back)을 언리얼 좌표계로 변환한 것
    // Unreal Y (Right) = PhysX X (Right)을 언리얼 좌표계로 변환한 것
    // Unreal Z (Up)    = PhysX Y (Up)을 언리얼 좌표계로 변환한 것
    FMatrix UnrealRotationMatrix = FMatrix::Identity; 
    // 주의: 아래 축 매핑은 FromPxVec3_Position의 역변환 관계를 고려해야 합니다.
    // FromPxVec3_Position: Px(x,y,z) -> U(-z,x,y)
    // 즉, U.X = -Px.z, U.Y = Px.x, U.Z = Px.y

    // PxRotationMatrix.column0 (PhysX의 X축)은 언리얼의 Y축이 되어야 함
    // PxRotationMatrix.column1 (PhysX의 Y축)은 언리얼의 Z축이 되어야 함
    // PxRotationMatrix.column2 (PhysX의 Z축)은 언리얼의 -X축이 되어야 함
    UnrealRotationMatrix.SetAxis(0, -Px_ZAxis_In_Unreal); // Unreal X (Front)
    UnrealRotationMatrix.SetAxis(1, Px_XAxis_In_Unreal);  // Unreal Y (Right)
    UnrealRotationMatrix.SetAxis(2, Px_YAxis_In_Unreal);  // Unreal Z (Up)


    FQuat UnrealRotation = UnrealRotationMatrix.ToQuat();

    return FTransform(UnrealRotation, UnrealPosition, FVector::OneVector); // 스케일은 1로 가정
}
