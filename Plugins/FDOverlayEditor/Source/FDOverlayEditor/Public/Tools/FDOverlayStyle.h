#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

/**
 * Slate style set for FDOverlayEditor
 */
class FDOVERLAYEDITOR_API FFDOverlayStyle
	: public FSlateStyleSet
{
public:
	static FName StyleName;

	/** Access the singleton instance for this style set */
	static FFDOverlayStyle& Get();

private:

	FFDOverlayStyle();
	~FFDOverlayStyle();
};