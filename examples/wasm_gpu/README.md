# DQ WebGPU wasm32-bare example

This is a browser-side equivalent of `examples/opengl/opengltest1.dq`.  DQ
creates GPU buffers and shaders, creates a pipeline, updates a uniform buffer,
and emits the frame commands.  `webgpu.js` only maps those calls to the WebGPU
JavaScript API and keeps opaque handle tables.

Build the module from the repository root:

```sh
build/dq-comp examples/wasm_gpu/webgpu_demo.dqproj
```

Serve this directory over HTTP (for example, `python3 -m http.server` from
`examples/wasm_gpu`) and open `index.html`.  A current Chromium, Firefox, or
Safari build with WebGPU enabled is required; loading a wasm module from a
`file://` URL is blocked by browsers.
