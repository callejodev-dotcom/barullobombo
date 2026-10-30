#include "PluginEditor.h"
#include "BinaryData.h"
#include "Presets.h"

namespace
{
    using escoita::U;
    using escoita::numero;
}

namespace ui
{
// ================================================================ tipografía
// Montserrat (títulos e rótulos, en maiúsculas espaciadas coma na web de Escoita) e Inter
// (texto corrido): as da familia Barullo. As dúas, licenza OFL.
static juce::Typeface::Ptr tf (int which)
{
    static juce::Typeface::Ptr t[3] = {
        juce::Typeface::createSystemTypefaceFor (BinaryData::InterMedium_ttf, BinaryData::InterMedium_ttfSize),
        juce::Typeface::createSystemTypefaceFor (BinaryData::MontserratSemiBold_ttf, BinaryData::MontserratSemiBold_ttfSize),
        juce::Typeface::createSystemTypefaceFor (BinaryData::MontserratBold_ttf, BinaryData::MontserratBold_ttfSize) };
    return t[which];
}
juce::Font body (float s) { return juce::Font (juce::FontOptions (tf (0)).withHeight (s)); }
juce::Font semi (float s) { return juce::Font (juce::FontOptions (tf (1)).withHeight (s)); }
juce::Font cond (float s) { return juce::Font (juce::FontOptions (tf (2)).withHeight (s)); }
juce::Font caps (float s, float tracking) { return juce::Font (juce::FontOptions (tf (1)).withHeight (s).withKerningFactor (tracking)); }
juce::String upper (const juce::String& t) { return escoita::maiusculas (t); }

void drawCaps (juce::Graphics& g, const juce::String& t, juce::Rectangle<float> r, juce::Justification j, float size, float tracking)
{
    g.setFont (caps (size, tracking));
    g.drawText (upper (t), r, j, false);
}

void glowText (juce::Graphics& g, const juce::String& t, juce::Rectangle<float> r, juce::Justification j, juce::Colour c)
{
    g.setColour (c.withAlpha (0.16f));
    for (const auto d : { juce::Point<float> (-0.8f, 0.0f), { 0.8f, 0.0f }, { 0.0f, -0.8f }, { 0.0f, 0.8f } })
        g.drawText (t, r.translated (d.x, d.y), j, false);
    g.setColour (c);
    g.drawText (t, r, j, false);
}

// ================================================================ pezas do aparello
void paintPanel (juce::Graphics& g, juce::Rectangle<float> r, float rad)
{
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (juce::Colours::black.withAlpha (0.11f));
        g.fillRoundedRectangle (r.translated (0.0f, 2.0f * (float) i).expanded ((float) i), rad + (float) i);
    }
    g.setGradientFill (juce::ColourGradient (col::panel1, 0.0f, r.getY(), col::panel2, 0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, rad);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.fillRect (r.getX() + rad, r.getY() + 0.5f, r.getWidth() - 2.0f * rad, 1.0f);
}

void paintWell (juce::Graphics& g, juce::Rectangle<float> r, float rad)
{
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.0f), rad);
    g.setColour (col::visor.brighter (0.06f));
    g.fillRoundedRectangle (r, rad);
    juce::Path p; p.addRoundedRectangle (r, rad);
    g.saveState();
    g.reduceClipRegion (p);
    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.65f), 0.0f, r.getY(), juce::Colours::transparentBlack, 0.0f, r.getY() + 5.0f, false));
    g.fillRect (r.withHeight (6.0f));
    g.restoreState();
}

void paintGlass (juce::Graphics& g, juce::Rectangle<float> r, float rad)
{
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.0f), rad);
    juce::ColourGradient gr (col::cristal1, r.getCentreX(), r.getY(), col::cristal2, r.getCentreX(), r.getBottom(), false);
    gr.addColour (0.55, juce::Colour (0xff0a1129));
    g.setGradientFill (gr);
    g.fillRoundedRectangle (r, rad);
    juce::Path p; p.addRoundedRectangle (r, rad);
    g.saveState();
    g.reduceClipRegion (p);
    for (int i = 0; i < 5; ++i)
    {
        g.setColour (juce::Colours::black.withAlpha (0.16f - 0.03f * (float) i));
        g.drawRoundedRectangle (r.reduced ((float) i + 0.5f), rad, 1.0f);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.05f), r.getX(), r.getY(),
                                             juce::Colours::transparentWhite, r.getX() + r.getWidth() * 0.32f, r.getY() + r.getHeight() * 0.55f, false));
    g.fillRect (r);
    g.restoreState();
}

void paintKey (juce::Graphics& g, juce::Rectangle<float> r, bool over, bool down)
{
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (r.translated (0.0f, 2.0f), 7.0f);
    if (down) r.translate (0.0f, 1.0f);
    const float lift = over && ! down ? 0.10f : 0.0f;
    g.setGradientFill (juce::ColourGradient (col::tecla1.brighter (lift), 0.0f, r.getY(), col::tecla2.brighter (lift * 0.6f), 0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, 7.0f);
    g.setColour (juce::Colours::white.withAlpha (down ? 0.04f : 0.10f));
    g.fillRect (r.getX() + 6.0f, r.getY() + 0.5f, r.getWidth() - 12.0f, 1.0f);
}

void paintLed (juce::Graphics& g, juce::Point<float> c, bool on, juce::Colour luz)
{
    if (on)
    {
        g.setColour (luz.withAlpha (0.16f)); g.fillEllipse (c.x - 7.0f, c.y - 7.0f, 14.0f, 14.0f);
        g.setColour (luz.withAlpha (0.34f)); g.fillEllipse (c.x - 4.6f, c.y - 4.6f, 9.2f, 9.2f);
        g.setColour (luz); g.fillEllipse (c.x - 3.0f, c.y - 3.0f, 6.0f, 6.0f);
        g.setColour (juce::Colours::white.withAlpha (0.55f)); g.fillEllipse (c.x - 1.3f, c.y - 2.0f, 2.2f, 2.0f);
    }
    else
    {
        g.setColour (juce::Colour (0xff3a3326)); g.fillEllipse (c.x - 3.0f, c.y - 3.0f, 6.0f, 6.0f);
        g.setColour (juce::Colours::black.withAlpha (0.6f)); g.drawEllipse (c.x - 3.0f, c.y - 3.0f, 6.0f, 6.0f, 0.8f);
    }
}

// rótulo dun grupo: na súa cor e cunha liña debaixo
void groupTitle (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& t, juce::Colour c)
{
    g.setColour (c);
    drawCaps (g, t, r.withHeight (14.0f), juce::Justification::centred, 9.0f, 0.28f);
    g.setColour (c.withAlpha (0.25f));
    g.fillRect (r.getX() + 8.0f, r.getY() + 18.0f, r.getWidth() - 16.0f, 1.0f);
}

// ================================================================ aspecto
Look::Look()
{
    setColour (juce::ResizableWindow::backgroundColourId, col::noite);
    setColour (juce::Label::textColourId, col::crema);
    setColour (juce::TextEditor::backgroundColourId, col::visor);
    setColour (juce::TextEditor::textColourId, col::ouro);
    setColour (juce::TextEditor::outlineColourId, col::lina);
    setColour (juce::TextEditor::focusedOutlineColourId, col::laranxa);
    setColour (juce::PopupMenu::backgroundColourId, col::panel2);
    setColour (juce::PopupMenu::textColourId, col::crema);
    setColour (juce::PopupMenu::headerTextColourId, col::laranxa);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, col::laranxa);
    setColour (juce::PopupMenu::highlightedTextColourId, col::noite);
    setColour (juce::TextButton::buttonColourId, col::tecla2);
    setColour (juce::TextButton::textColourOffId, col::crema2);
    setColour (juce::TextButton::textColourOnId, col::crema);
    setColour (juce::AlertWindow::backgroundColourId, col::panel2);
    setColour (juce::AlertWindow::textColourId, col::crema);
    setColour (juce::AlertWindow::outlineColourId, col::lina);
}

