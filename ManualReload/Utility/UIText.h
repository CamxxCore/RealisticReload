#pragma once

enum eAlignment
{
	AlignCenter = 0,
	AlignLeft = 1,
	AlignRight = 2
};

class UIText
{
	bool m_enabled,
	m_shadow,
	m_outline,
	m_pulse, 
	m_pulseState;
	float m_xPos, m_yPos;
	eFont m_font;
	eAlignment m_align;
	float m_scale;
	std::string m_text;
	color_s m_color;

	const char * textCommandType = "CELL_EMAIL_BCON";
public:
	UIText(std::string text, float x, float y, float scale, eFont font, eAlignment align, int r, int g, int b, int a, bool pulse);
	~UIText();
	void SetText(std::string text);
	void SetScale(float scale);
	void SetFont(eFont font);
	void SetColor(color_s color);
	void Draw();
};
