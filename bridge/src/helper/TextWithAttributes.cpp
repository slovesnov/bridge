/*
 * TextWithAttributes.cpp
 *
 *       Created on: 09.09.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "../base/Config.h"
#include "TextWithAttributes.h"

TextWithAttributes::TextWithAttributes(std::string text, int height,
		bool underline, bool bold, bool blackColor) {
	init(text, height, underline, bold, blackColor);
}

void TextWithAttributes::init(std::string text, int height, bool underline,
		bool bold, bool blackColor) {
	m_text = text;
	m_height = height;
	m_underline = underline;
	m_bold = bold;
	m_blackColor = blackColor;
}

TextWithAttributes::TextWithAttributes(std::string text, bool underlined) {
	init(text, gconfig->getFontHeight(), underlined, false, false);
}

TextWithAttributes::TextWithAttributes(const gchar* text) {
	init(text, gconfig->getFontHeight(), false, false, false);
}

TextWithAttributes::~TextWithAttributes() {
}

