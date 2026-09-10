// Browser-side implementation of the imports declared in webgl.dq.
// It keeps WebGL objects private and exposes stable numeric handles to Wasm.

export function createWebGLInterface(gl, canvas, getMemory) {
  const decoder = new TextDecoder();
  const buffers = [null];
  const shaders = [null];
  const programs = [null];
  const uniforms = [null];

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
    const scale = Math.min(devicePixelRatio, gl.getParameter(gl.MAX_VIEWPORT_DIMS)[0] / Math.max(1, canvas.clientWidth));
    const width = Math.max(1, Math.floor(canvas.clientWidth * scale));
    const height = Math.max(1, Math.floor(canvas.clientHeight * scale));
    if (canvas.width !== width || canvas.height !== height) {
      canvas.width = width;
      canvas.height = height;
    }
    gl.viewport(0, 0, canvas.width, canvas.height);
  }

  return {
    GlCreateBuffer(size, usage) {
      const buffer = gl.createBuffer();
      if (!buffer) throw new Error("Could not create WebGL buffer");
      gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
      gl.bufferData(gl.ARRAY_BUFFER, size, usage);
      return handle(buffers, buffer);
    },
    GlWriteBuffer(buffer, address, size) {
      gl.bindBuffer(gl.ARRAY_BUFFER, object(buffers, buffer, "buffer"));
      gl.bufferSubData(gl.ARRAY_BUFFER, 0, new Uint8Array(getMemory().buffer, address, size));
    },
    GlCreateShader(type, address, length) {
      const shader = gl.createShader(type);
      if (!shader) throw new Error("Could not create WebGL shader");
      gl.shaderSource(shader, decoder.decode(new Uint8Array(getMemory().buffer, address, length)));
      gl.compileShader(shader);
      if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        const info = gl.getShaderInfoLog(shader);
        gl.deleteShader(shader);
        throw new Error(`WebGL shader compilation failed: ${info}`);
      }
      return handle(shaders, shader);
    },
    GlCreateProgram(vertex_shader, fragment_shader) {
      const program = gl.createProgram();
      if (!program) throw new Error("Could not create WebGL program");
      gl.attachShader(program, object(shaders, vertex_shader, "shader"));
      gl.attachShader(program, object(shaders, fragment_shader, "shader"));
      gl.linkProgram(program);
      if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
        const info = gl.getProgramInfoLog(program);
        gl.deleteProgram(program);
        throw new Error(`WebGL program link failed: ${info}`);
      }
      return handle(programs, program);
    },
    GlGetUniformLocation(program, address, length) {
      const name = decoder.decode(new Uint8Array(getMemory().buffer, address, length));
      const location = gl.getUniformLocation(object(programs, program, "program"), name);
      if (location === null) throw new Error(`WebGL uniform not found: ${name}`);
      return handle(uniforms, location);
    },
    GlBeginFrame(red, green, blue, alpha) {
      resize();
      gl.clearColor(red, green, blue, alpha);
      gl.clear(gl.COLOR_BUFFER_BIT);
    },
    GlSetProgram(program) { gl.useProgram(object(programs, program, "program")); },
    GlSetVertexBuffer(buffer) { gl.bindBuffer(gl.ARRAY_BUFFER, object(buffers, buffer, "buffer")); },
    GlVertexAttribPointer(index, component_count, stride, offset) {
      gl.enableVertexAttribArray(index);
      gl.vertexAttribPointer(index, component_count, gl.FLOAT, false, stride, offset);
    },
    GlSetUniform2f(location, x, y) { gl.uniform2f(object(uniforms, location, "uniform"), x, y); },
    GlDraw(vertex_count) { gl.drawArrays(gl.TRIANGLES, 0, vertex_count); },
    GlEndFrame() {}
  };
}
