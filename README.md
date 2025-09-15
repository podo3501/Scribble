# DirectX 12 3D Game Engine Development(TDD 기반 개발)

이 프로젝트는 DirectX 12 기반 3D 게임 엔진으로, TDD(GTest/GMock)를 통해 안정성과 유지보수성을 확보했습니다.
렌더링 시스템에서는 Shadow Map, SSAO, Skinned Mesh, Instancing 등 게임 그래픽 기법을 지원하며, Renderer/Core 모듈 분리와 인터페이스 기반 설계로 확장성을 확보했습니다.

---

## 프로젝트 구조 및 기반 시스템
- TDD 기반 개발(Google Test / GMock)
- DirectX 12 엔진 초기 구조 설계
- Renderer를 **인터페이스 기반 구조**로 전환

---

## 렌더링 및 그래픽스 기능
- **큐브맵 텍스처** 로딩 및 렌더링
- **Normal Map 적용**
- **Shadow Map** 렌더링
- **SSAO (Screen Space Ambient Occlusion)**
- **Skinned Mesh** 로딩, 애니메이션 렌더링

---

## 리소스 관리 및 최적화
- Texture 핸들러 **오프셋 관리 개선**
- DirectXTK 통합 및 **그래픽 메모리 로딩 로직 교체**

---

## 비고
- DirectXTK12를 사용하지 않고 직접 구현하던 중, 오히려 DirectXTK12를 활용하는 편이 좋겠다고 생각하게 되었으며, 이후 프로젝트는 DirectXTK12 기반으로 확장될 수도 있습니다.
