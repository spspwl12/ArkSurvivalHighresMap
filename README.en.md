# ARK Survival High Resolution Map Project

## Project Introduction  
Unreal Engine Editor allows users to take high-resolution screenshots using the `HighResShot` command. However, requesting an extremely large image can cause a **Fatal Error**.  <br>
This program was created to solve that problem.  <br>
This tool is specialized for the game **ARK: Survival Evolved**, but in addition to ARK, other Unreal Engine-based games can also create tile maps.  <br>
The captured images can be viewed like a map using [Leaflet](https://github.com/Leaflet/Leaflet).  <br>
Since this project was originally inspired by the **ARK: Survival Evolved** game, it was named **ArkSurvivalHighresMap**.  <br>

**⚠️ Unreal Engine Editor must be installed to use this tool.**  

## Third-Party Programs Used in This Project  
The following third-party programs are utilized in this project:  
- [**minhook**](https://github.com/TsudaKageyu/minhook) → A static library used for hooking Unreal Engine functions.  
- [**libwebp**](https://github.com/webmproject/libwebp) → A static library used for encoding BMP files into WebP format.  
- [**libjpeg**](https://github.com/libjpeg-turbo/libjpeg-turbo) → A static library used for encoding BMP files into JPG format.  

## Example
### [Extinction](http://138.2.51.230:17875/Extinction/)
### [TheIsland](http://138.2.51.230:17875/TheIsland/)
### [Ragnarok](http://138.2.51.230:17875/Ragnarok/)
### [ScorchedEarth](http://138.2.51.230:17875/ScorchedEarth/)
### [TheCenter](http://138.2.51.230:17875/TheCenter/)
### [Genesis2](http://138.2.51.230:17875/Genesis2/)
### [CrystalIsles](http://138.2.51.230:17875/CrystalIsles/)
### [Fjordur](http://138.2.51.230:17875/Fjordur/)
[Fjordur_Asgard](http://138.2.51.230:17875/Fjordur_Asgard/)<br>
[Fjordur_Vanaheim](http://138.2.51.230:17875/Fjordur_Vanaheim/)

## Features
- **Sends Data Using Pipes**: The injected DLL communicates with the main program.  
- **Adjustable Zoom Level**: Users can set the maximum zoom level.  
- **Supports Tile Sizes (256 ~ 16,384)**: Large tiles are automatically split.  
- **Supports Various Image Formats**: Works with BMP, JPG, GIF, PNG, and WebP.  
- **Fast Image Conversion**: Uses multiple threads for quicker processing.  
- **Preview Before Saving**: Users can see the map and adjust coordinates.  
- **Auto-Unload Feature**: The DLL removes itself when the program closes.  
- **Saves Object Coordinate**: Saves the object's coordinates for use as markers.  
- **Supports Unreal Engine 5**: Capture is available in both Unreal Engine 4 and 5.  

## How to Compile and Run
1. [Download](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&channel=Release&version=VS2022) **Visual Studio 2022**. <br>
2. When the **Visual Studio Installer** appears, select the options as shown below:<br>
![a](https://github.com/user-attachments/assets/d213d50a-fec8-4166-87e6-651e8fe761ce)  
3. Click **Install** to begin the installation process.<br>
4. Download the **project**.<br>
![Animation18](https://github.com/user-attachments/assets/59cc19a3-ea36-4bfc-9ec7-af46f92a4509)
5. Extract the downloaded **ZIP file**.<br>
6. Double-click **ArkEditorCapture.sln** to open the project.<br>
7. Set the **platform** to **x64** and the **configuration** to **Release**.<br>
![Animation19](https://github.com/user-attachments/assets/ca213678-07eb-4b59-ade9-6be196a705e3)
8. Press **F7** to compile.<br>
9. When the compilation is finished, open the **Build** folder inside the project. Then, run **ArkEditorCapture.exe**.  <br>
10. Run **ARK DevKit**.<br>
11. Click the **Open** button in **ArkEditorCapture.exe**.<br>
![image](https://github.com/user-attachments/assets/82fdf965-5b21-4f7e-8aa7-2fdfb63e1df2)
12. Select the **folder** where the tile map will be saved. <br>
![image](https://github.com/user-attachments/assets/d392b565-2b75-4191-9ee9-46f44ed7c034)
13. Select an appropriate tile size (Bigger sizes need a more powerful computer.) <br>
![image](https://github.com/user-attachments/assets/ff0aa33a-b818-455f-90fc-4e4a3d408cdd)
14. Click the **Start Capture** button. (If the viewport rendering size is not 256 x 256, it will be automatically adjusted.)<br>
![image](https://github.com/user-attachments/assets/dbf1df2b-6860-4c71-8edb-3ffc52d5a5b0)

## Demo
![Animation11](https://github.com/user-attachments/assets/437f6d5e-874a-45fd-9f05-1b8b112f168c)
![Animation12](https://github.com/user-attachments/assets/ee2c5b1c-f564-49b8-aa5a-c2ffbd9e681d)
![Animation13](https://github.com/user-attachments/assets/38ca4a79-5744-4f0c-a948-e69f3cb09fa8)
![Animation14](https://github.com/user-attachments/assets/bf068637-872b-4e86-a2c0-3d58391c3f79)

## How to Install ARK DevKit
1. Go to the [Epic Games](https://store.epicgames.com/ko/p/ark--modkit) website and click the **Download** button. (If a sign-up is required, create an account.)  
2. After the Epic Games Launcher is installed, search for **ARK Modkit(UE4)** and download it.  

## Creating a Tile Map
1. Load the **map** where you want to create the **tile map** in **ARK DevKit**.<br>
   (**Warning:** If **ArkEditorCapture.exe** is open, opening a new map may cause a **crash error**.)
   - Official maps are located in the **Game/Maps/** folder.<br>
   - Mod maps are stored in the **Game/Mods/** folder.<br>
   - Among multiple files, choose the one **closest to the map name**.<br>
       Aberration -> `Game/Maps/Aberration/Aberration_P`<br>
       Genesis2 -> `Game/Maps/Genesis2/Gen2`<br>
       Valguero -> `Game/Mods/Valguero/Valguero_P`<br>
   ![Animation](https://github.com/user-attachments/assets/a66c5372-48b5-4164-892b-7638d4c64301)
2. Load **all levels**.<br>
   ![Animation2](https://github.com/user-attachments/assets/aa63dc5e-3f8a-431d-b407-ab0816eb8f07)
3. If **Compiling Shaders** appears in the **bottom right corner**, you need to wait. (**This process takes a very long time.**) <br>
   ![image](https://github.com/user-attachments/assets/5ebaec27-e912-4702-b9b9-8fbfe73988be)
4. Press **Alt+4** or click **Lit**.<br>
   ![Animation5](https://github.com/user-attachments/assets/b05246ac-afc8-40a8-8f71-451370fcb882)
5. Press **Alt+G** or click **Perspective**, then press **G** or select **Game View** to hide the square lines.<br>
   Then, press **Alt+J** or select **Top**. <br>
   ![Animation6](https://github.com/user-attachments/assets/ec763ba5-0bcd-4e56-9ac7-06d6ae0eea05)
6. If the map is too bright, search for **DirectionalLight** and adjust the **Intensity** value.<br>
   (**Note:** If there are multiple **DirectionalLight**, click the **eye icon** next to each one to check which one dims the map before adjusting the **Intensity** value.)<br>
   ![Animation7](https://github.com/user-attachments/assets/cecc2af1-2a48-4d83-a412-1fa65c58040a)
7. In the **Scene Outliner**, select any item, then **right-click** and choose **Select All**.<br>
   ![Animation8](https://github.com/user-attachments/assets/d7962cb6-20e7-4718-9dde-9c89a30cedad)
8. In **Details**, check **Force Infinite Draw Distance**.<br>
   ![Animation9](https://github.com/user-attachments/assets/cec65d45-56b1-4248-bc51-bc43945381f6)
9. Run **ArkEditorCapture.exe**. <br>
10. Click the **Open** button in **ArkEditorCapture.exe**. <br>
   ![image](https://github.com/user-attachments/assets/c994efe4-41ca-4451-8877-637df4c3560e)
11. Click the **Set Zero XY** and **Set Original Z** buttons in **ArkEditorCapture.exe**. <br>
    ![image](https://github.com/user-attachments/assets/0623b937-1770-4880-a445-71aff75bd2f1)
12. Click the **Set Current Size** button in **ArkEditorCapture.exe**. <br>
    ![Animation15](https://github.com/user-attachments/assets/30e5fdf2-2c99-4d94-8ecd-ac03dfa71c6b)
13. Click the **Preview** button to center the **map** on the screen.  <br>
    If the screen is too small and the full map is not visible, adjust the settings as follows.  <br>
    ![Animation16](https://github.com/user-attachments/assets/e475e5d7-3615-4c4d-80b3-85bfa1f2107d)
14. Once the setup is complete, click the **X button** or click the **Preview** screen to close it. <br>
15. Click **Start Capture** to capture the map using the set coordinates.  <br>
    ![Animation18](https://github.com/user-attachments/assets/80ee7f38-d059-424f-bad4-556973e7c97a)
16. Check the results. <br>
    ![{0F54A96E-80F8-4131-B77B-93F14B9DA4AB}](https://github.com/user-attachments/assets/f8699419-b63e-4fcb-ae21-8ae851e8245f)
17. Open the included **CheckMap.html** file on a **web server** to view the map. <br>

## Tile Map Creation Tips (ARK DevKit Guide)
### How to Run Unreal Without Epic Games Launcher  
Press **Windows Key + R** to open the **Run** dialog, then enter the following command and press **Enter**: <br>
- **UE4:**  
  `"%ProgramFiles%\Epic Games\ARKEditor\Engine\Binaries\Win64\UE4Editor.exe" ShooterGame/ShooterGame.uproject`  
- **UE5:**  
  `"%ProgramFiles%\Epic Games\ARKDevkit\Engine\Binaries\Win64\ShooterGameEditor.exe"  ShooterGame/ShooterGame.uproject`

### When Rectangular Lines Appear on an Object  
- Press **Alt+G** (**Perspective**) → Uncheck **Game View** or Press G → Press **Alt+J** (**Top**)  

### Hide the Sparkling Object Next to the Volcano (Island Map Only)  
- In the **Scene Outliner**, search for **BP_VolcanicLightning** and hide it (**Shortcut: H**)  

### When Glass Attached to Bridges Appears Black (Extinction Map Only)  
- In the top-left level window, search for **Ext_Proxymeshes**, then click the **eye icon** to hide it.  

### When the Overall Color is Too Purple (Genesis 2 Map Only)  
- **Show** in **Viewport** → Uncheck **Atmosphere**  

### If the map is too bright
- Search for **DirectionalLight** and adjust the **Intensity** value.  
- Search for **SkyLight** and hide it (**Shortcut: H**) .
  
### How to Fix Not Visible Explorer Notes or Certain Objects
- Select all resources in the Scene Outliner (**Shortcut: Ctrl+A**), then go to the Details tab, search for **Force Infinite Draw Distance**, and check it.

### How to Fix Checkerboard Shadows on the Map
- Search for **Landscape** in the Scene Outliner, then go to the Details tab and search for **Allow Height Field Shadow**, then uncheck it.

### How to Fix Water Visibility Issues When Zooming In/Out on the Map
- Search for **Landscape** in the Scene Outliner, then go to the Details tab and navigate to LOD → LODFalloff → Set **Square Root**.  

### How to Fix Low Detail on Rocks and Other Objects in the Map 
- Search for **ProxyMeshActor** in the Scene Outliner, then select all matching objects (**Shortcut: Ctrl+A**). <br>
  Go to the Details tab, search for **Min Draw Distance**, and change the value to **99999999**.  

### How to Fix Cloud Shadows Covering the Entire Map
- Show in Viewport → Uncheck **"Have True Sky Active"** option.
  
### How to Fix Unsatisfactory Water Color
- Click on the water object → Change Top Material. 

### Why Trees and Grass Are Visible in Perspective View but Not in Top View
![image](https://github.com/user-attachments/assets/aefe28fb-92b1-4c51-819d-34e312416fa5)
- **InstancedFoliageActor** objects are not visible from the **top view**, <br>
  but the relevant option (**Foliage Types**) does not exist in **ARK Devkit**, so this issue cannot be resolved.  
- Do not open it with **ARK DevKit**; opening it with **Unreal Engine 4 editor** may resolve the issue.

### How to fix: When capturing at a tile size of 256, objects stay in place, but at 1024 or higher, they appear in the wrong locations.
- Capture the zoom levels that have issues (0 to 4) using tile sizes of 256 or 512.

### How to display the actual object positions captured in a tile map image created by the program using Leaflet markers.
- Go to [Extinction](http://138.2.51.230:17875/Extinction/) and right-click to select "View Page Source" to reference the source code.  
