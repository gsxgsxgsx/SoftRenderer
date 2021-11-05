# SoftRenderer
---
- - -

### 第三方库依赖
* glad+glfw
* imgui工具
* stb图片处理库

## 运行截图
![run](demo/屏幕快照 2021-11-05 上午11.27.54.png)
## 渲染展示

### **Texture Filtering** 
#### Nearest Neighbor
![tex](demo/TexFilterNearestNeighbor.png)
#### Bilinear
![tex](demo/TexFilterBilinear.png)
#### MipMap
#### * with mipmap off
![mipmapoff](demo/MipMapOff.png)
#### * with mipmap on
![zzz](demo/MipMapOn.png)
#### * visiable mipmap 
![zzz](demo/MipMapVisiable.png)

### **Screen-Space Ambient Occlusion**
#### without blur (noise)
![ssao](demo/SSAO.png)
#### with blur
![ssao](demo/SSAOBlur.png)

- - -
### **Blinn-Phong**
#### with soft shadow PCF（Percentage Closer filtering)
![bp](demo/平行光.png)
### **Skybox**
#### with environment mapping
![sk](demo/Skybox.png)

### **Post Process**
![pp](demo/PostProcess.png)


### **PBR**
![pbr](demo/PBR.png)