juce::Typeface::Ptr Look::getTypefaceForFont (const juce::Font& f) { return tf (f.isBold() ? 1 : 0); }

// as teclas: en relevo, cun LED (aceso se está escollida; «led» = a cor da capa) e o texto en
// maiúsculas espaciadas; «gold» = un visor dourado (o selector da combinación)
void Look::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f).withTrimmedBottom (1.5f);
    if (b.getProperties().contains ("gold"))
    {
        paintWell (g, r, 8.0f);
        if (over) { g.setColour (col::laranxa.withAlpha (0.08f)); g.fillRoundedRectangle (r, 8.0f); }
        return;
    }
    paintKey (g, r, over, down);
    if (down) r.translate (0.0f, 1.0f);
    if (! b.getProperties().contains ("noLed"))
    {
        const auto luz = b.getProperties().contains ("led") ? juce::Colour ((juce::uint32) (juce::int64) b.getProperties()["led"]) : col::laranxa;
        paintLed (g, { r.getX() + 12.0f, r.getCentreY() }, b.getToggleState(), luz);
    }
}

void Look::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f).withTrimmedBottom (1.5f);
    if (b.getProperties().contains ("gold"))
    {
        g.setFont (caps (12.0f, 0.1f));
        glowText (g, upper (b.getButtonText()), r.reduced (14.0f, 0.0f).withTrimmedRight (16.0f), juce::Justification::centredLeft, col::ouro);
        juce::Path arrow;
        const float ax = r.getRight() - 18.0f, ay = r.getCentreY();
        arrow.addTriangle (ax - 5.0f, ay - 2.5f, ax + 5.0f, ay - 2.5f, ax, ay + 3.5f);
        g.setColour (col::ouro); g.fillPath (arrow);
        return;
    }
    if (down) r.translate (0.0f, 1.0f);
    if (! b.getProperties().contains ("noLed")) r.removeFromLeft (14.0f);
    juce::Colour c = b.getToggleState() || over ? col::crema : col::crema2;
    if (! b.isEnabled()) c = col::crema3.withAlpha (0.6f);
    g.setColour (c);
    const int curve = b.getProperties()["curve"];   // desfacer (−1) e refacer (+1): unha frecha en arco
    if (curve != 0)
    {
        const auto ctr = r.getCentre(); const float rad = 6.5f;
        const float a0 = curve > 0 ? -2.4f : 2.4f, a1 = curve > 0 ? 1.2f : -1.2f;
        juce::Path arc; arc.addCentredArc (ctr.x, ctr.y + 1.0f, rad, rad, 0.0f, a0, a1, true);
        g.strokePath (arc, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        const juce::Point<float> tip (ctr.x + rad * std::sin (a1), ctr.y + 1.0f - rad * std::cos (a1));
        juce::Path head; head.addTriangle (tip.x + (float) curve * 4.0f, tip.y, tip.x - (float) curve * 1.5f, tip.y - 4.0f, tip.x - (float) curve * 1.5f, tip.y + 4.0f);
        g.fillPath (head);
        return;
    }
    g.setFont (caps ((float) b.getProperties().getWithDefault ("fontSize", 9.5f), 0.15f));
    g.drawFittedText (upper (b.getButtonText()), r.toNearestInt().reduced (3, 0), juce::Justification::centred, 1, 0.7f);
}

