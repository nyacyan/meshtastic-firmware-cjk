#pragma once
#include "configuration.h"

#if defined(BOPOMOFO_IME) && HAS_SCREEN

#include "graphics/VirtualKeyboard.h"
#include "graphics/bpmf_engine.h"
#include <string>
#include <vector>

namespace graphics
{

// BopomofoInputModule - Bopomofo (注音) IME for the M5Stack Cardputer physical keyboard.
//
// Inherits VirtualKeyboard so it can be assigned to NotificationRenderer::virtualKeyboard
// and participate in the existing text_input notification pipeline without modifying the
// core draw path.  Only draw(), handleKeyChar(), moveCursorLeft/Right(), handlePress(),
// handleLongPress(), and toggleIME() are overridden; everything else (timeout, setCallback,
// setHeader, setInputText/getInputText, submitText, deleteCharacter) is inherited.
//
// Screen layout (240×135, FONT_HEIGHT_SMALL ≈ 19 px):
//   [Header          ]  ← fh px
//   ─────────────────
//   [Committed text  ]  ← (H - fh*3 - 5) px, multi-line
//   ─────────────────
//   [Composition bar ]  ← fh px  (current syllable buffer, e.g. "ㄐㄧˊ")
//   ─────────────────
//   [Candidate bar   ]  ← fh px  (up to CAND_PER_PAGE candidates)
//   [Status bar      ]  ← remainder (CH/EN mode + layout name)

class BopomofoInputModule : public VirtualKeyboard
{
  public:
    BopomofoInputModule();

    // ── Overrides ────────────────────────────────────────────────────────────
    void draw(OLEDDisplay *display, int16_t offsetX, int16_t offsetY) override;

    // Physical key input: called by NotificationRenderer when INPUT_BROKER_ANYKEY
    // or INPUT_BROKER_BACK arrives while this module is active.
    bool handleKeyChar(char c) override;

    void moveCursorLeft() override;   // navigate candidates
    void moveCursorRight() override;
    void handlePress() override;      // confirm selected candidate / submit
    void handleLongPress() override;  // long-press: submit text immediately
    void toggleIME() override;        // toggle Chinese ↔ English mode

  private:
    // Composition, candidates and their cache all belong to the engine; what
    // stays here is the part that is genuinely about this screen - which
    // candidate the cursor is on, and whether the user is in Chinese mode.
    bpmf::Engine engine_;
    int candidateIdx_ = 0;
    bool chineseMode_ = true;

    static constexpr int CAND_PER_PAGE = 5;

    void refreshCandidates();
    void confirmCandidate();
    void backspaceChar();

    // Draw helpers
    void drawTextArea(OLEDDisplay *display, int16_t x, int16_t y, int16_t w, int16_t h);
    void drawCompositionBar(OLEDDisplay *display, int16_t x, int16_t y, int16_t w, int16_t h);
    void drawCandidateBar(OLEDDisplay *display, int16_t x, int16_t y, int16_t w, int16_t h);
    void drawStatusBar(OLEDDisplay *display, int16_t x, int16_t y, int16_t w, int16_t h);
};

} // namespace graphics

#endif // BOPOMOFO_IME && HAS_SCREEN
