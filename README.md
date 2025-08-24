# DirectX 12 3D Game Engine Development(TDD 기반 개발)

이 프로젝트는 DirectX 12 기반 3D 게임 엔진으로, TDD(GTest/GMock)를 통해 안정성과 유지보수성을 확보했습니다.
렌더링 시스템에서는 Shadow Map, SSAO, Skinned Mesh, Instancing 등 게임 그래픽 기법을 지원하며, Renderer/Core 모듈 분리와 인터페이스 기반 설계로 확장성을 확보했습니다.

---

## 프로젝트 구조 및 기반 시스템
- TDD 기반 리소스 관리 및 유닛 테스트 (Google Test / GMock)
- DirectX 12 엔진 초기 구조 설계 및 **Renderer / Core 모듈 분리**
- 업데이트 루프와 렌더링 루프 **분리**
- **모델·뷰 구조 분리** 및 데이터/렌더링 모듈화
- Application 프로젝트 생성 및 Core 모듈 통합
- Renderer를 **인터페이스 기반 구조**로 전환

---

## 렌더링 및 그래픽스 기능
- VRAM 데이터 업로드 로직 구현
- **큐브맵 텍스처** 로딩 및 렌더링 구현
- **Normal Map 적용** 및 HLSL 쉐이더 작성
- **Shadow Map** 렌더링 구현
- **SSAO (Screen Space Ambient Occlusion)** 구현
- **Skinned Mesh** 로딩, 애니메이션 렌더링

---

## 리소스 관리 및 최적화
- RTV, SRV, DSV **인덱스 관리 문제 해결**
- Texture 핸들러 **오프셋 관리 개선**
- Material, Diffuse **인덱싱 로직 개선**
- SetupData 구조 개선 및 **리소스 경로 정리**
- DirectXTK 통합 및 **그래픽 메모리 로딩 로직 교체**

---

## 입력 및 카메라 시스템
- **키 입력 리스너** 및 카메라 제어 시스템 구현
- 윈도우 메시지 처리 및 **KeyInputManager**로 로직 이동
- 컬링 로직을 Camera로 이동하여 **최적화**

---

## 개발 환경
- **HLSL 쉐이더 추가** 및 CSO 파일 관리
