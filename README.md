# Graphics

Welcome to **Graphics** which implements some of the real-time rendering and physics-based rendering papers. Note that the renderer used is my [Cooler](https://github.com/freehyan/Cooler) renderer.

## Anti-aliasing

Deferred shading does not support anti-aliasing, because the scene geometry information is separated. And these information can not support hadware anti-aliasing, it can only be resolved by some post-processing methods.

* Deferred Shading MSAA 

<img src="http://7xwp8l.com1.z0.glb.clouddn.com/MSAA.png" width="500" height="300" alt="MSAA" />

[Reference]: [Deferred Shading MSAA](http://nvidiagameworks.github.io/GraphicsSamples/DeferredShadingMSAASample.htm), NVIDIA Research.

 
* Deferred Shading MLAA

<img src="http://7xwp8l.com1.z0.glb.clouddn.com/MLAA.png" width="500" height="300" alt="MLAA" />

[Reference]: [Practical Morphological Antialiasing](https://books.google.com/books?hl=en&lr=&id=tixuGR3iDmUC&oi=fnd&pg=PA95&dq=Practical+Morphological+Antialiasing&ots=DclEj303q_&sig=rOr_fJ4RmuP09Li7UhG81Kcq_wU#v=onepage&q=Practical%20Morphological%20Antialiasing&f=false), Jorge Jimenez, 2011.


## Order-Independent Transparency

OIT traditional approach is to do a depth sort of transparent objects, and then render them from back to front's order. This algorithm uses the UAV new feature to build the linked list, efficiently and correctly render transparent object effects.

<img src="http://7xwp8l.com1.z0.glb.clouddn.com/OIT.png" width="500" height="300" alt="OIT" />

[Reference]: [Order-Independent Transparency using Per-Pixel Linked List](https://books.google.com/books?hl=en&lr=&id=zfPRBQAAQBAJ&oi=fnd&pg=PA409&dq=Order-Independent+Transparency+using+Per-Pixel+Linked+List&ots=WGkV_9NQto&sig=GlU-4NkQ1E6vVkpPsGYwwHx4t94#v=onepage&q=Order-Independent%20Transparency%20using%20Per-Pixel%20Linked%20List&f=false), Nicolas Thibieroz, 2011


## Real-Time Lighting via Light Linked List

Deferred lighting is a popular technique, but it doesn't deal with transparency geometry and particle effects. The LLL technique stores dynamic lights in a linked list accessible per-pixels for all the elements in a game scene. The LLL not only speeds up real-time dynamic lighting it also makes it possible for transparent effect and particles to receive both lighting and shadowing.

<img src="http://7xwp8l.com1.z0.glb.clouddn.com/LLL.png" width="500" height="300" alt="LLL" />

[Reference]: [Real-Time Lighting via Light Linked List](https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&uact=8&ved=0ahUKEwj_hOeTrPLUAhXHj5QKHRxdBxoQFggoMAA&url=http%3A%2F%2Fadvances.realtimerendering.com%2Fs2014%2Finsomniac%2FLight%2520Linked%2520List.pptx&usg=AFQjCNHYENomC1xMKPF80DDWiCaQQuZVUA), Abdul Bezrati (Insomniac Games), 2014

## Screen Space Fluid Render

Screen Space Fluid Render is not based on polygonization. SSFR only handles surface particles of the visible range, so greatly improving the real-time performance. And it smoothes the surface to prevent the fluid from looking "blobby" or jelly-like.

<img src="http://7xwp8l.com1.z0.glb.clouddn.com/Fluid.png" width="500" height="300" alt="FLUID" />

[Reference]: [Screen Space Fluid Render](http://developer.download.nvidia.com/presentations/2010/gdc/Direct3D_Effects.pdf), NVIDIA, 2010.

## Exponential Shadow Map

The major problem of shadow map is aliasing. Filtering the shadow map alleviates aliasing, but unfortunately, native hardware-accelerated filtering cannot be applied, as the shadow test has to take place beforehand. Exponential shadow map can pre-filter shadow map to render high-quality shadows.

<img src="http://7xwp8l.com1.z0.glb.clouddn.com/ESM.png" width="500" height="300" alt="ESM" />

[Reference]: [Exponential Shadowm Maps](http://jankautz.com/publications/esm_gi08.pdf), Thomas Anne, 2008.

---

##### The following paper source code I do not want to open source temporarily, because they are also helpful to the graduation project in next year. I am sorry this behavior, thanks.

## Deferred Attribute Interpolation Shading

DAIS proposed a method to dynamically reduce the memory consumption of deferred shading, through the visibility buffer and triangle buffer instead of the original G-Buffer, and then in the shading phase through the triangular partial derivative for attribute interpolation for lighting calculations.

<img src="http://7xwp8l.com1.z0.glb.clouddn.com/DAIS.png" width="500" height="300" alt="DAIS" />

[Reference]: [Deferred Attribute Interpolation Shading](https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&uact=8&ved=0ahUKEwjU1LnZjvrUAhUEurwKHXM1BcIQFggoMAA&url=http%3A%2F%2Fcg.ivd.kit.edu%2Fpublications%2F2015%2Fdais%2FDAIS.pdf&usg=AFQjCNFVahpeS6ZBXtqPPabGsPTHxapU6g), GPU Pro 7, 2016.


## Volumetric Fog and Lighting

Volumetric fog is an extension of existing ray-marching algorithms. It uses the advantages of three-dimensional texture, calculate the in-scattering of light and volume fog effect.

<img src="http://7xwp8l.com1.z0.glb.clouddn.com/Volumetric%20Fog.png" width="500" height="300" alt="FOG" />

[Reference]: [Volumetric Fog and Lighting](https://books.google.com/books?hl=en&lr=&id=30ZOCgAAQBAJ&oi=fnd&pg=PA217&dq=Volumetric+Fog+and+Lighting&ots=2YlygWJDIN&sig=36V0ZSUOPcUglBFCKKxa_OOD5GQ#v=onepage&q=Volumetric%20Fog%20and%20Lighting&f=false), Bartlomiej Wro´ nski, 2015.

## Stochastic Screen Space Reflection

The traditional screen space reflection has no contact hardening, Specular elongation, pixel-pixel roughness and other real effects' characteristics. SSSR introduces the importance sampling and halton sequence to ensure the reflection of the realistic effect.

<img src="http://7xwp8l.com1.z0.glb.clouddn.com/SSR.png" width="500" height="300" alt="SSR" />

[Reference]: [Stochastic Screen Space Reflection](http://advances.realtimerendering.com/s2015/Stochastic%20Screen-Space%20Reflections.pptx), Tomasz Stachowiak, 2015.


## License

**Graphics** is available as open source under the terms of the [MIT License](http://opensource.org/licenses/MIT).
