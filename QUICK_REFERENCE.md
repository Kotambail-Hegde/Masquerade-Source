# ⚡ Quick Reference - Ultra-Fast CI/CD

## 📦 Required Files

```
your-repo/
├── .github/
│   └── workflows/
│       └── build.yml          ← GitHub Actions workflow
├── vcpkg.json                 ← Dependency manifest (REQUIRED)
├── CMakePresets.json          ← Build presets (optional)
├── CMakeLists.txt             ← Your existing file
└── (your other files)
```

## 🎯 Quick Commands

### Initial Setup
```bash
# Add the new files
git add vcpkg.json .github/workflows/build.yml
git commit -m "Add ultra-fast CI/CD"
git push origin ci_cd_support_1
```

### Update vcpkg Baseline
```bash
# Get latest commit
git ls-remote https://github.com/microsoft/vcpkg HEAD

# Update in vcpkg.json and build.yml
# Then commit and push
```

### Clear Caches (if needed)
```bash
gh cache delete --all
# Or go to: Settings → Actions → Caches
```

## 🔑 Key Features

✅ **GitHub Actions Binary Caching** - Pre-compiled packages  
✅ **SDL3 Caching** - Saves 5-7 minutes on Emscripten builds  
✅ **Parallel Compilation** - Uses all CPU cores  
✅ **Automatic vcpkg** - No manual setup needed  

## ⏱️ Build Times

| Stage | First Run | Cached |
|-------|-----------|--------|
| Windows | ~5 min | **~2 min** |
| Linux | ~4 min | **~2 min** |
| Emscripten | ~8 min | **~2 min** |
| **Total** | **~15 min** | **~3-5 min** |

## 🌐 Access Your Build

**GitHub Pages**: `https://YOUR_USERNAME.github.io/YOUR_REPO/`

## 🔧 Enable GitHub Pages

1. Go to: **Settings** → **Pages**
2. Source: **GitHub Actions**
3. Save

## 📊 Monitor

- **Builds**: `https://github.com/USER/REPO/actions`
- **Caches**: `https://github.com/USER/REPO/settings/actions/caches`
- **Pages**: `https://github.com/USER/REPO/settings/pages`

## 🐛 Troubleshooting

**Slow builds?**
→ Check if caches are being restored in logs

**Build fails?**
→ Check workflow logs for specific errors

**Pages not deploying?**
→ Verify Pages is enabled & branch name matches

## 📝 Important Notes

- First build creates all caches (~15 min)
- Subsequent builds use caches (~3-5 min)
- Caches expire after 7 days of no use
- Matrix builds run in parallel (Windows/Linux/Emscripten)

---

**Environment Variables Set by Workflow:**
- `VCPKG_ROOT` - Path to vcpkg installation
- `SDL3_ROOT` - Path to SDL3 installation (Emscripten only)
- `VCPKG_BINARY_SOURCES` - Binary cache configuration

**Artifacts Uploaded:**
- `windows-build/` - Windows executable + DLLs
- `linux-build/` - Linux binary
- `emscripten-build/` - Web files (HTML/JS/WASM)
