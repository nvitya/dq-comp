import { createWebGLInterface } from "./webgl-interface.js";

const canvas = document.querySelector("#screen");
const status = document.querySelector("#status");
const gl = canvas.getContext("webgl2");

function fail(message) {
  status.textContent = message;
  status.style.color = "#fca5a5";
  throw new Error(message);
}

if (!gl) fail("WebGL 2 is unavailable in this browser.");

let memory;
const webgl = createWebGLInterface(gl, canvas, () => memory);

try {
  const response = await fetch("triangle.wasm");
  if (!response.ok) fail(`Could not load triangle.wasm: ${response.status} ${response.statusText}`);
  const { instance } = await WebAssembly.instantiate(await response.arrayBuffer(), { webgl });
  memory = instance.exports.memory;
  instance.exports.Init();
  status.textContent = "DQ controls WebGL buffers, shaders, and render commands.";
  status.style.color = "#94a3b8";
  let previous;
  function frame(now) {
    const elapsed = previous === undefined ? 0 : now - previous;
    previous = now;
    instance.exports.Render(elapsed / 1000);
    requestAnimationFrame(frame);
  }
  requestAnimationFrame(frame);
} catch (error) {
  status.textContent = `Could not start WebGL: ${error.message}`;
  status.style.color = "#fca5a5";
}