// os mandos: disco mecanizado, anel de LED laranxa e o valor nun visor debaixo
void Look::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    auto area = juce::Rectangle<int> (x, y, w, h).toFloat();
    auto lcd = area.removeFromBottom (17.0f);
    lcd = lcd.withSizeKeepingCentre (juce::jmin (66.0f, area.getWidth() - 2.0f), 17.0f);
    area.removeFromBottom (3.0f);
    const float d = juce::jmin (area.getWidth(), area.getHeight());
    const auto c = area.withSizeKeepingCentre (d, d).getCentre();
    const float ring = d * 0.5f - 4.0f;
    const auto luz = s.getProperties().contains ("led") ? juce::Colour ((juce::uint32) (juce::int64) s.getProperties()["led"]) : col::laranxa;

    juce::Path track; track.addCentredArc (c.x, c.y, ring, ring, 0.0f, a0, a1, true);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const bool bip = s.getMinimum() < 0 && s.getMaximum() > 0;
    const double zeroAt = bip ? 0.0 : s.getMinimum();
    const float zero = a0 + (float) s.valueToProportionOfLength (zeroAt) * (a1 - a0);
    const float ang = a0 + pos * (a1 - a0);
    if (std::abs (ang - zero) > 0.01f)
    {
        juce::Path val; val.addCentredArc (c.x, c.y, ring, ring, 0.0f, juce::jmin (zero, ang), juce::jmax (zero, ang), true);
        const auto lit = s.isEnabled() ? luz : col::crema3;
        g.setColour (lit.withAlpha (0.12f)); g.strokePath (val, juce::PathStrokeType (8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (lit.withAlpha (0.26f)); g.strokePath (val, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (lit); g.strokePath (val, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const float kr = ring - 7.0f;
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillEllipse (c.x - kr, c.y - kr + 2.5f, kr * 2.0f, kr * 2.0f);
    juce::ColourGradient disc (juce::Colour (0xff4a5577), c.x - kr * 0.3f, c.y - kr * 0.45f, juce::Colour (0xff0e1323), c.x + kr * 0.55f, c.y + kr * 1.05f, true);
    disc.addColour (0.55, juce::Colour (0xff232b45));
    g.setGradientFill (disc);
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f, 1.0f);
    const juce::Point<float> tip (c.x + kr * 0.82f * std::sin (ang), c.y - kr * 0.82f * std::cos (ang));
    g.setColour (col::crema.withAlpha (s.isEnabled() ? 0.92f : 0.4f));
    g.drawLine ({ c, tip }, 2.0f);

    paintWell (g, lcd, 5.0f);
    g.setFont (caps (9.0f, 0.04f));
    glowText (g, upper (s.getTextFromValue (s.getValue())), lcd, juce::Justification::centred, s.isEnabled() ? col::ouro : col::crema3);
}

// os deslizadores: a pista afundida, a luz laranxa desde abaixo ata o capuchón e o capuchón mecanizado
void Look::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    const float cx = (float) x + (float) w * 0.5f;
    const float top = (float) y, bot = (float) (y + h);
    auto slot = juce::Rectangle<float> (cx - 4.5f, top - 7.0f, 9.0f, bot - top + 14.0f);
    paintWell (g, slot, 4.5f);
    const int steps = s.getMaximum() - s.getMinimum() <= 3.5 ? (int) (s.getMaximum() - s.getMinimum()) : 10;
    g.setColour (col::crema3.withAlpha (0.55f));
    for (int t = 0; t <= steps; ++t)
    {
        const float ty = bot - (bot - top) * (float) t / (float) juce::jmax (1, steps);
        const float len = (t % 5 == 0 || steps <= 3) ? 5.0f : 2.5f;
        g.fillRect (cx - 9.0f - len, ty - 0.5f, len, 1.0f);
        g.fillRect (cx + 9.0f, ty - 0.5f, len, 1.0f);
    }
    if (pos < bot - 0.5f)
    {
        auto lit = juce::Rectangle<float> (cx - 1.5f, pos, 3.0f, bot + 4.0f - pos);
        g.setColour (col::laranxa.withAlpha (0.14f)); g.fillRoundedRectangle (lit.expanded (3.0f, 0.0f), 3.0f);
        g.setColour (col::laranxa.withAlpha (0.30f)); g.fillRoundedRectangle (lit.expanded (1.5f, 0.0f), 2.0f);
        g.setColour (col::laranxa); g.fillRoundedRectangle (lit, 1.5f);
    }
    auto cap = juce::Rectangle<float> (cx - 13.0f, pos - 7.0f, 26.0f, 14.0f);
    g.setColour (juce::Colours::black.withAlpha (0.55f)); g.fillRoundedRectangle (cap.translated (0.0f, 2.5f), 4.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a5577), cap.getX(), cap.getY(), juce::Colour (0xff141a2e), cap.getX(), cap.getBottom(), false));
    g.fillRoundedRectangle (cap, 4.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f)); g.drawRoundedRectangle (cap.reduced (0.5f), 4.0f, 1.0f);
    g.setColour (col::crema); g.fillRect (cap.getX() + 4.0f, pos - 0.8f, cap.getWidth() - 8.0f, 1.6f);
}

void Look::drawTooltip (juce::Graphics& g, const juce::String& text, int w, int h)
{
    auto r = juce::Rectangle<int> (w, h).toFloat();
    g.setColour (col::panel2); g.fillRoundedRectangle (r, 6.0f);
    g.setColour (col::lina); g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);
    juce::AttributedString s; s.setJustification (juce::Justification::topLeft);
    s.append (text, body (13.5f), col::crema);
    juce::TextLayout tl; tl.createLayoutWithBalancedLineLengths (s, 300.0f);
    tl.draw (g, r.reduced (10.0f, 7.0f));
}

juce::Rectangle<int> Look::getTooltipBounds (const juce::String& text, juce::Point<int> pos, juce::Rectangle<int> parent)
{
    juce::AttributedString s; s.append (text, body (13.5f));
    juce::TextLayout tl; tl.createLayoutWithBalancedLineLengths (s, 300.0f);
    const int w = (int) tl.getWidth() + 22, h = (int) tl.getHeight() + 16;
    return juce::Rectangle<int> (pos.x > parent.getCentreX() ? pos.x - (w + 12) : pos.x + 12,
                                 pos.y > parent.getCentreY() ? pos.y - (h + 8) : pos.y + 18, w, h).constrainedWithin (parent);
}

void Look::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    g.fillAll (col::panel2);
    g.setColour (col::lina); g.drawRect (0, 0, w, h);
}

// ================================================================ mando
Knob::Knob()
{
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    slider.setMouseDragSensitivity (180);
    addAndMakeVisible (slider);
}

void Knob::paint (juce::Graphics& g)
{
    g.setColour (slider.isEnabled() ? col::crema2 : col::crema3);
    drawCaps (g, name, getLocalBounds().removeFromBottom (12).toFloat(), juce::Justification::centred, 7.5f, 0.16f);
}



// ================================================================ a pantalla
static juce::String tempo (double ms) { return ms < 999.5 ? juce::String (juce::roundToInt (ms)) + " ms" : numero (ms / 1000.0, ms < 9950 ? 2 : 1) + " s"; }
static juce::String hz (double v) { return v < 999.5 ? numero (v, v < 99.95 ? 1 : 0) + " Hz" : numero (v / 1000.0, v < 9950 ? 1 : 0) + " kHz"; }

// a nota máis próxima a unha frecuencia e os cents que lle faltan ou sobran (Do central = Do3)
static juce::String notaDe (double f)
{
    static const char* n[12] = { "Do", "Do♯", "Re", "Mi♭", "Mi", "Fa", "Fa♯", "Sol", "La♭", "La", "Si♭", "Si" };
    const double m = 69.0 + 12.0 * std::log2 (f / 440.0);
    const int nota = (int) std::lround (m);
    const int cents = (int) std::lround ((m - nota) * 100.0);
    juce::String t = U (n[((nota % 12) + 12) % 12]) + juce::String (nota / 12 - 2);
    if (cents != 0) t << " " << (cents > 0 ? "+" : U ("−")) << std::abs (cents) << " cents";
    return t;
}

static const char* nomesMotor[bombo::NUM_MOTORES] = { "808", "909", "Sintético", "Acústico" };

void Pantalla::calcular (const bombo::Axustes& a)
{
    feito = a; hai = true;
    notaFeita = proc.ultimaNota.load();
    // un golpe co son de agora, a unha frecuencia baixa (abonda para velo)
    duracion = juce::jlimit (0.12, 6.0, a.caida / 1000.0 * 1.1 + (a.sala > 0.01 ? 0.25 : 0.03));
    const double srP = duracion > 1.5 ? 8000.0 : 16000.0;
    bombo::Bombo b;
    b.axustar (a);
    b.preparar (srP);
    b.disparar (1.0, a.seguir ? notaFeita : 36);
    const int cols = 640, total = juce::jmax (cols, (int) (duracion * srP));
    minimos.assign ((size_t) cols, 0.0f); maximos.assign ((size_t) cols, 0.0f);
    for (int i = 0; i < total; ++i)
    {
        double l, r;
        b.procesar (l, r);
        const float v = (float) (0.5 * (l + r));
        const size_t c = (size_t) ((long long) i * cols / total);
        minimos[c] = juce::jmin (minimos[c], v); maximos[c] = juce::jmax (maximos[c], v);
    }
}

