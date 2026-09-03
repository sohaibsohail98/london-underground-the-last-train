import { defineConfig } from 'vite';
import { fileURLToPath } from 'node:url';

export default defineConfig({
  base: './',
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url)),
    },
  },
  assetsInclude: ['**/*.hdr', '**/*.ktx2', '**/*.glb'],
  build: {
    target: 'esnext',
    sourcemap: true,
    assetsInlineLimit: 0,
    rollupOptions: {
      output: {
        manualChunks: {
          three: ['three', 'three/webgpu', 'three/tsl'],
        },
      },
    },
  },
  esbuild: {
    target: 'esnext',
  },
  server: {
    port: 5173,
  },
});
