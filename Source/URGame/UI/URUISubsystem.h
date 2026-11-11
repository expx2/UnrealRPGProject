// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "URUISubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCinematicModeChangedDelegate, const bool, bCinematicMode);

enum class EPopupType : uint8;
/**
 * 
 */
UCLASS(BlueprintType, Category="UR|UI")
class URGAME_API UURUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	UURUISubsystem();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable)
	void TestFunction();

	/*
	 * PlayerController 시작 시 호출되어 기본 UI 생성
	 */
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	void InitRootWidget();
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	bool HasRootWidget();
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	void AddSceneHUD(UUserWidget* Widget);
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	void AddGameWidget(UUserWidget* Widget);
	
	UFUNCTION(BlueprintPure, Category="UR|UI")
	class UURUIRootWidget* GetRootWidget();
	
	// 토스트 메세지
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	void ToastMessage(const FText& Message);
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	void AbilityFailedToastMessage(const struct FGameplayTagContainer& FailureReason);
	
	// 팝업 여닫기
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	class UURPopupWidget* OpenPopup(EPopupType PopupType);
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	class UURPopupWidget* GetPopup(EPopupType PopupType);
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	void ClosePopup(EPopupType PopupType);
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	void CloseAllPopups();
	
	void AddPopup(UURPopupWidget* Popup);
	
	bool RemovePopup(UURPopupWidget* Popup);
	
	void UpdateInputModeByPopup();
	
	void UpdateCinematicMode(bool bInCinematicMode);
	
	UPROPERTY(BlueprintAssignable, Category="UR|UI");
	FCinematicModeChangedDelegate OnCinematicModeChanged;

protected:
	void ClearRootWidget();
	
	UPROPERTY(BlueprintReadOnly, Category="UR|UI")
	TObjectPtr<class UURPopupWidgetMapping> Mapping;
	
	UPROPERTY(BlueprintReadOnly, Category="UR|UI")
	TSubclassOf<class UURUIRootWidget> RootWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, Category="UR|UI")
	TObjectPtr<class UURUIRootWidget> RootWidget;
	
	UPROPERTY(BlueprintReadOnly, Category="UR|UI")
	TArray<TObjectPtr<UURPopupWidget>> OpenPopups;
};