void Pantalla::tick()
{
    const double agora = juce::Time::getMillisecondCounterHiRes();
    const double dt = juce::jlimit (0.0, 200.0, agora - ultimoTick);
    ultimoTick = agora;
    if (desdeGolpe >= 0) desdeGolpe += dt;
    const int g = proc.golpes.load();
    if (g != golpesVistos) { golpesVistos = g; desdeGolpe = 0; led = 1; rastro.clear(); }
    const double tonAgora = proc.tonAgora.load();
    if (desdeGolpe >= 0 && tonAgora > 0 && (rastro.empty() || desdeGolpe > rastro.back().x)) rastro.push_back ({ desdeGolpe, tonAgora });
    led *= 0.85f;
    saida = juce::jmax (proc.nivelSaida.load(), saida * 0.88f);
    const auto a = proc.axustes();
    if (! hai || a != feito || (a.seguir && proc.ultimaNota.load() != notaFeita)) calcular (a);
    repaint();
}

void Pantalla::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    paintGlass (g, r, 12.0f);
    if (! hai) calcular (proc.axustes());
    const auto& a = feito;
    const int nota = a.seguir ? notaFeita : 36;

    auto in = r.reduced (18.0f, 14.0f);
    auto meters = in.removeFromRight (44.0f);
    in.removeFromRight (14.0f);
    // o rótulo: o motor e a afinación (coa súa nota)
    {
        auto top = in.removeFromTop (14.0f);
        g.setColour (col::crema3);
        drawCaps (g, U ("Corpo · ") + U (nomesMotor[juce::jlimit (0, bombo::NUM_MOTORES - 1, a.motor)]), top, juce::Justification::centredLeft, 8.0f, 0.24f);
        const double f0 = bombo::frecuenciaBase (a, nota);
        drawCaps (g, (a.seguir ? U ("Segue a nota · ") : U ("Afinación · ")) + hz (f0) + U (" · ") + notaDe (f0), top, juce::Justification::centredRight, 8.0f, 0.24f);
    }
    in.removeFromTop (6.0f);
    auto graf = in.withTrimmedBottom (14.0f);
    auto xOf = [&] (double t) { return graf.getX() + (float) (t / duracion) * graf.getWidth(); };

    // a reixa: o cero e unhas marcas de tempo
    g.setColour (col::lina.withAlpha (0.5f));
    g.fillRect (graf.getX(), graf.getCentreY(), graf.getWidth(), 1.0f);
    const double ms = duracion * 1000.0;
    const double paso = ms > 3000 ? 1000 : ms > 1500 ? 500 : ms > 600 ? 200 : ms > 300 ? 100 : 50;
    g.setFont (caps (7.0f, 0.06f));
    for (double t = 0; t <= ms + 1e-9; t += paso)
    {
        g.setColour (col::lina.withAlpha (0.5f));
        g.fillRect (xOf (t / 1000.0), graf.getY(), 1.0f, graf.getHeight());
        g.setColour (col::crema3);
        g.drawText (tempo (t), juce::Rectangle<float> (xOf (t / 1000.0) + 3.0f, graf.getBottom() + 2.0f, 60.0f, 11.0f), juce::Justification::centredLeft, false);
    }

    // o ton: baixa co barrido (escala logarítmica, á dereita)
    const double fMax = bombo::tonEn (a, 0.0, nota);
    const double fLo = 20.0, fHi = juce::jlimit (250.0, 4000.0, fMax * 1.35);
    auto yOfF = [&] (double f) { return graf.getBottom() - (float) (std::log (f / fLo) / std::log (fHi / fLo)) * graf.getHeight(); };
    g.setFont (caps (7.0f, 0.06f));
    for (double f : { 25.0, 50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0 })
    {
        if (f > fHi * 0.95) break;
        const float y = yOfF (f);
        g.setColour (col::azul.withAlpha (0.10f));
        g.fillRect (graf.getX(), y, graf.getWidth(), 1.0f);
        g.setColour (col::azul.withAlpha (0.5f));
        g.drawText (f < 999.5 ? juce::String (juce::roundToInt (f)) + " Hz" : juce::String (juce::roundToInt (f / 1000.0)) + " kHz",
                    juce::Rectangle<float> (graf.getRight() - 64.0f, y - 11.0f, 62.0f, 10.0f), juce::Justification::centredRight, false);
    }

    // a onda: o mínimo e o máximo de cada columna
    if (! maximos.empty())
    {
        const float cy = graf.getCentreY(), h = graf.getHeight() * 0.5f / 0.9f;
        const int cols = (int) maximos.size();
        juce::Path onda;
        for (int c = 0; c < cols; ++c)
        {
            const float x = graf.getX() + graf.getWidth() * ((float) c + 0.5f) / (float) cols;
            const float y = cy - juce::jlimit (-0.95f, 0.95f, maximos[(size_t) c]) * h;
            if (c == 0) onda.startNewSubPath (x, y); else onda.lineTo (x, y);
        }
        for (int c = cols - 1; c >= 0; --c)
            onda.lineTo (graf.getX() + graf.getWidth() * ((float) c + 0.5f) / (float) cols, cy - juce::jlimit (-0.95f, 0.95f, minimos[(size_t) c]) * h);
        onda.closeSubPath();
        g.setGradientFill (juce::ColourGradient (col::laranxa.withAlpha (0.55f), graf.getX(), cy, col::laranxa.withAlpha (0.18f), graf.getRight(), cy, false));
        g.fillPath (onda);
        g.setColour (col::laranxa.withAlpha (0.9f));
        g.strokePath (onda, juce::PathStrokeType (1.0f));
    }

    // a curva do ton
    {
        juce::Path p;
        const int pasos = 200;
        for (int i = 0; i <= pasos; ++i)
        {
            const double t = duracion * i / pasos;
            const float x = xOf (t), y = yOfF (juce::jlimit (fLo, fHi, bombo::tonEn (a, t, nota)));
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (col::azul.withAlpha (0.25f)); g.strokePath (p, juce::PathStrokeType (5.0f));
        g.setColour (col::azul); g.strokePath (p, juce::PathStrokeType (1.8f));
    }

    // o ton que soou de verdade desde o último golpe: co glide, vese ir dunha nota á outra
    if (rastro.size() > 1)
    {
        juce::Path p;
        bool dentro = false;
        for (auto& q : rastro)
        {
            if (q.x > ms) break;
            const float x = xOf (q.x / 1000.0), y = yOfF (juce::jlimit (fLo, fHi, q.y));
            if (dentro) p.lineTo (x, y); else p.startNewSubPath (x, y);
            dentro = true;
        }
        g.setColour (col::crema.withAlpha (0.25f)); g.strokePath (p, juce::PathStrokeType (4.0f));
        g.setColour (col::crema); g.strokePath (p, juce::PathStrokeType (1.4f));
    }

    // o cursor: por onde vai o último golpe
    if (desdeGolpe >= 0 && desdeGolpe <= ms)
    {
        const float x = xOf (desdeGolpe / 1000.0);
        g.setColour (col::crema.withAlpha (0.18f)); g.fillRect (x - 2.0f, graf.getY(), 5.0f, graf.getHeight());
        g.setColour (col::crema.withAlpha (0.8f)); g.fillRect (x, graf.getY(), 1.2f, graf.getHeight());
    }

    // a lenda
    {
        const auto fonte = caps (8.0f, 0.16f);
        const juce::String txt[2] = { U ("A onda"), U ("O ton") };
        const juce::Colour cs[2] = { col::laranxa, col::azul };
        float ancho = 0;
        for (auto& t : txt) ancho += 22.0f + juce::GlyphArrangement::getStringWidth (fonte, upper (t)) + 18.0f;
        float x = graf.getCentreX() - ancho * 0.5f;
        const float y = r.getY() + 14.0f;
        g.setFont (fonte);
        for (int i = 0; i < 2; ++i)
        {
            g.setColour (cs[i]);
            g.fillRoundedRectangle (x, y + 5.0f, 16.0f, 3.0f, 1.5f);
            g.setColour (col::crema2);
            const float w = juce::GlyphArrangement::getStringWidth (fonte, upper (txt[i]));
            g.drawText (upper (txt[i]), juce::Rectangle<float> (x + 22.0f, y, w + 4.0f, 14.0f), juce::Justification::centredLeft, false);
            x += 22.0f + w + 18.0f;
        }
    }

    // o golpe (un LED) e o nivel de saída
    {
        paintLed (g, { meters.getCentreX(), meters.getY() + 6.0f }, led > 0.15f);
        g.setColour (col::crema3);
        drawCaps (g, U ("Golpe"), juce::Rectangle<float> (meters.getX() - 8.0f, meters.getY() + 13.0f, meters.getWidth() + 16.0f, 10.0f), juce::Justification::centred, 6.5f, 0.06f);
        auto m = meters.withTrimmedTop (30.0f).withTrimmedBottom (14.0f).withSizeKeepingCentre (18.0f, meters.getHeight() - 44.0f);
        paintWell (g, m, 4.0f);
        const float db = juce::Decibels::gainToDecibels (saida, -60.0f);
        const float alto = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f) * (m.getHeight() - 4.0f);
        if (alto > 0)
        {
            auto bar = m.reduced (3.0f, 2.0f).removeFromBottom (alto);
            g.setGradientFill (juce::ColourGradient (col::verde, 0.0f, m.getBottom(), col::ouro, 0.0f, m.getY(), false));
            g.fillRoundedRectangle (bar, 2.0f);
        }
        g.setColour (col::crema3);
        drawCaps (g, U ("Saída"), juce::Rectangle<float> (m.getX() - 14.0f, m.getBottom() + 2.0f, m.getWidth() + 28.0f, 10.0f), juce::Justification::centred, 6.5f, 0.06f);
    }
}

