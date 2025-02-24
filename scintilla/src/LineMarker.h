// Scintilla source code edit control
/** @file LineMarker.h
 ** Defines the look of a line marker in the margin .
 **/
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#pragma once

namespace Scintilla::Internal {

class XPM;
class RGBAImage;

typedef void (*DrawLineMarkerFn)(Surface *surface, PRectangle rcWhole, const Font *fontForCharacter, int part, Scintilla::MarginType marginStyle, const void *lineMarker);

struct LineMarkerPod {
	Scintilla::MarkerSymbol markType = Scintilla::MarkerSymbol::Circle;
	ColourRGBA fore = black;
	ColourRGBA back = white;
	ColourRGBA backSelected = ColourRGBA(0xff, 0x00, 0x00);
	Scintilla::Layer layer = Scintilla::Layer::Base;
	XYPOSITION strokeWidth = 1.0f;
	/** Some platforms, notably PLAT_CURSES, do not support Scintilla's native
	 * Draw function for drawing line markers. Allow those platforms to override
	 * it instead of creating a new method(s) in the Surface class that existing
	 * platforms must implement as empty. */
	DrawLineMarkerFn customDraw = nullptr;
	constexpr LineMarkerPod() noexcept = default;
};

/**
 */
class LineMarker final : public LineMarkerPod {
public:
	enum class FoldPart {
		undefined, head, body, tail, headWithTail
	};

	std::unique_ptr<XPM> pxpm;
	std::unique_ptr<RGBAImage> image;

	LineMarker() noexcept = default;
	LineMarker(const LineMarker &other);
	LineMarker(LineMarker &&) noexcept = default;
	LineMarker &operator=(const LineMarker& other);
	LineMarker &operator=(LineMarker&&) noexcept = default;
	~LineMarker() = default;

	void SetXPM(const char *textForm);
	void SetXPM(const char *const *linesForm);
	void SCICALL SetRGBAImage(Point sizeRGBAImage, float scale, const unsigned char *pixelsRGBAImage);
	void AlignedPolygon(Surface *surface, const Point *pts, size_t npts) const;
	void SCICALL Draw(Surface *surface, PRectangle rcWhole, const Font *fontForCharacter, FoldPart part, Scintilla::MarginType marginStyle) const;
	void SCICALL DrawFoldingMark(Surface *surface, PRectangle rcWhole, FoldPart part) const;

private:
	void CopyImage(const LineMarker &other);
};

}
