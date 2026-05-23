<h1 align="center"><a href="#">&lt;kit.h&gt;</a></h1>
<p align="center">
Portable game toolkit in C.
<br/>
<img src="https://i.imgur.com/WMNO8J1.png"              /><!--pbr--><br/>
<img src="https://i.imgur.com/2HZyXba.png" width="204px"/><!--triangle-->
<img src="https://i.imgur.com/zZaApb2.png" width="204px"/><!--debugdraw-->
<img src="https://i.imgur.com/A1e45dT.png" width="204px"/><!--bvh-->
<img src="https://i.imgur.com/NLeR66w.png" width="204px"/><!--ui-->
<img src="https://i.imgur.com/Wpyd4gY.png" width="204px"/><!--sprite-->
<img src="https://i.imgur.com/t1dxy6n.png" width="204px"/><!--fx-->
<img src="https://i.imgur.com/gxiyrzX.png" width="204px"/><!--input-->
</p>

# About

- [x] Portable game toolkit to develop both 2D/3D games+editors with it.
- [x] Supersedes previous [v1](https://github.com/r-lyeh/v1), [v2](https://github.com/r-lyeh/v2) and [v3](https://github.com/r-lyeh/v3) engine attempts.
- [x] Simple: very small engine foundations that combine well together.
- [x] Community fueled: create, publish and share decentralized extensions.
- [x] Everywhere: For Web, Desktops, Consoles and Mobiles.
- [x] Standalone: All batteries included. The major included dependencies:
  - [x] [SDL3](https://wiki.libsdl.org/SDL3/) [+ SDL3_Net](https://wiki.libsdl.org/SDL3_net/) as Platform/Network layers.
  - [x] [SDL3_GPU](https://wiki.libsdl.org/SDL3/CategoryGPU) [+ SDL3_Renderer](https://wiki.libsdl.org/SDL3/CategoryRender) as Rendering layers.
  - [x] [SDL3_Audio](https://wiki.libsdl.org/SDL3/CategoryAudio) [+ MojoAL](https://github.com/icculus/mojoAL/) as Audio3D layers.
  - [x] [SDL3_Input](https://wiki.libsdl.org/SDL3/APIByCategory#input-events) [+ SDL3_Gesture](https://github.com/libsdl-org/SDL_gesture) as Input layers.
  - [x] [SDL3_TTF](https://wiki.libsdl.org/SDL3_ttf) [+ DearImgui](https://github.com/ocornut/imgui) as UI/Windowing layers.
  - [x] [Lua](https://lua.org) [+ derivatives](https://github.com/hengestone/lua-languages) as Scripting layers.

<!--
  - [x] [WebGPU](https://webgpu.org) as the Rendering Layer.
  - [x] [WGSL](https://www.w3.org/TR/WGSL/) as the Shading language. Optional SPIRV and GLSL shaders.-->
<!-- minimalist: everything should be easy to grasp when opening any existing source file. when possible, i like my public apis to be a line or maybe two. similarly, their implementations would be ideally contained in a single page too. -->
<!-- gameobjs could use anything from the lua,luajit,teal,hurdy ecosystem -->
<!-- difference between art/ and res/ + embed folders -->
<!-- use git clone https://github.com/anyuser/kit.anything to install any module that will be compiled automatically. use `cl ext` for more help on extensions -->
<!-- similarly, you can just create your own extensions for others to use. for that, create a `kit.something` repository on GH with a `kit1` project tag in it. the repo will be automatically visible to the community -->
<!-- auto-generated docs: comments starting with `///` will be added to the documentation -->
<!-- bool, float/int24, double/int53, strings -->
<!-- REFLECT() -->
<!-- final optional args when using a string
     alert("hello");
     alert("hello", "world");

     also,
     void *obj = make(vec3, 1); save(&obj); // implicit type
     vs
     vec3 p = {1,2,3}; save(&p, "vec3"); // explicit types
-->
<!-- ## kit object notation (.kon)

it's basically a ini file transformation, which then resembles a breed between csv and lisp.
the transformation process is as follows, considering we use ini [sections] to specify the type we serialize:

```ini
[vec2]
x=10
y=20
```

then we basically replace `[ to (`, `]\n to :`, `\n to ,` and `\n\0 to )`, which leads to:

```lisp
(vec2:x=10,y=20)
```

that's it. this simple format brings some nice properties implied:
- datas are typed
- no multi-line handling, which means format is simpler to parse
- nested types allowed `(person:name="john",location=(vec2:x=10,y=20),age=42)`
- for each `key=` found, its value will be either a `number`, a `"string"` or a nested `(struct)`.
- @todo: since we use floating numbers, Inf,-Inf,NaN keywords are parsed and evaluated as numbers too

-->

# Features and roadmap
- [x] High level C.
  - [x] DS: Typed containers for bitpools, vectors, hashmaps.
  - [x] Strings: Quick temporaries and interned strings.
  - [x] Reflection
  - [x] Serialization
  - [x] Modules
  - [ ] Scenetree/Game objects
- [x] 3D Audio: OGG, MP3, FLAC, WAV/ADPCM. <!-- @todo: qoa, streaming -->
- [x] 3D DebugDraw: via SDL_Renderer.
- [x] 3D GPU Sprites: via SDL_Renderer. Soon: ASE, TMX.
- [x] 3D GPU PBR rendering: via SDL_GPU. GLTF, GLB.
- [x] 3D GPU Post-effects.
- [x] Image: WEBP, QOI, PNG, JPG, TGA, BMP, PSD.
- [x] Text: MO.
- [x] Script: Lua. Soon: LuaJIT, Teal, Hurdy, Erde and TypeScript.
- [x] UI: DearImgui abstraction.
- [x] Devices: Webcam, Keyboard, Mouse, Gamepads, Touch, Rumble.
- [x] Data files: JSON, JSON5, XML.
- [ ] Math libs: vecmath, pcg, noise, simd, actor/camera.
- [ ] Network: replicated filesystem snapshots. <!-- no sockets, network or ipc abstractions! we will use replicated tree/filesystems instead -->
- [ ] Profiler.

# Motivation and rationale

TBD.
<!--
aims to be fun to use.
fun to use = expressive C + frictionless + quick to build + rapid iteration
- [ ] Expressive C
- [ ] Frictionless
- [ ] Quick to build
- [ ] Rapid iteration
-->

# Build

```
cl hello.c && hello
```

# License
This software is multi-licensed into the [μLicense](https://github.com/r-lyeh/uLicense), [Unlicense](https://unlicense.org/), [0-BSD](https://opensource.org/licenses/0BSD) and [MIT (No Attribution)](https://github.com/aws/mit-0) licenses. Choose whichever license you prefer. Any contribution to this repository is implicitly subjected to the same licensing conditions aforementioned.

## Links
Still looking for alternatives? 
[amulet](https://github.com/ianmaclarty/amulet), 
[aroma](https://github.com/leafo/aroma/), 
[astera](https://github.com/tek256/astera), 
[blendelf](https://github.com/jesterKing/BlendELF), 
[bullordengine](https://github.com/MarilynDafa/Bulllord-Engine), 
[candle](https://github.com/EvilPudding/candle), 
[cave](https://github.com/kieselsteini/cave), 
[chickpea](https://github.com/ivansafrin/chickpea), 
[corange](https://github.com/orangeduck/Corange), 
[cute](https://github.com/RandyGaul/cute_framework), 
[dos-like](https://github.com/mattiasgustavsson/dos-like), 
[ejoy2d](https://github.com/ejoy/ejoy2d), 
[exengine](https://github.com/exezin/exengine), 
[game-framework](https://github.com/Planimeter/game-framework), 
[gunslinger](https://github.com/MrFrenik/gunslinger), 
[hate](https://github.com/excessive/hate), 
[high-impact](https://github.com/phoboslab/high_impact/), 
[horde3d](https://github.com/horde3d/Horde3D), 
[island](https://github.com/island-org/island), 
[juno](https://github.com/rxi/juno), 
[l](https://github.com/Lyatus/L), 
[limbus](https://github.com/redien/limbus), 
[love](https://github.com/love2d/love/), 
[lovr](https://github.com/bjornbytes/lovr), 
[mini3d](https://github.com/mini3d/mini3d), 
[mintaro](https://github.com/mackron/mintaro), 
[mio](https://github.com/ccxvii/mio), 
[ofx](https://openframeworks.cc), 
[olive.c](https://github.com/tsoding/olive.c), 
[opensource](https://github.com/w23/OpenSource), 
[ouzel](https://github.com/elnormous/ouzel/), 
[pez](https://github.com/prideout/pez), 
[pixie](https://github.com/mattiasgustavsson/pixie), 
[polycode](https://github.com/ivansafrin/Polycode), 
[punity](https://github.com/martincohen/Punity), 
[r96](https://github.com/badlogic/r96), 
[raygpu](https://github.com/manuel5975p/raygpu/), 
[raylib](https://github.com/raysan5/raylib),
[ricotech](https://github.com/dbechrd/RicoTech), 
[rizz](https://github.com/septag/rizz), 
[rvnicraven](https://github.com/Captain4LK/RvnicRaven-ray), 
[tigr](https://github.com/erkkah/tigr), 
[v1](https://github.com/r-lyeh/v1), <!-- boo<colony9<moon9<edge<eve<ava<fwk<v2<v3<kit -->
[v2](https://github.com/r-lyeh/v2), 
[v3](https://github.com/r-lyeh/v3), 
[yourgamelib](https://github.com/duddel/yourgamelib), 

<a href="https://github.com/r-lyeh/kit/issues"><img alt="Issues" src="https://img.shields.io/github/issues-raw/r-lyeh/kit.svg?label=Issues&logo=github&logoColor=white"/></a> <a href="https://discord.gg/yyjCkUQKPV"><img alt="Discord" src="https://img.shields.io/discord/354670964400848898?color=5865F2&label=Chat&logo=discord&logoColor=white"/></a>