// ================================================================ vista principal
MainView::MainView (BBProcessor& p) : proc (p), pantalla (p)
{
    setLookAndFeel (&look);
    auto key = [this] (juce::TextButton& b, const juce::String& tip) { b.setTooltip (tip); b.setWantsKeyboardFocus (false); addAndMakeVisible (b); };

    // cabeceira: o preset, os presets propios, o A/B e desfacer
    presetBtn.getProperties().set ("gold", true);
    presetBtn.onClick = [this] { presetsMenu(); };
    key (presetBtn, U ("Os presets de fábrica, por grupos: house e techno, hip hop e trap, rock e acústicos, e electrónica."));
    prevBtn.setButtonText (U ("‹")); nextBtn.setButtonText (U ("›"));
    for (auto* b : { &prevBtn, &nextBtn, &presetsBtn, &undoBtn, &redoBtn, &funcBtn }) b->getProperties().set ("noLed", true);
    prevBtn.onClick = [this] { proc.estado.novoPaso(); proc.cargarPreset ((proc.getPreset() + bombo::NUM_PRESETS - 1) % bombo::NUM_PRESETS); proc.tocar(); refreshAll(); };
    nextBtn.onClick = [this] { proc.estado.novoPaso(); proc.cargarPreset ((proc.getPreset() + 1) % bombo::NUM_PRESETS); proc.tocar(); refreshAll(); };
    key (prevBtn, U ("O preset anterior (e tócao)."));
    key (nextBtn, U ("O preset seguinte (e tócao)."));
    presetsBtn.onClick = [this] { showPresetsMenu(); };
    key (presetsBtn, U ("Os teus presets: gardar o actual, abrir a carpeta e copiar entre A e B."));
    for (int i = 0; i < 2; ++i)
    {
        abBtn[i].setButtonText (i == 0 ? "A" : "B");
        abBtn[i].onClick = [this, i] { if (i != proc.estado.lado()) proc.estado.cambiarLado(); refreshAll(); };
        key (abBtn[i], U ("A/B: dous axustes para comparar. Cambiar de lado garda o actual; para copiar un lado no outro, no menú dos presets."));
    }
    undoBtn.getProperties().set ("curve", -1); redoBtn.getProperties().set ("curve", 1);
    undoBtn.setTitle (U ("Desfacer")); redoBtn.setTitle (U ("Refacer"));
    undoBtn.onClick = [this] { proc.undo.undo(); refreshAll(); };
    redoBtn.onClick = [this] { proc.undo.redo(); refreshAll(); };
    key (undoBtn, U ("Desfacer o último cambio. Cada clic é un paso."));
    key (redoBtn, U ("Refacer."));
    funcBtn.setButtonText (U ("Función"));
    funcBtn.onClick = [this] { functionMenu(); };
    key (funcBtn, U ("Avisos e ligazóns."));

    // o motor
    {
        auto* prm = proc.apvts.getParameter ("motor");
        motorAtt = std::make_unique<juce::ParameterAttachment> (*prm, [this, prm] (float v)
        {
            const int t = (int) std::lround (prm->convertFrom0to1 (prm->convertTo0to1 (v)));
            for (int i = 0; i < bombo::NUM_MOTORES; ++i) motorBtn[i].setToggleState (i == t, juce::dontSendNotification);
            refreshMotor();
            pantalla.repaint();
        });
        const juce::String axudas[bombo::NUM_MOTORES] = {
            U ("808: un filtro ressonante que vibra ao golpealo, coma na caixa de ritmos de 1980. Redondo, suave e, coa caída longa, o baixo do trap."),
            U ("909: un oscilador triangular redondeado, cun barrido rápido e un golpe marcado. O bombo do house e do techno."),
            U ("Sintético: unha senoide limpa, co barrido e a caída libres. Dende o minimal ata o hardstyle."),
            U ("Acústico: un parche de verdade: os modos da membrana, a maza, o abafado, o parche de diante e o casco. Do jazz ao metal.") };
        for (int i = 0; i < bombo::NUM_MOTORES; ++i)
        {
            motorBtn[i].setButtonText (U (nomesMotor[i]));
            motorBtn[i].onClick = [this, i] { motorAtt->setValueAsCompleteGesture ((float) i); proc.tocar(); };
            key (motorBtn[i], axudas[i]);
        }
        motorAtt->sendInitialUpdate();
    }
    {
        auto* prm = proc.apvts.getParameter ("seguir");
        seguirAtt = std::make_unique<juce::ParameterAttachment> (*prm, [this] (float v) { seguirBtn.setToggleState (v > 0.5f, juce::dontSendNotification); refreshMotor(); pantalla.repaint(); });
        seguirBtn.setButtonText (U ("Seguir nota"));
        seguirBtn.onClick = [this] { seguirAtt->setValueAsCompleteGesture (seguirBtn.getToggleState() ? 0.0f : 1.0f); };
        key (seguirBtn, U ("Seguir a nota: cada nota MIDI soa no seu ton (para tocar o bombo coma un baixo, coma o 808 do trap). Apagado, todas as notas soan na afinación."));
        seguirAtt->sendInitialUpdate();
    }
    {
        auto* prm = proc.apvts.getParameter ("glide");
        glideAtt = std::make_unique<juce::ParameterAttachment> (*prm, [this] (float v) { glideBtn.setToggleState (v > 0.5f, juce::dontSendNotification); refreshMotor(); });
        glideBtn.setButtonText (U ("Glide"));
        glideBtn.onClick = [this] { glideAtt->setValueAsCompleteGesture (glideBtn.getToggleState() ? 0.0f : 1.0f); };
        key (glideBtn, U ("Glide: se tocas unha nota mentres segues premendo a anterior, o ton desliza ata ela sen golpear de novo (o slide do 808). Soltando antes, golpea normal. O tempo, en Máis."));
        glideAtt->sendInitialUpdate();
    }
    tocarBtn.setButtonText (U ("Tocar"));
    tocarBtn.setTriggeredOnMouseDown (true);
    tocarBtn.onClick = [this] { proc.tocar(); };
    key (tocarBtn, U ("Un golpe, para escoitalo sen teclado (tamén o dan as notas MIDI)."));
    maisBtn.setButtonText (U ("Máis"));
    maisBtn.onClick = [this] { mostrarMais (! mais); };
    key (maisBtn, U ("Os demais controis: o barrido, o ton do golpe, o sub, a velocidade, o tempo do glide, o parche (maza, abafado e casco), a sala e o volume."));

    auto knob = [this] (Knob& kn, const char* pid, const juce::String& name, const juce::String& tip, std::function<juce::String (double)> fmt)
    {
        kn.name = name;
        kn.slider.setTooltip (tip);
        kn.att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, pid, kn.slider);
        kn.slider.textFromValueFunction = std::move (fmt);
        kn.slider.onValueChange = [this] { pantalla.repaint(); };
        // ao soltar o mando, un golpe co son novo
        kn.slider.onDragEnd = [this] { proc.tocar(); };
        addAndMakeVisible (kn);
    };
    auto pc = [] (double v) { return juce::String (juce::roundToInt (v)) + " %"; };
    knob (afinacion, "afinacion", U ("Afinación"), U ("O ton do bombo cando acaba o barrido. Na pantalla, a nota que lle corresponde (para afinalo co tema)."), hz);
    knob (caida, "caida", U ("Caída"), U ("Canto tarda o corpo en apagarse (60 dB): curto e seco ou longo coma un baixo."), tempo);
    knob (golpe, "golpe", "Golpe", U ("O clic do ataque: o que fai que o bombo se oia nos altofalantes pequenos."), pc);
    knob (caracter, "caracter", U ("Carácter"), U ("Saturación: redondea o pico e sobe a cola. Moito, e o bombo racha."), pc);
    knob (barrido, "barrido", "Barrido", U ("Canto máis alto empeza o ton: o «pum» do bombo. Nun parche, a pel estirada polo golpe (pouco)."),
          [] (double v) { return numero (v, 1) + " st"; });
    knob (tempoBarrido, "tempoBarrido", "Tempo barrido", U ("Canto tarda o ton en baixar á afinación: rápido, un golpe seco; lento, un «piu» de caixa de ritmos."),
          [] (double v) { return numero (v, v < 9.95 ? 1 : 0) + " ms"; });
    knob (tonGolpe, "tonGolpe", "Ton golpe", U ("O brillo do golpe: grave, un «tum» de feltro; agudo, o clic dunha maza de plástico."), hz);
    knob (sub, "sub", "Sub", U ("Unha senoide limpa na afinación por debaixo do corpo: peso nos graves sen cambiar o ataque."), pc);
    knob (velocidade, "velocidade", "Velocidade", U ("Canto conta a forza da nota MIDI: a 0 %, todas igual de fortes."), pc);
    knob (tempoGlide, "tempoGlide", "Tempo glide", U ("Co glide aceso: canto tarda o ton en deslizar dunha nota á seguinte."), [] (double v) { return juce::String (juce::roundToInt (v)) + " ms"; });
    knob (maza, "maza", "Maza", U ("Só no Acústico: con que se golpea. Feltro (suave e grave), madeira ou plástico (curto e cun clic)."),
          [] (double v) { return v < 33.5 ? U ("Feltro") : v < 66.5 ? U ("Madeira") : U ("Plástico"); });
    knob (abafado, "abafado", "Abafado", U ("Só no Acústico: unha almofada contra o parche. Aberto resoa; abafado é curto e con pegada."), pc);
    knob (casco, "casco", "Casco", U ("Só no Acústico: o parche de diante e a madeira do casco, que seguen soando despois do golpe."), pc);
    knob (sala, "sala", "Sala", U ("Un cuarto curto arredor do bombo (en estéreo), coma os micrófonos de ambiente."), pc);
    knob (volume, "volume", "Volume", U ("O volume de saída (despois, o limitador a −1 dBFS)."), [] (double v) { return (v >= 0.05 ? "+" : "") + numero (v, 1) + " dB"; });
    volume.slider.setDoubleClickReturnValue (true, 0.0);
    for (auto* k : { &afinacion, &caida, &golpe, &caracter }) k->slider.setMouseDragSensitivity (220);

    addAndMakeVisible (pantalla);
    newVersion.getProperties().set ("noLed", true);
    newVersion.onClick = [] { juce::URL (escoita::Actualizacions::downloadsUrl()).launchInDefaultBrowser(); };
    newVersion.setWantsKeyboardFocus (false);
    addChildComponent (newVersion);

    juce::Desktop::getInstance().addGlobalMouseListener (this);
    setSize (W, H);
    mostrarMais (false);
    refreshMotor();
    refreshAll();
    startTimerHz (30);
}

