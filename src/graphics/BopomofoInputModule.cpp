#include "configuration.h"

#if defined(BOPOMOFO_IME) && HAS_SCREEN

#include "graphics/BopomofoInputModule.h"
#include "graphics/ScreenFonts.h"
#include <OLEDDisplay.h>
#include <algorithm>
#include <cstring>

#ifndef OLED_CJK_SIZE
#define OLED_CJK_SIZE 10
#endif

namespace graphics
{

BopomofoInputModule::BopomofoInputModule()
{
    // Base class VirtualKeyboard() constructor is called automatically.
    // Timeout is managed by the inherited lastActivityTime / resetTimeout().
}

// ── Core IME helpers ────────────────────────────────────────────────────────

void BopomofoInputModule::refreshCandidates()
{
    candidateIdx_ = 0;
    engine_.refresh();
}

void BopomofoInputModule::confirmCandidate()
{
    if (!engine_.hasCandidates()) return;
    size_t idx = std::min((size_t)candidateIdx_, engine_.candidates().size() - 1);
    setInputText(getInputText() + engine_.select(idx));
    candidateIdx_ = 0;
}

void BopomofoInputModule::backspaceChar()
{
    if (engine_.composing()) {
        engine_.backspace();
        refreshCandidates();
        return;
    }
    // Delete last UTF-8 character from the committed input text
    std::string t = getInputText();
    if (t.empty()) return;
    // Walk backward to find the start of the last UTF-8 code unit sequence
    size_t pos = t.size();
    while (pos > 0) {
        --pos;
        uint8_t b = (uint8_t)t[pos];
        if ((b & 0xC0) != 0x80) break; // not a continuation byte → start found
    }
    setInputText(t.substr(0, pos));
}

// ── VirtualKeyboard overrides ───────────────────────────────────────────────

bool BopomofoInputModule::handleKeyChar(char c)
{
    resetTimeout();
    uint8_t uc = (uint8_t)c;

    // ── Backspace ──────────────────────────────────────────────────────────
    if (uc == 0x08) {
        if (!engine_.composing() && getInputText().empty()) {
            // Both composer and committed text are empty: exit keyboard (same as original freetext DEL-to-close)
            cancelInput();
        } else {
            backspaceChar();
        }
        return true;
    }

    // ── Ctrl+Space (0x02): toggle Chinese / English mode ─────────────────
    // Tab (0x09) is permanently intercepted by CannedMessageModule for channel switching.
    // Ctrl+Space generates 0x02 (IME_TOGGLE_CHAR) in CardputerKeyboard.cpp, which
    // CannedMessageModule ignores (kbchar < 32, not in any handler), so it reaches here.
    if (uc == 0x02) {
        toggleIME();
        return true;
    }

    // ── ESC: clear composition or cancel ──────────────────────────────────
    if (uc == 0x1B) {
        if (engine_.composing()) {
            engine_.clearComposition();
            candidateIdx_ = 0;
        } else {
            // Let caller handle full cancel via INPUT_BROKER_CANCEL
        }
        return true;
    }

    if (!chineseMode_) {
        // ── English mode ──────────────────────────────────────────────────
        if (uc == 0x0D || uc == 0x0A) { // Enter
            submitText();
            return true;
        }
        if (uc >= 0x20 && uc < 0x7F) {
            setInputText(getInputText() + c);
            return true;
        }
        return false;
    }

    // ── Chinese mode ──────────────────────────────────────────────────────

    if (uc == 0x0D || uc == 0x0A) { // Enter: confirm or submit
        if (engine_.hasCandidates()) {
            confirmCandidate();
        } else if (engine_.composing()) {
            // No candidates: discard composition and submit current committed text
            engine_.clearComposition();
        } else {
            submitText();
        }
        return true;
    }

    if (uc == 0x20) { // Space: first-tone commit
        if (engine_.hasCandidates()) {
            confirmCandidate();
        } else if (engine_.addSpace()) {
            refreshCandidates();
        } else if (!getInputText().empty()) {
            // No composition in progress, just pass through as space
            setInputText(getInputText() + ' ');
        }
        return true;
    }

    // Printable ASCII: try as Bopomofo key
    if (uc >= 0x21 && uc < 0x7F) {
        if (const bpmf::Symbol *sym = bpmf::lookup_key(c)) {
            // Candidates are refreshed on every symbol, so whether this one
            // closed the syllable makes no difference here - a tone is just
            // another symbol.
            engine_.addSymbol(*sym);
            refreshCandidates();
            return true;
        }
        // Not a Bopomofo key: if no composition in progress, pass through as ASCII
        if (!engine_.composing()) {
            setInputText(getInputText() + c);
            return true;
        }
        // Ignore characters that arrive mid-composition with no Bopomofo mapping
        return true;
    }

    return false;
}

void BopomofoInputModule::moveCursorLeft()
{
    resetTimeout();
    if (engine_.hasCandidates())
        candidateIdx_ = std::max(0, candidateIdx_ - 1);
}

void BopomofoInputModule::moveCursorRight()
{
    resetTimeout();
    if (engine_.hasCandidates())
        candidateIdx_ = std::min((int)engine_.candidates().size() - 1, candidateIdx_ + 1);
}

void BopomofoInputModule::handlePress()
{
    resetTimeout();
    if (engine_.hasCandidates()) {
        confirmCandidate();
    } else if (engine_.composing()) {
        // No candidates yet: discard composition
        engine_.clearComposition();
    } else {
        submitText();
    }
}

void BopomofoInputModule::handleLongPress()
{
    resetTimeout();
    // Long press always submits the committed text immediately
    engine_.clearComposition();
    submitText();
}

void BopomofoInputModule::toggleIME()
{
    resetTimeout();
    chineseMode_ = !chineseMode_;
    if (!chineseMode_) {
        // Clear any pending composition when switching to English
        engine_.clearComposition();
        candidateIdx_ = 0;
    }
}

// ── Draw ─────────────────────────────────────────────────────────────────────

void BopomofoInputModule::draw(OLEDDisplay *display, int16_t /*offsetX*/, int16_t /*offsetY*/)
{
    resetTimeout();

    int16_t W = (int16_t)display->getWidth();
    int16_t H = (int16_t)display->getHeight();
    int16_t fh = FONT_HEIGHT_SMALL;

    // Layout zones:
    //   header     : y=0,          h=fh
    //   separator
    //   text area  : y=fh+1,       h=H-fh*3-5
    //   separator
    //   composition: y=fh*?+?,     h=fh
    //   separator
    //   candidates : h=fh
    //   status     : remainder

    int16_t yHeader = 0;
    int16_t ySep1   = fh;
    int16_t yText   = fh + 1;
    int16_t textH   = H - fh * 3 - 5;
    if (textH < fh) textH = fh; // minimum 1 line
    int16_t ySep2   = yText + textH;
    int16_t yComp   = ySep2 + 1;
    int16_t ySep3   = yComp + fh;
    int16_t yCand   = ySep3 + 1;
    int16_t yStatus = yCand + fh + 1;

    display->setColor(WHITE);

    // Header
    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    std::string hdr = getHeader();
    if (!hdr.empty())
        display->drawString(2, yHeader, hdr.c_str());

    // Separator 1
    display->drawHorizontalLine(0, ySep1, W);

    // Text area (committed text)
    drawTextArea(display, 2, yText, W - 4, textH);

    // Separator 2
    display->drawHorizontalLine(0, ySep2, W);

    // Composition bar
    drawCompositionBar(display, 2, yComp, W - 4, fh);

    // Separator 3
    display->drawHorizontalLine(0, ySep3, W);

    // Candidate bar
    drawCandidateBar(display, 2, yCand, W - 4, fh);

    // Status bar
    if (yStatus < H)
        drawStatusBar(display, 0, yStatus, W, H - yStatus);
}

// ── Draw helpers ──────────────────────────────────────────────────────────────

void BopomofoInputModule::drawTextArea(OLEDDisplay *display, int16_t x, int16_t y, int16_t w, int16_t h)
{
    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_LEFT);

