# ArkSurvivalHighresMap

## 프로젝트 소개
언리얼 엔진 에디터 프로그램은 고해상도 사진을 HighResShot 명령어를 이용해서 사진을 찍을 수 있으나, 
너무 큰 사진을 요구할 때 Fatal Error가 뜨는 문제점이 있습니다.<br>
그런 불편을 해소하기 위해 프로그램을 만들게 되었습니다.<br>
아크 서바이벌 이볼브드 (ARK: Survival Evolved) 게임을 예로 들어 타일맵으로 만들었으나 아크뿐만 아니라 언리얼 엔진 기반인 게임들도 타일맵을 만들 수 있습니다.<br>
이후 캡처된 이미지는 [Leaflet](https://github.com/Leaflet/Leaflet)를 이용하여 지도처럼 볼 수 있습니다.<br>
**※ 언리얼엔진 에디터가 설치되어있어야 합니다.**

이 프로그램에 사용된 Third-Party Program 은 다음과 같습니다.
- minhook ( https://github.com/TsudaKageyu/minhook ) => 정적 라이브러리, 언리얼 엔진 함수를 후킹할 떄 쓰입니다.<br>
- libwebp ( https://github.com/webmproject/libwebp ) => 정적 라이브러리, bmp 파일을 webp 확장자로 인코딩할 때 쓰입니다.<br>
- libjpeg ( https://github.com/libjpeg-turbo/libjpeg-turbo ) => 정적 라이브러리, bmp 파일을 jpg 확장자로 인코딩할 때 쓰입니다.<br>

## 미리보기
### [Extinction](http://138.2.51.230:17875/Extinction/)
### [TheIsland](http://138.2.51.230:17875/TheIsland/)
### [Ragnarok](http://138.2.51.230:17875/Ragnarok/)
### [ScorchedEarth](http://138.2.51.230:17875/ScorchedEarth/)
### [TheCenter](http://138.2.51.230:17875/TheCenter/)
### [Fjordur](http://138.2.51.230:17875/Fjordur/)
[Fjordur_Asgard](http://138.2.51.230:17875/Fjordur_Asgard/)<br>
[Fjordur_Vanaheim](http://138.2.51.230:17875/Fjordur_Vanaheim/)

## 주요 특징
- 파이프 통신: 인젝션 된 DLL과 파이프 통신을 해 데이터를 주고받습니다.
- 확대 레벨 설정 가능: 사용자가 직접 확대 최대치를 설정할 수 있습니다.
- 타일 크기 256 ~ 16384 지원: 256 보다 큰 타일을 고르면 프로그램이 알아서 잘라줍니다.
- 다양한 이미지 포맷 지원: bmp뿐만 아니라 jpg, gif, png, webp도 지원합니다.
- 이미지 변환 멀티스레드 지원: 더 빠른 속도로 이미지 변환이 가능합니다.
- 셀프 언로드 기능: 원격 프로그램이 종료되면, DLL은 알아서 언로드 됩니다.
- 오브젝트(AActor) 위치 저장: 맵에 있는 오브젝트의 위치를 저장해 마커 등 활용이 가능합니다.
  
## 컴파일 및 실행 방법
1. Visual Studio 2022 를 다운로드 합니다. ( https://visualstudio.microsoft.com/ko/thank-you-downloading-visual-studio/?sku=Community&channel=Release&version=VS2022&source=VSLandingPage&cid=2030&passive=false )
2. Visual Studio Installer 가 뜨면 
![a](https://github.com/user-attachments/assets/d213d50a-fec8-4166-87e6-651e8fe761ce)
 와 같이 체크를 합니다.
3. 설치 (Install)를 합니다.
4. 프로젝트를 다운로드 합니다.<br>
![Animation](https://github.com/user-attachments/assets/3147cc4b-bb18-4b47-984a-1b770e501d3e)
5. 압축을 풉니다.
6. ArkEditorCapture.sln 파일을 더블클릭 해 프로젝트를 엽니다.
7. 플랫폼을 x64 그리고 구성을 Release 로 설정합니다.<br>
   ![image](https://github.com/user-attachments/assets/e7b9c243-21b0-4f86-b463-1903bd6934d9)
8. F7를 눌러 컴파일 합니다.
9. 컴파일이 완료되면, 프로젝트 폴더안의 Build 폴더에 있는 ArkEditorCapture.exe 를 실행합니다.
10. ARK DevKit을 실행합니다.
11. ArkEditorCapture.exe 의 Open 버튼을 누릅니다.<br>
![image](https://github.com/user-attachments/assets/097c288a-d346-42c5-8974-c24a50e78753)
12. 타일맵이 저장될 폴더를 선택합니다.
13. 적당한 타일 사이즈를 선택합니다. ( 크기가 커질수록 고사양 요구 )
14. StartCapture 버튼을 클릭합니다. ( 뷰포트 렌더링 사이즈가 256 x 256 아닐경우 자동으로 조정합니다. )
    
## 작동 화면
![Animation4](https://github.com/user-attachments/assets/d3f26cb1-082b-49bd-a1eb-9329839bd25b)

## ARK DevKit 설치방법
1. 에픽게임즈 사이트에 들어가 ( https://store.epicgames.com/ko/p/ark--modkit ) 받기 버튼을 누릅니다. ( 회원가입이 필요한 경우 가입합니다. )
2. ARK Modkit(UE4) 를 설치합니다.
   
## 타일맵 만들기
1. Ark Devkit에서 타일맵을 만들 맵을 불러옵니다. ( 많은 시간 소요 )
   - 공식 맵은 Game/Maps/ 폴더 안에 있고<br>모드 맵은 Game/Mods/ 폴더 안에 있습니다.
   - 여러개의 파일 중, 맵 이름에 가장 근접한 파일을 고릅니다.<br>
       Aberration -> Game/Maps/Aberration/Aberration_P<br>
       Genesis2 -> Game/Maps/Genesis2/Gen2<br>
       Valguero -> Game/Mods/Valguero/Valguero_P<br>
   ![Animation](https://github.com/user-attachments/assets/a66c5372-48b5-4164-892b-7638d4c64301)
2. 모든 레벨을 불러옵니다.<br>
   ![Animation2](https://github.com/user-attachments/assets/aa63dc5e-3f8a-431d-b407-ab0816eb8f07)
3. 우측 하단에 **셰이더 컴파일중**이 있으면 기다려야 합니다. ( 매우 오랜 시간 소요 )<br>
   ![Animation3](https://github.com/user-attachments/assets/82002671-a480-4584-b3b7-ad4f1bceb24c)
4. Alt+4 또는 라이팅 포함을 누릅니다
   ![Animation5](https://github.com/user-attachments/assets/b05246ac-afc8-40a8-8f71-451370fcb882)
5. Alt+G 또는 원근을 누르고 G 또는 게임 뷰를 눌러 네모난 선이 안 보이도록 합니다. 그리고 Alt+J 또는 상단을 누릅니다. 
   ![Animation6](https://github.com/user-attachments/assets/ec763ba5-0bcd-4e56-9ac7-06d6ae0eea05)
6. 맵이 너무 밝으면 **DirectionalLight**를 검색해 Intensity 값을 조절합니다. ( ※ DirectionalLight가 여러개인경우, 옆에 눈 아이콘을 클릭해 어두워 지는지 확인 후 Intensity 값을 조절합니다. )
   ![Animation7](https://github.com/user-attachments/assets/cecc2af1-2a48-4d83-a412-1fa65c58040a)
7. 씬 아웃라이너에서 아무거나 선택한 다음 오른쪽 클릭 후 모두 선택을 누릅니다.
   ![Animation8](https://github.com/user-attachments/assets/d7962cb6-20e7-4718-9dde-9c89a30cedad)
8. 디테일에서 **Force Infinite Draw Distance** 를 체크합니다.
   ![Animation9](https://github.com/user-attachments/assets/cec65d45-56b1-4248-bc51-bc43945381f6)
9. ArkEditorCapture.exe를 실행합니다
10. ArkEditorCapture.exe의 Open 버튼을 누릅니다.
11. ArkEditorCapture.exe의 ZeroXY와 ZOriginal 버튼을 눌러 맵이 뷰포트에 들어오는지 확인합니다.
    ![Animation10](https://github.com/user-attachments/assets/4ef76d1e-bffb-4803-8779-a4fe1ccf8f07)
12. 맵이 한번에 안보인다면, 스크롤을 조정하여 한번에 보이게끔 설정합니다
    ![Animation11](https://github.com/user-attachments/assets/ca1c1238-11ed-40bc-beba-1f719ac16815)

## 타일맵 제작 팁 ( 아크 서바이벌 이볼브드 기준 )
### 에픽게임즈 런처 없이 언리얼 실행 방법
윈도우키 + R 을 입력해 실행창이 뜨면 아래 문장을 입력후 엔터
- UE4 : %ProgramFiles%\Epic Games\ARKEditor\Engine\Binaries\Win64\UE4Editor.exe" ShooterGame/ShooterGame.uproject
- UE5 : %ProgramFiles%\Epic Games\ARKDevkit\Engine\Binaries\Win64\ShooterGameEditor.exe"  ShooterGame/ShooterGame.uproject

### 오브젝트에 사각형 선이 보일 시
- Alt+G(원근) -> G(게임뷰) 체크 해제 -> Alt+J(상단) 

### 익스팅션 한정 레벨 해제옵션 ( 좌측 상단에 있는 레벨 창 )
- Ext_DesertVista
- Ext_Proxymeshes
- Ext_LakesAndRivers

### 맵 밝기가 너무 밝은 경우
- DirectionalLight 검색 후 Intensity 값 조절
- SkyLight 검색 후 끄기

### 아일랜드 맵 화산 옆 반짝거리는 오브젝트 숨기기
- 씬 아웃라이너에 BP_VolcanicLightning 검색 후 숨기기 ( 단축키 H )

### 탐험 노트( Explorer Note ) 또는 일부 오브젝트가 안 보일시
- 모든 리소스 선택 후 (Ctrl+A) 다음 디테일 탭에서 Force Infinite Draw Distance 검색 후 체크해야 일부 오브젝트 보임

### 맵 그림자가 체크무늬처럼 보기 싫을 때 해결 방법
- 씬 아웃라이너에 Landscape 검색 -> Lighting -> Allow Height Field Shadow 검색 후 체크 해제

### 물 색깔이 마음에 안 들 때
- 물 오브젝트 클릭 -> Top Material 변경( 미리보기를 더블클릭해 색깔 사용자 지정할 수도 있음 )

### 해당 프로그램으로 만들어진 타일맵 사진에 찍힌 실제 오브젝트 위치를 Leaflet 마커를 이용해 표시하는 법
- [Extinction](http://138.2.51.230:17875/Extinction/) 접속 후 오른쪽 마우스 클릭해서 소스코드 보기 후 참고
