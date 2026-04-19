// plugin_waitstring.cpp - Text editing (WaitString) functions
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "wasabi.h"
#include "resource.h"

void CPlugin::WaitString_NukeSelection() {
  if (m_waitstring.bActive &&
      m_waitstring.nSelAnchorPos != -1) {
    // nuke selection.  note: start & end are INCLUSIVE.
    int start = (m_waitstring.nCursorPos < m_waitstring.nSelAnchorPos) ? m_waitstring.nCursorPos : m_waitstring.nSelAnchorPos;
    int end = (m_waitstring.nCursorPos > m_waitstring.nSelAnchorPos) ? m_waitstring.nCursorPos - 1 : m_waitstring.nSelAnchorPos - 1;
    int len = (m_waitstring.bDisplayAsCode ? lstrlenA((char*)m_waitstring.szText) : lstrlenW(m_waitstring.szText));
    int how_far_to_shift = end - start + 1;
    int num_chars_to_shift = len - end;  // includes NULL char

    if (m_waitstring.bDisplayAsCode) {
      char* ptr = (char*)m_waitstring.szText;
      for (int i = 0; i < num_chars_to_shift; i++)
        *(ptr + start + i) = *(ptr + start + i + how_far_to_shift);
    } else {
      for (int i = 0; i < num_chars_to_shift; i++)
        m_waitstring.szText[start + i] = m_waitstring.szText[start + i + how_far_to_shift];
    }

    // clear selection
    m_waitstring.nCursorPos = start;
    m_waitstring.nSelAnchorPos = -1;
  }
}

void CPlugin::WaitString_Cut() {
  if (m_waitstring.bActive &&
      m_waitstring.nSelAnchorPos != -1) {
    WaitString_Copy();
    WaitString_NukeSelection();
  }
}

void CPlugin::WaitString_Copy() {
  if (m_waitstring.bActive &&
      m_waitstring.nSelAnchorPos != -1) {
    // note: start & end are INCLUSIVE.
    int start = (m_waitstring.nCursorPos < m_waitstring.nSelAnchorPos) ? m_waitstring.nCursorPos : m_waitstring.nSelAnchorPos;
    int end = (m_waitstring.nCursorPos > m_waitstring.nSelAnchorPos) ? m_waitstring.nCursorPos - 1 : m_waitstring.nSelAnchorPos - 1;
    int chars_to_copy = end - start + 1;

    if (m_waitstring.bDisplayAsCode) {
      char* ptr = (char*)m_waitstring.szText;
      for (int i = 0; i < chars_to_copy; i++)
        m_waitstring.szClipboard[i] = *(ptr + start + i);
      m_waitstring.szClipboard[chars_to_copy] = 0;

      char tmp[64000];
      ConvertLFCToCRsA(m_waitstring.szClipboard, tmp);
      copyStringToClipboardA(tmp);
    } else {
      for (int i = 0; i < chars_to_copy; i++)
        m_waitstring.szClipboardW[i] = m_waitstring.szText[start + i];
      m_waitstring.szClipboardW[chars_to_copy] = 0;

      wchar_t tmp[64000];
      ConvertLFCToCRsW(m_waitstring.szClipboardW, tmp);
      copyStringToClipboardW(tmp);
    }
  }
}

void CPlugin::WaitString_Paste() {
  // NOTE: if there is a selection, it is wiped out, and replaced with the clipboard contents.

  if (m_waitstring.bActive) {
    WaitString_NukeSelection();

    if (m_waitstring.bDisplayAsCode) {
      char tmp[64000];
      lstrcpyA(tmp, getStringFromClipboardA());
      ConvertCRsToLFCA(tmp, m_waitstring.szClipboard);
    } else {
      wchar_t tmp[64000];
      lstrcpyW(tmp, getStringFromClipboardW());
      ConvertCRsToLFCW(tmp, m_waitstring.szClipboardW);
    }

    int len;
    int chars_to_insert;

    if (m_waitstring.bDisplayAsCode) {
      len = lstrlenA((char*)m_waitstring.szText);
      chars_to_insert = lstrlenA(m_waitstring.szClipboard);
    } else {
      len = lstrlenW(m_waitstring.szText);
      chars_to_insert = lstrlenW(m_waitstring.szClipboardW);
    }

    if (len + chars_to_insert + 1 >= m_waitstring.nMaxLen) {
      chars_to_insert = m_waitstring.nMaxLen - len - 1;

      // inform user
      AddError(wasabiApiLangString(IDS_STRING_TOO_LONG), 2.5f, ERR_MISC, true);
    } else {
      // m_fShowUserMessageUntilThisTime = GetTime();	// if there was an error message already, clear it
    }

    int i;
    if (m_waitstring.bDisplayAsCode) {
      char* ptr = (char*)m_waitstring.szText;
      for (i = len; i >= m_waitstring.nCursorPos; i--)
        *(ptr + i + chars_to_insert) = *(ptr + i);
      for (i = 0; i < chars_to_insert; i++)
        *(ptr + i + m_waitstring.nCursorPos) = m_waitstring.szClipboard[i];
    } else {
      for (i = len; i >= m_waitstring.nCursorPos; i--)
        m_waitstring.szText[i + chars_to_insert] = m_waitstring.szText[i];
      for (i = 0; i < chars_to_insert; i++)
        m_waitstring.szText[i + m_waitstring.nCursorPos] = m_waitstring.szClipboardW[i];
    }
    m_waitstring.nCursorPos += chars_to_insert;
  }
}

