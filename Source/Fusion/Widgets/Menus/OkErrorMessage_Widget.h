// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"
#include "OkErrorMessage_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UOkErrorMessage_Widget : public UMasterWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	UFUNCTION()
	void OnClickedErrorOkButton();
	
	UFUNCTION()
	void OnRep_ErrorText();

	UPROPERTY(replicatedUsing = OnRep_ErrorText)
	FText ErrorText;

protected:


	UPROPERTY(meta = (BindWidget))
	UButton* ErrorOkButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ErrorTextBlock;




};