    std::string text = getInputText();
    if (text.empty()) return;

    int16_t lineH = FONT_HEIGHT_SMALL;
    int maxLines = h / lineH;
    if (maxLines < 1) maxLines = 1;

    // Approximate chars-per-line (CJK glyph ≈ OLED_CJK_SIZE px wide, ASCII ≈ 8 px)
    // Use display width to estimate, but just render with drawString and let the
    // library hardware-clip at the right edge. For overflow on the Y axis, we show
    // the LAST maxLines "blocks" so the cursor area is always visible.

    // Split into segments of ~w bytes (rough estimate: assume avg 12 px per char on 10×10 font)
    const size_t approxCharsPerLine = (size_t)(w / 10);
    const size_t approxBytesPerLine = approxCharsPerLine * 3; // worst-case 3 bytes/char

    // Find start offset: we want to show the last maxLines * approxBytesPerLine bytes
    size_t startByte = 0;
    size_t totalBytes = text.size();
    size_t windowBytes = (size_t)(maxLines)*approxBytesPerLine;
    if (totalBytes > windowBytes) {
        startByte = totalBytes - windowBytes;
        // Align to UTF-8 char boundary
        while (startByte < totalBytes && (text[startByte] & 0xC0) == 0x80)
            ++startByte;
    }