MainView::~MainView()
{
    juce::Desktop::getInstance().removeGlobalMouseListener (this);
    stopTimer();
    setLookAndFeel (nullptr);
}

// cada clic (en calquera parte) empeza un paso novo de desfacer
void MainView::mouseDown (const juce::MouseEvent&) { proc.estado.novoPaso(); }

void MainView::mostrarMais (bool s)
{
    mais = s;
    maisBtn.setToggleState (s, juce::dontSendNotification);
    pantalla.setVisible (! s);
    for (auto* k : { &barrido, &tempoBarrido, &tonGolpe, &sub, &velocidade, &tempoGlide, &maza, &abafado, &casco, &sala, &volume }) k->setVisible (s);
    repaint();
}

// o que non conta co motor de agora vese apagado: o parche fóra do Acústico e a afinación seguindo a nota
void MainView::refreshMotor()
{
    const auto a = proc.axustes();
    for (auto* k : { &maza, &abafado, &casco })
        if (k->slider.isEnabled() != (a.motor == bombo::ACUSTICO)) { k->slider.setEnabled (a.motor == bombo::ACUSTICO); k->repaint(); }
    if (afinacion.slider.isEnabled() == a.seguir) { afinacion.slider.setEnabled (! a.seguir); afinacion.repaint(); }
    if (tempoGlide.slider.isEnabled() != a.glide) { tempoGlide.slider.setEnabled (a.glide); tempoGlide.repaint(); }
}

