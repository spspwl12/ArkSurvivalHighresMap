# ArkSurvivalHighresMap

## 프로젝트 소개
아크 서바이벌 이볼브드 (ARK: Survival Evolved) 게임의 맵 지형을 고해상도로 캡처해 주는 프로그램입니다.<br>
이후 캡처된 이미지는 Leaflet를 이용하여 지도처럼 볼 수 있습니다.<br>
**※ ARK DevKit 이 설치되어있어야 합니다.**

이 프로그램에 사용된 Third-Party Program 은 다음과 같습니다.
- minhook ( https://github.com/TsudaKageyu/minhook ) => 정적 라이브러리, 언리얼 엔진 함수를 후킹할 떄 쓰입니다.<br>
- libwebp ( https://github.com/webmproject/libwebp ) => 정적 라이브러리, bmp 파일을 webp 확장자로 인코딩할 때 쓰입니다.<br>
- libjpeg ( https://github.com/libjpeg-turbo/libjpeg-turbo ) => 정적 라이브러리, bmp 파일을 jpg 확장자로 인코딩할 때 쓰입니다.<br>

## 미리보기
### <a href="http://138.2.51.230:17875/Extinction/" target="_blank">Extinction</a>
### <a href="http://138.2.51.230:17875/TheIsland/" target="_blank">TheIsland</a>
### <a href="http://138.2.51.230:17875/Ragnarok/" target="_blank">Ragnarok</a>
### <a href="http://138.2.51.230:17875/ScorchedEarth/" target="_blank">ScorchedEarth</a>
### <a href="http://138.2.51.230:17875/Fjordur/" target="_blank">Fjordur</a>
<a href="http://138.2.51.230:17875/Fjordur_Asgard/" target="_blank">Fjordur_Asgard</a><br>
<a href="http://138.2.51.230:17875/Fjordur_Vanaheim/" target="_blank">Fjordur_Vanaheim</a><br>

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
4. 