void CPlugin::WaitString_SeekLeftWord() {
  // move to beginning of prior word
  if (m_waitstring.bDisplayAsCode) {
    char* ptr = (char*)m_waitstring.szText;
    while (m_waitstring.nCursorPos > 0 &&
           !IsAlphanumericChar(*(ptr + m_waitstring.nCursorPos - 1)))
      m_waitstring.nCursorPos--;

    while (m_waitstring.nCursorPos > 0 &&
           IsAlphanumericChar(*(ptr + m_waitstring.nCursorPos - 1)))
      m_waitstring.nCursorPos--;
  } else {
    while (m_waitstring.nCursorPos > 0 &&
           !IsAlphanumericChar(m_waitstring.szText[m_waitstring.nCursorPos - 1]))
      m_waitstring.nCursorPos--;

    while (m_waitstring.nCursorPos > 0 &&
           IsAlphanumericChar(m_waitstring.szText[m_waitstring.nCursorPos - 1]))
      m_waitstring.nCursorPos--;
  }
}

void CPlugin::WaitString_SeekRightWord() {
  // move to beginning of next word

  // testing  lotsa   stuff

  if (m_waitstring.bDisplayAsCode) {
    int len = lstrlenA((char*)m_waitstring.szText);

    char* ptr = (char*)m_waitstring.szText;
    while (m_waitstring.nCursorPos < len &&
           IsAlphanumericChar(*(ptr + m_waitstring.nCursorPos)))
      m_waitstring.nCursorPos++;

    while (m_waitstring.nCursorPos < len &&
           !IsAlphanumericChar(*(ptr + m_waitstring.nCursorPos)))
      m_waitstring.nCursorPos++;
  } else {
    int len = lstrlenW(m_waitstring.szText);

    while (m_waitstring.nCursorPos < len &&
           IsAlphanumericChar(m_waitstring.szText[m_waitstring.nCursorPos]))
      m_waitstring.nCursorPos++;

    while (m_waitstring.nCursorPos < len &&
           !IsAlphanumericChar(m_waitstring.szText[m_waitstring.nCursorPos]))
      m_waitstring.nCursorPos++;
  }
}

int CPlugin::WaitString_GetCursorColumn() {
  if (m_waitstring.bDisplayAsCode) {
    int column = 0;
    char* ptr = (char*)m_waitstring.szText;
    while (m_waitstring.nCursorPos - column - 1 >= 0 &&
           *(ptr + m_waitstring.nCursorPos - column - 1) != LINEFEED_CONTROL_CHAR)
      column++;

    return column;
  } else {
    return m_waitstring.nCursorPos;
  }
}

int CPlugin::WaitString_GetLineLength() {
  int line_start = m_waitstring.nCursorPos - WaitString_GetCursorColumn();
  int line_length = 0;

  if (m_waitstring.bDisplayAsCode) {
    char* ptr = (char*)m_waitstring.szText;
    while (*(ptr + line_start + line_length) != 0 &&
           *(ptr + line_start + line_length) != LINEFEED_CONTROL_CHAR)
      line_length++;
  } else {
    while (m_waitstring.szText[line_start + line_length] != 0 &&
           m_waitstring.szText[line_start + line_length] != LINEFEED_CONTROL_CHAR)
      line_length++;
  }

  return line_length;
}

void CPlugin::WaitString_SeekUpOneLine() {
  int column = g_plugin.WaitString_GetCursorColumn();

  if (column != m_waitstring.nCursorPos) {
    // seek to very end of previous line (cursor will be at the semicolon)
    m_waitstring.nCursorPos -= column + 1;

    int new_column = g_plugin.WaitString_GetCursorColumn();

    if (new_column > column)
      m_waitstring.nCursorPos -= (new_column - column);
  }
}

void CPlugin::WaitString_SeekDownOneLine() {
  int column = g_plugin.WaitString_GetCursorColumn();
  int newpos = m_waitstring.nCursorPos;

  char* ptr = (char*)m_waitstring.szText;
  while (*(ptr + newpos) != 0 && *(ptr + newpos) != LINEFEED_CONTROL_CHAR)
    newpos++;

  if (*(ptr + newpos) != 0) {
    m_waitstring.nCursorPos = newpos + 1;

    while (column > 0 &&
           *(ptr + m_waitstring.nCursorPos) != LINEFEED_CONTROL_CHAR &&
           *(ptr + m_waitstring.nCursorPos) != 0) {
      m_waitstring.nCursorPos++;
      column--;
    }
  }
}
