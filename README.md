# AimLab — 에임 트레이너 (C++ / OpenGL)

CG 텀프로젝트(AimLab 모작)를 다시 정리한 프로젝트. 시작 화면 → 3라운드 → 최종 결과까지 동작한다.

## 빌드·실행

1. `CG_Term_Project.sln`을 VS 2022 또는 VS 2026으로 연다 (툴셋 v143/v145 자동 선택, x64만).
2. 빌드하면 `x64\Debug\AimLab.exe` 옆에 DLL·`Shaders\`·`Resources\`가 복사된다.
3. F5 또는 `AimLab.exe` 직접 실행.

라이브러리(freeglut·GLEW·GLM)는 `External/`에 들어 있어 따로 설치할 필요가 없다 → `External/README.md`.
총 모델은 `AimLab/Resources/Gun.obj` + `Gun.mtl` + `Gun.png`(텍스처)이고, 크기·방향은 자동으로 맞춘다 → `AimLab/Resources/README.md`.

## 게임 흐름

```
시작 화면(R로 시작)
  → 1라운드 GridShot  (30초, 정지 표적 3개를 클릭. 명중 ≥ 20 이면 통과)
  → 2라운드 Tracking  (움직이는 표적 6개를 좌클릭 누른 채 2초 따라가면 처치, 40초. 처치 ≥ 4 이면 통과)
  → 3라운드 Boss      (순간이동 보스 HP 34를 0으로, 75초. HP 22·11에서 보호막 → 좌클릭 누른 채 따라가야 깨짐)
  → 최종 결과(10초, ALL CLEAR 또는 "몇 라운드에서 실패") → 시작 화면
```

- 통과 점수를 못 넘기면 그 라운드 결과(5초)를 보여 준 뒤 다음 라운드 없이 최종 결과로 간다.
- 보스 보호막: 켜지면 총알이 막히고 보스가 순간이동 없이 빠르게 움직인다. 좌클릭을 누른 채 보스를 조준하면 게이지가 차고,
  다 차면 보호막이 깨지며 순간이동 후 다음 페이즈로 넘어간다. 조준하지 않으면 게이지가 조금씩 줄어든다.
- 플레이어는 맵 중앙에서 14m 떨어진 곳에서 중앙을 바라본다. 표적·보스는 중앙 구역(GridShot 8×8×8, Tracking·Boss 12m 폭)에 생긴다.
  시작 화면에 들어오거나 라운드가 시작되면 시점이 중앙으로 맞춰진다. 화면과 명중 판정이 같은 눈 위치를 쓴다. 세로 시야각은 55°(`cfg::kFovYDeg`).
- 명중·처치마다 점수 +1 (라운드별로 집계). 규칙 수치는 `AimLab/Game/GameConfig.h`에서 조정한다.

## 조작

| 키 | 동작 |
|---|---|
| 마우스 | 시점 (시작 화면에서도 가능) |
| 좌클릭 | 사격 (Tracking과 보스 보호막은 누르고 있기) |
| R | 시작 (시작 화면) |
| 2 / 3 | 조준점 색 / 모양 (시작·결과 화면) |
| M | 조준점 설정 표시 |
| Esc | 마우스 놓기 / 다시 잡기 |
| Q | 종료 |

## 폴더 구조

```
AimLab/
  Core/      App(GLUT 창·메인 루프, World 보관), Settings(조준점), Palette(색)
  Game/      게임 로직
    GameConfig.h   규칙 수치 (시간·표적 수·HP)
    GameTypes.h    Phase·RoundType·GameEvent(화면 효과용 사건)
    AimMath.h      조준 방향·광선-구 판정·눈 위치
    World.*        3라운드 진행·점수·판정 전부
  Input/     InputManager — GLUT 입력 콜백은 여기만 받는다. 씬은 상태를 조회만 한다
  Scene/     SceneManager + Title / Game / Result 씬
  Render/    Renderer·Shader·Mesh·Camera·GunModel·Texture — 상태를 읽어서 그리기만 한다
  Effects/   명중 파편 (보여 주기용, 판정과 무관)
  Shaders/   lit(조명), unlit(단색: 조준점·UI·바)
  Resources/ Gun.obj 등
External/   freeglut 3.6.0 · GLEW 2.2.0 · GLM 1.0.1 · stb_image 2.30 (x64)
```

한 프레임: 입력 상태 갱신 → 로직 진행(World::tick) → 사건(파편) → 씬 update(시점·사격) → 그리기.

## 원본 대비 고친 점

- 명중 판정: 등 뒤 표적도 맞던 문제, 겹치면 가장 가까운 표적이 아니라 목록 첫 표적이 맞던 문제
- 텍스트 UI가 셰이더가 켜진 채 그려지던 문제 (글자 전에 셰이더 해제)
- 원형 조준점이 화면 비율 때문에 찌그러지던 문제 (픽셀 단위로 그림)
- 결과 화면에서 파편이 공중에 멈추던 문제
- 표적 방향 전환 확률이 프레임률에 따라 달라지던 문제 (초 단위 확률로)
- 종료 시 GL 컨텍스트가 사라진 뒤 GPU 자원을 지우던 문제
- OBJ 로더: 사각형 이상 면·음수 인덱스 지원
