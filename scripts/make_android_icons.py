#!/usr/bin/env python3
"""
Generates the Android launcher icons from assets/ui/icons/masquerade.png

Usage (from the repository root, needs `pip install pillow`):
    python make_android_icons.py assets/ui/icons/masquerade.png android/app/src/main/res [#101820]

The third argument is the background colour of the adaptive icon (default #101820).
Existing ic_launcher* files in the target res folder are removed first (the Android Studio template ships .webp /
vector versions with the same resource names, which would otherwise cause "Duplicate resources" build errors).
"""
import glob, os, sys
from PIL import Image, ImageDraw

LEGACY = {"mdpi": 48, "hdpi": 72, "xhdpi": 96, "xxhdpi": 144, "xxxhdpi": 192}   # ic_launcher.png sizes (px)
ADAPTIVE = {k: v * 9 // 4 for k, v in LEGACY.items()}                            # 108dp layer: 108,162,216,324,432 px
SAFE_ZONE = 0.62        # fraction of the 108dp adaptive layer used by the artwork (launcher masks the outer edge)

def square(img):
    side = max(img.size)
    out = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    out.paste(img, ((side - img.width) // 2, (side - img.height) // 2))
    return out

def main():
    if len(sys.argv) < 3:
        print(__doc__); sys.exit(1)
    src, res = sys.argv[1], sys.argv[2]
    bg = sys.argv[3] if len(sys.argv) > 3 else "#101820"
    logo = square(Image.open(src).convert("RGBA"))

    # remove template icons with the same resource names
    for pattern in ("mipmap-*/ic_launcher*", "drawable*/ic_launcher_foreground*", "drawable*/ic_launcher_background*"):
        for f in glob.glob(os.path.join(res, pattern)):
            os.remove(f); print("removed", f)

    for density, size in LEGACY.items():
        d = os.path.join(res, f"mipmap-{density}"); os.makedirs(d, exist_ok=True)
        icon = logo.resize((size, size), Image.LANCZOS)
        icon.save(os.path.join(d, "ic_launcher.png"))
        # round version: same artwork cropped to a circle
        mask = Image.new("L", (size * 4, size * 4), 0)
        ImageDraw.Draw(mask).ellipse((0, 0, size * 4 - 1, size * 4 - 1), fill=255)
        mask = mask.resize((size, size), Image.LANCZOS)
        round_icon = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        round_icon.paste(icon, (0, 0), mask)
        round_icon.save(os.path.join(d, "ic_launcher_round.png"))

    for density, size in ADAPTIVE.items():
        d = os.path.join(res, f"mipmap-{density}"); os.makedirs(d, exist_ok=True)
        art = int(size * SAFE_ZONE)
        layer = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        layer.paste(logo.resize((art, art), Image.LANCZOS), ((size - art) // 2, (size - art) // 2))
        layer.save(os.path.join(d, "ic_launcher_foreground.png"))

    v26 = os.path.join(res, "mipmap-anydpi-v26"); os.makedirs(v26, exist_ok=True)
    xml = ('<?xml version="1.0" encoding="utf-8"?>\n'
           '<adaptive-icon xmlns:android="http://schemas.android.com/apk/res/android">\n'
           '    <background android:drawable="@color/ic_launcher_background" />\n'
           '    <foreground android:drawable="@mipmap/ic_launcher_foreground" />\n'
           '</adaptive-icon>\n')
    for name in ("ic_launcher.xml", "ic_launcher_round.xml"):
        open(os.path.join(v26, name), "w", encoding="utf-8").write(xml)

    values = os.path.join(res, "values"); os.makedirs(values, exist_ok=True)
    open(os.path.join(values, "ic_launcher_background.xml"), "w", encoding="utf-8").write(
        '<?xml version="1.0" encoding="utf-8"?>\n<resources>\n'
        f'    <color name="ic_launcher_background">{bg}</color>\n</resources>\n')
    print("done:", res)

main()
