#include "Tools/FDOverlayStyle.h"

#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyleMacros.h"
#include "Interfaces/IPluginManager.h"

FName FFDOverlayStyle::StyleName("FDOverlayStyle");

FFDOverlayStyle::FFDOverlayStyle()
	:FSlateStyleSet(StyleName)
{
	const FVector2D IconSize(16.0f, 16.0f);
	const FVector2D ToolbarIconSize(20.0f, 20.0f);

	SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("FDOverlayEditor/Content/Icons"));
	SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));

	// Viewport icons 
	Set("FDOverlay.XChannel", new IMAGE_BRUSH_SVG("XChannel", ToolbarIconSize));
	Set("FDOverlay.YChannel", new IMAGE_BRUSH_SVG("YChannel", ToolbarIconSize));
	Set("FDOverlay.ZChannel", new IMAGE_BRUSH_SVG("ZChannel", ToolbarIconSize));
	Set("FDOverlay.WChannel", new IMAGE_BRUSH_SVG("WChannel", ToolbarIconSize));
	Set("FDOverlay.Compact",  new IMAGE_BRUSH_SVG("Compact",  ToolbarIconSize));
	Set("FDOverlay.Iterable", new IMAGE_BRUSH_SVG("Iterable", ToolbarIconSize));
	Set("FDOverlay.Exploded", new IMAGE_BRUSH_SVG("Exploded", ToolbarIconSize));
	// Top toolbar icons
	Set("FDOverlay.ApplyChanges", new CORE_IMAGE_BRUSH_SVG("Starship/Common/Apply", ToolbarIconSize));

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FFDOverlayStyle::~FFDOverlayStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}

FFDOverlayStyle& FFDOverlayStyle::Get()
{
	static FFDOverlayStyle Inst;
	return Inst;
}
