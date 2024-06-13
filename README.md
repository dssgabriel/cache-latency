# Cache latency benchmark

Simple random pointer chasing loop benchmark to measure average cache access latency.

## Quick start

**Pre-requisites:**
- C23 conforming compiler
- Meson
- Ninja

**Build**
```sh
meson setup <builddir>
meson compile -C <builddir>
```

**Run**
```sh
./<builddir>/cache-latency [MAX_MEMORY_SIZE]
```

## Todo-list

- [ ] Add a script to generate plots

## Credits

Code heavily inspired by Yaspr's (not on GitHub anymore) cache latency benchmark in the `ybench` tool.
