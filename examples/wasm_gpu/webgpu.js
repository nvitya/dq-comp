// This is intentionally only an ABI adapter: resource lifetime, shaders, and
// every render command are selected by the DQ module through numeric handles.

const canvas = document.querySelector("#screen");
const status = document.querySelector("#status");
const encoder = new TextDecoder();

function fail(message) {
  status.textContent = message;
  throw new Error(message);
}

if (!navigator.gpu) fail("WebGPU is unavailable in this browser.");

const adapter = await navigator.gpu.requestAdapter();
if (!adapter) fail("No WebGPU adapter is available.");
const device = await adapter.requestDevice();
const context = canvas.getContext("webgpu");
const format = navigator.gpu.getPreferredCanvasFormat();
device.addEventListener("uncapturederror", ({ error }) => {
  status.textContent = "WebGPU error: " + error.message;
  status.style.color = "#fca5a5";
  console.error(error);
});
device.lost.then(({ message }) => {
  status.textContent = "WebGPU device lost" + (message ? ": " + message : "");
  status.style.color = "#fca5a5";
});
const buffers = [null];
const shaders = [null];
const pipelines = [null];
let memory;
let pass;
let command_encoder;
let vertex_buffer;
let uniform_buffer;
let current_pipeline;
let current_layout;
let bind_groups = new Map();
let configured = false;

function handle(objects, value) {
  objects.push(value);
  return objects.length - 1;
}

function object(objects, id, label) {
  const value = objects[id];
  if (!value) throw new Error(`Invalid ${label} handle: ${id}`);
  return value;
}

function resize() {
  const scale = Math.min(devicePixelRatio, device.limits.maxTextureDimension2D / Math.max(1, canvas.clientWidth));
  const width = Math.max(1, Math.floor(canvas.clientWidth * scale));
  const height = Math.max(1, Math.floor(canvas.clientHeight * scale));
  if (canvas.width !== width || canvas.height !== height) {
    canvas.width = width;
    canvas.height = height;
    configured = false;
  }
  if (!configured) {
    context.configure({ device, format, alphaMode: "opaque" });
    configured = true;
  }
}

const webgpu = {
  create_buffer(size, usage) {
    return handle(buffers, device.createBuffer({ size, usage }));
  },
  write_buffer(buffer, address, size) {
    device.queue.writeBuffer(object(buffers, buffer, "buffer"), 0, memory.buffer, address, size);
  },
  create_shader(address, length) {
    const source = encoder.decode(new Uint8Array(memory.buffer, address, length));
    return handle(shaders, device.createShaderModule({ code: source }));
  },
  create_render_pipeline(vertex_shader, fragment_shader) {
    const layout = device.createBindGroupLayout({
      entries: [{ binding: 0, visibility: GPUShaderStage.VERTEX, buffer: { type: "uniform" } }]
    });
    const pipeline = device.createRenderPipeline({
      layout: device.createPipelineLayout({ bindGroupLayouts: [layout] }),
      vertex: {
        module: object(shaders, vertex_shader, "shader"), entryPoint: "vs_main",
        buffers: [{ arrayStride: 20, attributes: [
          { shaderLocation: 0, offset: 0, format: "float32x2" },
          { shaderLocation: 1, offset: 8, format: "float32x3" }
        ] }]
      },
      fragment: { module: object(shaders, fragment_shader, "shader"), entryPoint: "fs_main", targets: [{ format }] },
      primitive: { topology: "triangle-list" }
    });
    return handle(pipelines, { pipeline, layout });
  },
  begin_frame(red, green, blue, alpha) {
    resize();
    command_encoder = device.createCommandEncoder();
    pass = command_encoder.beginRenderPass({ colorAttachments: [{
      view: context.getCurrentTexture().createView(),
      clearValue: { r: red, g: green, b: blue, a: alpha }, loadOp: "clear", storeOp: "store"
    }] });
  },
  set_pipeline(id) {
    const selected = object(pipelines, id, "pipeline");
    pass.setPipeline(selected.pipeline);
    current_pipeline = id;
    current_layout = selected.layout;
  },
  set_vertex_buffer(id) { vertex_buffer = object(buffers, id, "buffer"); },
  set_uniform_buffer(id) { uniform_buffer = object(buffers, id, "buffer"); },
  draw(vertex_count) {
    if (!current_layout || !vertex_buffer || !uniform_buffer) throw new Error("Incomplete render state");
    const key = `${current_pipeline}:${uniform_buffer}`;
    let group = bind_groups.get(key);
    if (!group) {
      group = device.createBindGroup({ layout: current_layout, entries: [{ binding: 0, resource: { buffer: uniform_buffer } }] });
      bind_groups.set(key, group);
    }
    pass.setVertexBuffer(0, vertex_buffer);
    pass.setBindGroup(0, group);
    pass.draw(vertex_count);
  },
  end_frame() {
    pass.end();
    device.queue.submit([command_encoder.finish()]);
    pass = command_encoder = undefined;
    current_layout = undefined;
  }
};

try {
  const response = await fetch("webgpu_demo.wasm");
  if (!response.ok) fail(`Could not load webgpu_demo.wasm: ${response.status} ${response.statusText}`);
  const { instance } = await WebAssembly.instantiate(await response.arrayBuffer(), { webgpu });
  memory = instance.exports.memory;
  instance.exports.Init();
  status.textContent = "DQ controls WebGPU buffers, shaders, and render commands.";
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
  status.textContent = `Could not start WebGPU: ${error.message}`;
}
