# Font licences

The CJK headers under `variants/` hold rasterised bitmap data generated from the
third-party fonts listed below. No font file is redistributed with this
firmware; only glyph bitmaps are embedded. The full terms for every font used in
a generation chain are kept in this directory.

## Character set

The Hanzi coverage of `traditional_chinese_utf8_10x10.h` and
`traditional_chinese_utf8_16x16.h` (5093 and 5115 codepoints) is the Ministry
of Education of Taiwan's "常用國字標準字體表" (Standard List of Frequently Used
Characters), 4808 characters plus 281 non-Hanzi symbols, published at
<https://language.moe.gov.tw/uploads/files/17694982751288.pdf> and parsed with
`pdfplumber`. This list only decides which codepoints are included; no glyph
shape or other content is taken from it, and it carries no licence terms of
its own to reproduce here.

## traditional_chinese_utf8_10x10.h

Used through `OLED_CJK_CUSTOM` by the `gat562_family_zhtw`,
`seeed_wio_tracker_L1_zhtw`, `heltec-v3_zhtw`, `heltec-v4_zhtw`,
`heltec-v4-r8-oled_zhtw` and the three `nrf52_promicro_diy_tcxo*_zhtw`
environments. Each codepoint comes from the first source in the chain that
covers it.

| Source | Role | Terms | File |
|------|------|-------|------|
| Hand-drawn for this project | The four Bopomofo tone marks, U+02C7 U+02CA U+02CB U+02D9 | No third-party font involved | - |
| Fusion Pixel Font 10px monospaced zh_hant (TakWolf) | Main font, 5038 of 5093 glyphs | SIL OFL 1.1, Reserved Font Name "Fusion Pixel" | `LICENSE-fusion-pixel-font.txt` |
| Noto Sans CJK TC DemiLight (Google) | Last-resort fallback, 51 glyphs: the Greek letters and one CJK character | SIL OFL 1.1 | `LICENSE-noto-sans-cjk.txt` |

The tone marks are drawn by hand rather than taken from a font because they are
shown on every keystroke that carries a tone, and an outline font rasterised to
10px leaves them as one or two isolated pixels.

Fusion Pixel Font is itself assembled from several pixel fonts. Its upstream
licences are kept alongside as `LICENSE-fusion-pixel-upstream-*.txt`, covering
Ark Pixel, Boutique Bitmap 9x9 and Galmuri.

## traditional_chinese_utf8_16x16.h

Generated from Noto Sans CJK (SIL OFL 1.1, `LICENSE-noto-sans-cjk.txt`) and, for
the emoji it carries, Noto Emoji (SIL OFL 1.1, same terms). Used by the
`m5stack-cardputer-adv_zhtw`, `heltec-wireless-paper_zhtw` and
`heltec-mesh-node-t114_zhtw` environments, and the header repeats these terms in
its own file comment.

## src/graphics/emotes_tw_16x16.h

Emoji bitmaps for the `EMOTES_TW_16X16` build flag. 114 of the 115 glyphs are
taken 1:1 from the GNU Unifont `.hex` bitmaps - sampled on the pixel grid,
neither rescaled nor redrawn.

| Font | Role | Terms | File |
|------|------|-------|------|
| GNU Unifont 17.0.05 | 114 of 115 glyphs | Dual-licensed since 13.0.04; this project takes the SIL OFL 1.1 arm | `LICENSE-unifont.txt` |

The other arm of Unifont's dual licence is GPL v2 or later with the GNU font
embedding exception; either arm may be used. Its `COPYING` carves out
`jiskan16-plane00.hex` and `jiskan16-plane02.hex` as Public Domain, but those
files no longer ship in 17.0.05, and every glyph used here comes from plane 00
(a handful of symbols) or plane 01 (the emoji), both covered by the dual licence.

The remaining glyph, the Taiwan flag, is hand-designed for this project and is
not derived from Unifont: the flag is a sequence of two regional indicators,
which Unifont renders as two boxed letters rather than one glyph.

## src/graphics/emotes_tw_10x10.h

Emoji bitmaps for the `EMOTES_TW_10X10` build flag, used by `gat562_family_zhtw`,
`seeed_wio_tracker_L1_zhtw`, `heltec-v3_zhtw`, `heltec-v4_zhtw`,
`heltec-v4-r8-oled_zhtw` and the three `nrf52_promicro_diy_tcxo*_zhtw`
environments. **No third-party font is involved.**
All 90 entries are hand-designed for this project, because no bitmap emoji font
exists at this size - Unifont stops at 16x16 and downscaling destroys the
pixel-grid alignment that makes a bitmap font legible. The 90 entries share 85
distinct bitmaps: emoji that render identically point at one copy.