    // Draw one line at a time from the visible portion
    const char *ptr = text.c_str() + startByte;
    int16_t curY = y;
    while (*ptr && curY + lineH <= y + h) {
        // Find how much of ptr fits in one line width
        const char *lineStart = ptr;
        int16_t consumed = 0;
        while (*ptr) {
            // Determine byte length of next UTF-8 char
            uint8_t b = (uint8_t)*ptr;
            int charLen = (b < 0x80) ? 1 : (b < 0xE0) ? 2 : (b < 0xF0) ? 3 : 4;
            // Rough width estimate
            int16_t charW = (charLen == 1) ? 8 : OLED_CJK_SIZE;
            if (consumed + charW > w) break;
            consumed += charW;
            ptr += charLen;
        }
        // Draw this line segment
        std::string line(lineStart, (size_t)(ptr - lineStart));
        display->drawString(x, curY, line.c_str());
        curY += lineH;
    }
}

void BopomofoInputModule::drawCompositionBar(OLEDDisplay *display, int16_t x, int16_t y, int16_t /*w*/, int16_t /*h*/)
{
    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_LEFT);

    if (engine_.composing()) {
        display->drawString(x, y, engine_.composingText().c_str());
    } else if (chineseMode_) {
        display->drawString(x, y, "_"); // ready-for-input placeholder
    } else {
        display->drawString(x, y, "[EN]");
    }
}

void BopomofoInputModule::drawCandidateBar(OLEDDisplay *display, int16_t x, int16_t y, int16_t w, int16_t h)
{
    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_LEFT);

    const std::vector<std::string> &candidates = engine_.candidates();
    if (candidates.empty()) {
        if (chineseMode_ && engine_.composing())
            display->drawString(x, y, "---");
        return;
    }

    // Determine which page to show based on current cursor position
    int pageStart = (candidateIdx_ / CAND_PER_PAGE) * CAND_PER_PAGE;
    int pageEnd   = std::min((int)candidates.size(), pageStart + CAND_PER_PAGE);

    int16_t cx = x;
    for (int i = pageStart; i < pageEnd; i++) {
        const std::string &cand = candidates[i];
        int16_t sw = (int16_t)display->getStringWidth(cand.c_str(), cand.size(), true);
        if (cx + sw > x + w) break;

        bool selected = (i == candidateIdx_);
        if (selected) {
            // Highlight selected candidate with inverted rectangle
            display->fillRect(cx - 1, y, sw + 2, h - 1);
            display->setColor(BLACK);
        }
        display->drawString(cx, y, cand.c_str());
        if (selected)
            display->setColor(WHITE);

        cx += sw + 4; // gap between candidates
    }

    // Show ">" if more candidates exist beyond the current page
    if ((int)candidates.size() > pageStart + CAND_PER_PAGE) {
        display->setTextAlignment(TEXT_ALIGN_RIGHT);
        display->drawString(x + w, y, ">");
        display->setTextAlignment(TEXT_ALIGN_LEFT);
    }
}

void BopomofoInputModule::drawStatusBar(OLEDDisplay *display, int16_t x, int16_t y, int16_t w, int16_t /*h*/)
{
    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_RIGHT);

    const char *layoutName = bpmf::LAYOUT_NAME; // "Daqian26" or "Eten(ET41)"
    std::string statusStr = chineseMode_
        ? (std::string("CH/") + layoutName)
        : "EN";

    display->drawString((int16_t)(x + w - 2), y, statusStr.c_str());
    display->setTextAlignment(TEXT_ALIGN_LEFT);
}

} // namespace graphics

#endif // BOPOMOFO_IME && HAS_SCREEN
