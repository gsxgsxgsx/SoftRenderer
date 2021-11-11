# SoftRenderer

## 第三方库依赖
* glad+glfw
* imgui工具
* stb图片处理库

## 运行截图

<table>
    <tr>
        <td><figure >
            <figcaption></figcaption>
            <img width="600"  src="demo/running.png"></figure>
            <figure>
        </td>
    </tr>
</table>

## 渲染展示

## **Texture**
### **Texture Filtering** 

<table>
    <tr>
        <td><figure >
            <figcaption> Nearest Neighbor</figcaption>
            <img width="400"   src="demo/TexFilterNearestNeighbor.png"></figure>
            <figure>
        </td>
        <td><figure >
            <figcaption >Bilinear</figcaption>
            <img width="400"  src="demo/TexFilterBilinear.png"><figure>
        </td>
    </tr>
</table>

### **MipMap**

<table>
    <tr>
        <td><figure >
            <figcaption> with mipmap off</figcaption>
            <img width="400"   src="demo/MipMapOff.png"></figure>
            <figure>
        </td>
        <td><figure >
            <figcaption >with mipmap on</figcaption>
            <img width="400"  src="demo/MipMapOn.png"><figure>
        </td>
    </tr>
    <tr>
        <td><figure >
            <figcaption >* visualize mipmap </figcaption>
            <img width="400"  src="demo/MipMapVisiable.png"><figure>
        </td>
    </tr>
</table>

### **Perspective correct**

<table>
    <tr>
        <td><figure >
            <figcaption> without Perspective correction</figcaption>
            <img width="400"   src="demo/perCorrectOff.png"></figure>
            <figure>
        </td>
        <td><figure >
            <figcaption >with Perspective correction</figcaption>
            <img width="400"  src="demo/perCorrectOn.png"><figure>
        </td>
    </tr>
</table>


### **Skybox**

<table>
    <tr>
        <td><figure >
            <figcaption>with simple environment mapping</figcaption>
            <img width="600"   src="demo/Skybox.png"></figure>
            <figure>
        </td>
    </tr>
</table>

#### generate 6 skybox textures from Equi-rectangular Projection
#### origin:
<img width="150"  src="resources/Mesh.jpg">

<div></div>
<img width="150"  src="resources/test-white.png">
<img width="150"  src="demo/test+y.png">
<div></div>
<img width="150"  src="demo/test-x.png">
<img width="150"  src="demo/test+z.png">
<img width="150" src="demo/test+x.png">
<img width="150"  src="demo/test-z.png">
<div></div>
<img width="150"  src="resources/test-white.png">
<img width="150"  src="demo/test-y.png">

- - -

## **Screen-Space Ambient Occlusion**

<table>
    <tr>
        <td><figure >
            <figcaption> without blur (noise)</figcaption>
            <img width="400"   src="demo/SSAO.png"></figure>
            <figure>
        </td>
        <td><figure >
            <figcaption >with blur</figcaption>
            <img width="400"  src="demo/SSAOBlur.png"><figure>
        </td>
    </tr>
</table>

- - -

## **Blinn-Phong**
#### with soft shadow PCF（Percentage Closer filtering)
![bp](demo/平行光.png)


## **Post Process**
![pp](demo/PostProcess.png)


## **Physically Based Rendering**


<img width="300"  src="resources/hdr/snow_machine/test8_Bg.jpg">

<div></div>
<img width="150"  src="resources/test-white.png">
<img width="150"  src="demo/-y.png">
<div></div>
<img width="150"  src="demo/-x.png">
<img width="150"  src="demo/+z.png">
<img width="150" src="demo/+x.png">
<img width="150"  src="demo/-z.png">
<div></div>
<img width="150"  src="resources/test-white.png">
<img width="150"  src="demo/+y.png">

![pbr](demo/PBR.png)