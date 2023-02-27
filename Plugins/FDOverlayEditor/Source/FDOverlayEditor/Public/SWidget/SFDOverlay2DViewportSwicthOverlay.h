#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBoxPanel.h"
#include "Input/Reply.h"

DECLARE_DELEGATE_RetVal(
FReply,
FOnSwitchOverlayAdd)
DECLARE_DELEGATE_RetVal(
FReply,
FOnSwitchOverlaySub)

class FDOVERLAYEDITOR_API SFDSwitchOverlayBox : public SBoxPanel
{
	SLATE_DECLARE_WIDGET(SFDSwitchOverlayBox, SBoxPanel)
public:
	class FSlot : public SBoxPanel::TSlot<FSlot>
	{
	public:
		SLATE_SLOT_BEGIN_ARGS(FSlot, SBoxPanel::TSlot<FSlot>)
			SLATE_SLOT_END_ARGS()

			void Construct(const FChildren& SlotOwner, FSlotArguments&& InArgs)
		{
			SBoxPanel::TSlot<FSlot>::Construct(SlotOwner, MoveTemp(InArgs));
		}

	};

	SLATE_BEGIN_ARGS(SFDSwitchOverlayBox)
	{}
	SLATE_SLOT_ARGUMENT(SFDSwitchOverlayBox::FSlot, Slots)
		/** Optional icon to display in the button. */
		SLATE_ATTRIBUTE(const FSlateBrush*, Icon)
		/** Call when the +/- is clicked. */
		SLATE_EVENT(FOnSwitchOverlayAdd, OnSwitchOverlayAdd)
		SLATE_EVENT(FOnSwitchOverlaySub, OnSwitchOverlaySub)
	SLATE_END_ARGS()

		FORCENOINLINE SFDSwitchOverlayBox()
		: SBoxPanel(Orient_Horizontal)
	{
		SetCanTick(false);
		bCanSupportFocus = false;
	}

	/**
	 * Construct this widget
	 *
	 * @param	InArgs	The declaration data for this widget
	 */
	void Construct(const FArguments& InArgs);

private:

	static FSlot::FSlotArguments Slot()
	{
		return FSlot::FSlotArguments(MakeUnique<FSlot>());
	}
};