void MainView::presetsMenu()
{
    juce::PopupMenu m;
    juce::String grupo;
    for (int i = 0; i < bombo::NUM_PRESETS; ++i)
    {
        if (grupo != bombo::PRESETS[i].grupo) { grupo = bombo::PRESETS[i].grupo; m.addSectionHeader (U (bombo::PRESETS[i].grupo)); }
        m.addItem (100 + i, U (bombo::PRESETS[i].nome), true, i == proc.getPreset() && proc.nomePreset() == U (bombo::PRESETS[i].nome));
    }
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetBtn), [this] (int r)
    {
        if (r < 100) return;
        proc.estado.novoPaso();
        proc.cargarPreset (r - 100);
        proc.tocar();
        refreshAll();
    });
}

void MainView::showPresetsMenu()
{
    juce::PopupMenu m;
    m.addSectionHeader (U ("Os teus presets"));
    proc.estado.engadirAoMenu (m, [sp = juce::Component::SafePointer<MainView> (this)] { if (sp != nullptr) sp->refreshAll(); });
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetsBtn));
}

void MainView::functionMenu()
{
    juce::PopupMenu m;
    m.addSectionHeader (U ("Función"));
    m.addItem (6, U ("Avisar das versións novas"), true, versions->enabled());
    m.addItem (7, U ("escoita.link"));
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&funcBtn), [this] (int r)
    {
        if (r == 6) versions->setEnabled (! versions->enabled());
        else if (r == 7) juce::URL ("https://www.escoita.link").launchInDefaultBrowser();
    });
}

void MainView::refreshState()
{
    for (int i = 0; i < 2; ++i)
        if (abBtn[i].getToggleState() != (i == proc.estado.lado())) abBtn[i].setToggleState (i == proc.estado.lado(), juce::dontSendNotification);
    if (undoBtn.isEnabled() != proc.undo.canUndo()) undoBtn.setEnabled (proc.undo.canUndo());
    if (redoBtn.isEnabled() != proc.undo.canRedo()) redoBtn.setEnabled (proc.undo.canRedo());
    const auto t = proc.nomePreset() + (proc.presetEditado() ? U (" *") : juce::String());
    if (presetBtn.getButtonText() != t) presetBtn.setButtonText (t);
}

void MainView::refreshAll()
{
    refreshState();
    refreshMotor();
    pantalla.repaint();
}

void MainView::timerCallback()
{
    refreshState();
    if (! mais) pantalla.tick();
    // o LED de TOCAR acéndese con cada golpe (tamén os do MIDI)
    const int g = proc.golpes.load();
    if (g != golpesVistos) { golpesVistos = g; ledTocar = 1.0f; }
    ledTocar *= 0.8f;
    if (tocarBtn.getToggleState() != (ledTocar > 0.2f)) tocarBtn.setToggleState (ledTocar > 0.2f, juce::dontSendNotification);
    const auto nv = versions->newVersion();
    if (nv.isNotEmpty() != newVersion.isVisible())
    {
        newVersion.setButtonText (U ("Hai unha versión nova: ") + nv + U (" · descargar"));
        newVersion.setVisible (nv.isNotEmpty());
        repaint();
    }
}

