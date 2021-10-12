# TextureConverter for StarCitizen
## Abstract
Unsplit texture asset files which split by mipmap levels. ~~or Split texture files for starcitizen texture assets.(WIP)~~
## 실행
Simply drag&drop for unsplit/split texture file(s) or input path of file as parameter. It also supports directory path. 
~~프로그램은 각 파일을 처리한 결과 파일을 하위 디렉토리 ```out```에 저장합니다.(기본값 기준)  ~~
### 옵션
- ```-w```: 하위 디렉토리에 저장하지 않고 현재 디렉토리의 파일에 덮어씁니다.
- ```-v```: 실행 과정을 출력합니다.
- ```-r```: 디렉토리의 경우, 하위 디렉토리의 파일 또한 변환합니다.
- ```-i```: 변환하는 파일의 정보를 출력합니다.
- ~~```-c```: png파일을 밉맵분리 dds파일로 변환합니다.~~ (예정)


## 개요
스타시티즌 에셋 중 밉맵 파일이 분리된 dds 파일을 합치거나 dds 파일의 밉맵을 분리 생성합니다.  
## 실행
실행 파일에 분리/통합할 dds 파일을 드래그&드롭 하거나 경로를 인자로 넣어 주세요. 디렉토리도 지원합니다.  
프로그램은 각 파일을 처리한 결과 파일을 하위 디렉토리 ```out```에 저장합니다.(기본값 기준)  
### 옵션
- ```-w```: 하위 디렉토리에 저장하지 않고 현재 디렉토리의 파일에 덮어씁니다.
- ```-v```: 실행 과정을 출력합니다.
- ```-r```: 디렉토리의 경우, 하위 디렉토리의 파일 또한 변환합니다.
- ```-i```: 변환하는 파일의 정보를 출력합니다.
- ~~```-c```: png파일을 밉맵분리 dds파일로 변환합니다.~~ (예정)
