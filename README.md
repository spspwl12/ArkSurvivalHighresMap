# ArkSurvivalHighresMap

## 프로젝트 소개
아크 서바이벌 이볼브드 (ARK: Survival Evolved) 게임의 맵 지형을 고해상도로 캡처해 주는 프로그램입니다.<br>
이후 캡처된 이미지는 Leaflet를 이용하여 지도처럼 볼 수 있습니다.<br><br>

이 프로그램에 사용된 Third-Party Program 은 다음과 같습니다.
- minhook ( https://github.com/TsudaKageyu/minhook ) => 정적 라이브러리, 언리얼 엔진 함수를 후킹할 떄 쓰입니다.<br>
  
## 주요 특징
- 파이프 통신: 인젝션 된 DLL과 파이프 통신을 해 데이터를 주고받습니다.
- 확대 레벨 설정 가능: 사용자가 직접 확대 최대치를 설정할 수 있습니다.
- 다양한 이미지 포맷 지원: bmp뿐만 아니라 jpg, gif, png도 지원합니다.
- 타일 크기 256 ~ 16384 지원: 256 보다 큰 타일을 고르면 프로그램이 알아서 잘라줍니다.
- 셀프 언로드 기능: 원격 조종 프로그램이 종료되면, DLL은 알아서 언로드 됩니다.
  
## 컴파일 및 실행 방법
1. Visual Studio 2022 를 다운로드 합니다. ( https://visualstudio.microsoft.com/ko/thank-you-downloading-visual-studio/?sku=Community&channel=Release&version=VS2022&source=VSLandingPage&cid=2030&passive=false )
2. Visual Studio Installer 가 뜨면 
![a](https://github.com/user-attachments/assets/d213d50a-fec8-4166-87e6-651e8fe761ce)
 와 같이 체크를 합니다.
3. 설치 (Install)를 합니다.
4. 프로젝트를 다운로드 합니다.<br>
![aa](https://github.com/user-attachments/assets/edf2fff8-8bfc-4e0f-9c3a-28b12de66713)
5. 압축을 풉니다.
6. sokoban.sln 파일을 더블클릭 해 프로젝트를 엽니다.
7. 플랫폼을 x86 그리고 구성을 Release 로 설정합니다.<br>
   ![image](https://github.com/user-attachments/assets/e70b6756-e9bb-4013-9f97-88bf58752792)
8. F7를 눌러 컴파일 합니다.
9. 컴파일이 완료되면, 프로젝트 폴더안의 Build 폴더에 있는 sokoban.exe 를 실행합니다.
    
## 작동 화면


