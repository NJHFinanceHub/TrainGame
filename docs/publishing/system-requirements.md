# System Requirements

## Minimum Requirements

| Component | Specification |
|-----------|---------------|
| **OS** | Windows 10 64-bit (version 1909 or later) |
| **Processor** | Intel Core i5-8400 / AMD Ryzen 5 2600 |
| **Memory** | 12 GB RAM |
| **Graphics** | NVIDIA GeForce GTX 1060 6GB / AMD Radeon RX 580 8GB |
| **DirectX** | Version 12 |
| **Storage** | 50 GB available space (SSD recommended) |
| **Sound** | DirectX compatible sound card |
| **Additional Notes** | HDD supported but SSD strongly recommended for car streaming performance. Internet required for Steam Workshop and Cloud Saves. |

## Recommended Requirements

| Component | Specification |
|-----------|---------------|
| **OS** | Windows 10/11 64-bit |
| **Processor** | Intel Core i7-10700 / AMD Ryzen 7 3700X |
| **Memory** | 16 GB RAM |
| **Graphics** | NVIDIA GeForce RTX 3060 / AMD Radeon RX 6600 XT |
| **DirectX** | Version 12 |
| **Storage** | 50 GB available space (SSD required) |
| **Sound** | DirectX compatible sound card |
| **Additional Notes** | SSD required for recommended experience. RTX hardware enables full Lumen global illumination quality. |

## Rationale

### CPU
- UE5 GameplayAbilities and NPC AI systems require solid multi-threaded performance
- Minimum targets 4-core/6-thread at ~3.8 GHz base
- Recommended targets 8-core/16-thread for AI, physics, and car streaming overlap

### GPU
- Nanite mesh streaming requires DX12-capable hardware with 6+ GB VRAM
- Lumen dynamic GI scales well but benefits from ray tracing hardware
- GTX 1060 6GB handles Nanite software fallback at 1080p/30 FPS medium settings
- RTX 3060 enables hardware ray tracing for Lumen at 1080p/60 FPS high settings

### RAM
- 12 GB minimum for car streaming buffer (active car + adjacent cars in memory)
- 16 GB recommended for seamless streaming with no hitching during zone transitions

### Storage
- 50 GB accounts for: UE5 Nanite-compressed meshes (~20 GB), textures (~15 GB), audio (~8 GB), cinematics and other assets (~7 GB)
- SSD strongly recommended: the car-by-car streaming architecture benefits heavily from fast sequential reads
- HDD supported at minimum spec but may experience visible pop-in during rapid car transitions

### Steam Deck
- Runs at Medium settings, 1280x800, 30 FPS target
- 50 GB storage requirement fits on 256 GB model with room for OS and other games
- FSR 2.0 upscaling recommended for Deck play
