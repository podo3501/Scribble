# DirectX 12 Game Engine Development Log

> DirectX 12 기반 게임 엔진 개발 과정에서의 주요 구현 및 리팩토링 내역 정리

## 🎯 프로젝트 구조 및 기반 시스템
- DirectX 12 엔진 초기 구조 설계 및 **Renderer / Core 모듈 분리**
- 업데이트 루프와 렌더링 루프 분리
- 모델·뷰 구조 분리 및 **데이터/렌더링 모듈화**
- Application 프로젝트 생성 및 Core 모듈 통합
- Renderer를 **인터페이스 기반 구조**로 전환

## 🖥️ 렌더링 및 그래픽스 기능
- VRAM 데이터 업로드 로직 구현
- 큐브맵 텍스처 로딩 및 렌더링 구현
- Grid 렌더링 기능 추가
- Normal Map 적용 및 HLSL 쉐이더 작성
- Shadow Map 렌더링 구현
- SSAO(Screen Space Ambient Occlusion) 구현 및 버그 수정
- Skinned Mesh 로딩, 애니메이션 렌더링 및 버그 수정
- Instance, Material, Texture 중복 관리 구조 설계
- 인스턴스 버퍼 인덱스 문제 해결 및 컬링 최적화

## 🛠️ 리소스 관리 및 최적화
- Geometry와 RenderItem 통합
- RTV, SRV, DSV 인덱스 관리 문제 해결
- Texture 핸들러 오프셋 관리 개선
- Material, Diffuse 인덱싱 로직 개선
- SetupData 구조 개선 및 리소스 경로 정리
- DirectXTK 통합 및 그래픽 메모리 로딩 로직 교체

## 🎮 입력 및 카메라 시스템
- 키 입력 리스너 및 카메라 제어 시스템 구현
- 윈도우 메시지 처리 및 `KeyInputManager`로 로직 이동
- 컬링 로직을 Camera로 이동하여 최적화

## 🧪 테스트 및 개발 환경
- Google Test 환경 구축 및 TDD 기반 개발
- HLSL 쉐이더 추가 및 CSO 파일 관리

---
