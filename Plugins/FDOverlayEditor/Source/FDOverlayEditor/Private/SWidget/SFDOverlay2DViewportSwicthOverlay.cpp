// Copyright HandsomeCheese. All Rights Reserved.
#include "SWidget/SFDOverlay2DViewportSwicthOverlay.h"

#include "SPrimaryButton.h"
#include "Templates\SharedPointer.h"

#define LOCTEXT_NAMESPACE "SFDOverlay2DViewportSwicthOverlay"

SLATE_IMPLEMENT_WIDGET(SFDSwitchOverlayBox)
void SFDSwitchOverlayBox::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SFDSwitchOverlayBox::Construct(const SFDSwitchOverlayBox::FArguments& InArgs)
{
	SFDSwitchOverlayBox::FArguments Args = MoveTemp(SFDSwitchOverlayBox::FArguments()
			+SFDSwitchOverlayBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(FMargin(0.0f, 0.0f, 0.f, 15.f))
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::Get().GetBrush("EditorViewport.OverlayBrush"))
					.Padding(8.f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(FMargin(0.f, 0.f, 8.f, 0.f))
						[
							SNew(SImage)
							.Image(InArgs._Icon)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(0.0, 0.f, 2.f, 0.f))
						[
							SNew(SPrimaryButton)
							.Text(LOCTEXT("SwitchOverlay", "-"))
							.ToolTipText(LOCTEXT("SwitchOverlayTooltip", "Subtract the ID of the UV-Display"))
							.OnClicked(InArgs._OnSwitchOverlaySub)

						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(0.0, 0.f, 2.f, 0.f))
						[
							SNew(SPrimaryButton)
							.Text(LOCTEXT("SwitchOverlay", "+"))
							.ToolTipText(LOCTEXT("SwitchOverlayTooltip", "Add the ID of the UV-Display"))
							.OnClicked(InArgs._OnSwitchOverlayAdd)
						]
					]
				]
			]
		);

		Children.Reserve(Args._Slots.Num());

		for (const FSlot::FSlotArguments& Arg : Args._Slots)
		{
			// Because we want to override the AutoWidth, the base class doesn't exactly have the same parent.
			//We are casting from parent to child to a different parent to prevent a reinterpret_cast
			const FSlotBase::FSlotArguments& ChilSlotArgument = static_cast<const FSlotBase::FSlotArguments&>(Arg);
			const SBoxPanel::FSlot::FSlotArguments& BoxSlotArgument = static_cast<const SBoxPanel::FSlot::FSlotArguments&>(ChilSlotArgument);
			// Because InArgs is const&, we need to do some hacking here. That would need to changed in the future.
			//The Slot has a unique_ptr, it cannot be copied. Anyway, previously, the Children.Add(), was wrong if we added the same slot twice.
			//Because of that, it doesn't matter if we steal the slot from the FArguments.
			Children.AddSlot(MoveTemp(const_cast<SBoxPanel::FSlot::FSlotArguments&>(BoxSlotArgument)));
		}
	

}



#undef LOCTEXT_NAMESPACE