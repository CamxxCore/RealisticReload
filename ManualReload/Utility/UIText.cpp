#include "stdafx.h"
#include "UIText.h"

UIText::UIText( std::string text, float x, float y, float scale, eFont font, eAlignment align, int r, int g, int b, int a, bool pulse ) {
    m_text = text;
    m_xPos = x;
    m_yPos = y;
    m_scale = scale;
    m_font = font;
    m_align = align;
    m_shadow = false;
    m_outline = true;
    m_color.r = r;
    m_color.g = g;
    m_color.b = b;
    m_color.a = a;
    m_pulse = pulse;
    m_enabled = true;
}

void UIText::SetText( std::string text ) {
    m_text = text;
}

void UIText::SetScale( float scale ) {
    m_scale = scale;
}

void UIText::SetFont( eFont font ) {
    m_font = font;
}

void UIText::SetColor( color_s color ) {
    m_color = color;
}

void UIText::Draw() {
    if ( m_enabled ) {
        float x = m_xPos / 1280.0f;
        float y = m_yPos / 720.0f;

        if ( m_pulse ) {
            bool b = false;

            if ( m_pulseState ) {
                if ( b |= m_color.a < 255 ) {
                    m_color.a = min( 255, m_color.a + 5 );
                }

                m_pulseState = b;
            }

            else {
                if ( b |= m_color.a > 100 ) {
                    m_color.a = max( 100, m_color.a - 5 );
                }

                m_pulseState = !b;
            }
        }

        if ( m_shadow )
            UI::SET_TEXT_DROP_SHADOW();

        if ( m_outline )
            UI::SET_TEXT_OUTLINE();

        UI::SET_TEXT_FONT( m_font );

        UI::SET_TEXT_SCALE( m_scale, m_scale );

        UI::SET_TEXT_COLOUR( m_color.r, m_color.g, m_color.b, m_color.a );

        UI::SET_TEXT_JUSTIFICATION( m_align );

        UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT( textCommandType );

        UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME( m_text.c_str() );

        UI::END_TEXT_COMMAND_DISPLAY_TEXT( x, y );
    }
}

UIText::~UIText()
{ }