void MainView::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff161e33), 0.0f, 0.0f, juce::Colour (0xff0b101e), 0.0f, (float) H, false));
    g.fillAll();

    paintPanel (g, headerArea.toFloat(), 14.0f);
    {
        const auto plate = juce::Rectangle<float> ((float) headerArea.getX() + 10.0f, (float) headerArea.getY() + 6.0f, 188.0f, (float) headerArea.getHeight() - 12.0f);
        paintWell (g, plate, 9.0f);
        g.setColour (col::crema);
        g.setFont (cond (14.5f).withExtraKerningFactor (0.07f));
        g.drawText ("BARULLO BOMBO", plate.reduced (14.0f, 0.0f).withTrimmedBottom (14.0f).withTrimmedTop (4.0f), juce::Justification::centredLeft, false);
        auto rotulo = plate.reduced (14.0f, 0.0f).withTop (plate.getBottom() - 17.0f).withHeight (11.0f);
        g.setFont (caps (7.5f, 0.36f));
        const float we = juce::GlyphArrangement::getStringWidth (caps (7.5f, 0.36f), "ESCOITA");
        g.setColour (col::crema2); g.drawText ("ESCOITA", rotulo, juce::Justification::centredLeft, false);
        g.setColour (col::laranxa); g.drawText (".", rotulo.withTrimmedLeft (we - 1.5f), juce::Justification::centredLeft, false);
        g.setColour (col::crema2); g.drawText (U (" · FERRAMENTAS"), rotulo.withTrimmedLeft (we + 4.0f), juce::Justification::centredLeft, false);
    }
    auto well = [&] (juce::Component& a, juce::Component& b) { paintWell (g, a.getBounds().getUnion (b.getBounds()).expanded (4).toFloat(), 11.0f); };
    well (prevBtn, nextBtn);
    well (presetsBtn, redoBtn);

    if (mais)
    {
        paintPanel (g, screenArea.toFloat(), 12.0f);
        auto titulo = [&] (juce::Component& a, juce::Component& b, const juce::String& t)
        { groupTitle (g, juce::Rectangle<float> ((float) a.getX() + 4.0f, (float) screenArea.getY() + 18.0f, (float) (b.getRight() - a.getX()) - 8.0f, 20.0f), t); };
        titulo (barrido, tempoGlide, U ("O corpo e o golpe"));
        titulo (maza, volume, U ("O parche e a sala"));
    }

    paintPanel (g, controlsArea.toFloat(), 14.0f);
    paintWell (g, motorBtn[0].getBounds().getUnion (motorBtn[bombo::NUM_MOTORES - 1].getBounds()).expanded (4).toFloat(), 11.0f);

    const auto r = juce::Rectangle<float> (22.0f, (float) H - 22.0f, (float) W - 44.0f, 14.0f);
    g.setColour (col::crema3);
    const float we = juce::GlyphArrangement::getStringWidth (caps (8.0f, 0.3f), "ESCOITA");
    drawCaps (g, "ESCOITA", r, juce::Justification::centredLeft, 8.0f, 0.3f);
    g.setColour (col::laranxa);
    drawCaps (g, ".", r.withTrimmedLeft (we - 2.0f), juce::Justification::centredLeft, 8.0f, 0.3f);
    g.setColour (col::crema3);
    drawCaps (g, U (" Laboratorio creativo · v") + escoita::Actualizacions::currentVersion(), r.withTrimmedLeft (we + 3.0f), juce::Justification::centredLeft, 8.0f, 0.3f);
    if (! newVersion.isVisible())
        drawCaps (g, U ("Un sintetizador de bombos de Barullo"), r, juce::Justification::centred, 8.0f, 0.2f);
    drawCaps (g, U ("42°14′N 8°43′W"), r, juce::Justification::centredRight, 8.0f, 0.3f);
}

void MainView::resized()
{
    const int x0 = 16, w = W - 32;
    headerArea = { x0, 14, w, 52 };
    screenArea = { x0, 76, w, 240 };
    controlsArea = { x0, 326, w, 238 };
    {
        int x = headerArea.getX() + 210;
        const int y = headerArea.getY() + 10, h = 32;
        presetBtn.setBounds (x, y, 214, h); x += 226;
        prevBtn.setBounds (x, y, 34, h); x += 38;
        nextBtn.setBounds (x, y, 34, h); x += 48;
        presetsBtn.setBounds (x, y, 76, h); x += 80;
        abBtn[0].setBounds (x, y, 36, h); x += 40;
        abBtn[1].setBounds (x, y, 36, h); x += 44;
        undoBtn.setBounds (x, y, 32, h); x += 36;
        redoBtn.setBounds (x, y, 32, h);
        funcBtn.setBounds (headerArea.getRight() - 10 - 80, y, 80, h);
    }
    pantalla.setBounds (screenArea);
    {
        // «Máis»: dous grupos (seis e cinco), nunha fila
        const int kw = 74, kh = 150, gap = 24;
        const int ancho = 11 * kw + gap, x0g = screenArea.getCentreX() - ancho / 2, y = screenArea.getY() + 54;
        int i = 0;
        for (auto* k : { &barrido, &tempoBarrido, &tonGolpe, &sub, &velocidade, &tempoGlide }) k->setBounds (x0g + i++ * kw, y, kw, kh);
        i = 0;
        for (auto* k : { &maza, &abafado, &casco, &sala, &volume }) k->setBounds (x0g + 6 * kw + gap + i++ * kw, y, kw, kh);
    }
    {
        const int tw = 128, th = 30, ty = controlsArea.getY() + 16;
        const int tx = controlsArea.getCentreX() - (bombo::NUM_MOTORES * tw + (bombo::NUM_MOTORES - 1) * 4) / 2;
        for (int i = 0; i < bombo::NUM_MOTORES; ++i) motorBtn[i].setBounds (tx + i * (tw + 4), ty, tw, th);
        const int kw = 130, kh = 150, ky = controlsArea.getY() + 62;
        const int kx = controlsArea.getCentreX() - 2 * kw;
        int i = 0;
        for (auto* k : { &afinacion, &caida, &golpe, &caracter }) k->setBounds (kx + i++ * kw, ky, kw, kh);
        tocarBtn.setBounds (controlsArea.getX() + 22, ky + kh / 2 - 58, 118, 32);
        seguirBtn.setBounds (controlsArea.getX() + 22, ky + kh / 2 - 18, 118, 32);
        glideBtn.setBounds (controlsArea.getX() + 22, ky + kh / 2 + 22, 118, 32);
        maisBtn.setBounds (controlsArea.getRight() - 22 - 118, ky + kh / 2 - 20, 118, 32);
    }
    newVersion.setBounds (W / 2 - 170, H - 27, 340, 22);
}
} // namespace ui

// ================================================================ xanela (escalable)
BBEditor::BBEditor (BBProcessor& p) : AudioProcessorEditor (&p), proc (p), main (p)
{
    addAndMakeVisible (main);
    setResizable (true, true);
    setResizeLimits (ui::MainView::W * 6 / 10, ui::MainView::H * 6 / 10, ui::MainView::W * 2, ui::MainView::H * 2);
    if (auto* c = getConstrainer()) c->setFixedAspectRatio ((double) ui::MainView::W / ui::MainView::H);
    float s = proc.uiScale;
    if (auto* d = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        const auto ua = d->userArea;
        const float fit = juce::jmin ((float) (ua.getWidth() - 40) / (float) ui::MainView::W, (float) (ua.getHeight() - 90) / (float) ui::MainView::H);
        if (s > fit && fit > 0.4f) s = fit;
    }
    setSize (juce::roundToInt (ui::MainView::W * s), juce::roundToInt (ui::MainView::H * s));
}

void BBEditor::resized()
{
    const float s = (float) getWidth() / (float) ui::MainView::W;
    main.setBounds (0, 0, ui::MainView::W, ui::MainView::H);
    main.setTransform (juce::AffineTransform::scale (s));
    proc.uiScale = s;
}
