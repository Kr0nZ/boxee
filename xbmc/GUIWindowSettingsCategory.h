#pragma once

/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "GUIWindow.h"
#include "SettingsControls.h"
#include "GUISettings.h"
#include "HalServices.h"
#include "FileItem.h"

#define WIRED_INTERFACE_ID               0
#define WIRELESS_INTERFACE_ID            1

class CGUIWindowSettingsCategory :
      public CGUIWindow
{
public:
  CGUIWindowSettingsCategory(void);
  virtual ~CGUIWindowSettingsCategory(void);
  virtual bool OnMessage(CGUIMessage &message);
  virtual bool OnAction(const CAction &action);
  virtual void Render();
  virtual int GetID() const { return CGUIWindow::GetID() + m_iScreen; };

  // static function as it's accessed elsewhere
  static void FillInVisualisations(CSetting *pSetting, int iControlID);

  void SetNewLanguage(CStdString newLanguage);
  CStdString GetNewLanguage();
  bool LoadNewLanguage();

  bool TestAndSetNewResolution(RESOLUTION newResolution = RES_INVALID);
  void SetTempUnit(const CStdString& tempUnit);
  void SetTimeFormat(const CStdString& timeFormat);

#ifdef HAS_EMBEDDED
  CStdString GetCountryByCode(const CStdString& countryCode);
  bool IsTimezoneCityExist(const CStdString& cityName);
#endif

protected:
  virtual void OnInitWindow();
//  virtual void OnClose();

  static void GetLanguages(std::vector<CStdString> &vecLanguage);
  
  void CheckNetworkSettings();
  void CheckRuntimeSettings();
  void FillInSubtitleHeights(CSetting *pSetting);
  void FillInSubtitleFonts(CSetting *pSetting);
  void FillInCharSets(CSetting *pSetting);
  void FillInSkinFonts(CSetting *pSetting);
  void FillInSkins(CSetting *pSetting);
  void FillInSoundSkins(CSetting *pSetting);
  void FillInLanguages(CSetting *pSetting);
  void FillInTimezoneCountries(CSetting *pSetting);
  void FillInTimezoneCities(CSetting *pSetting);
  void FillInKeyboards(CSetting *pSetting, int nKeyboard);
  void FillInVoiceMasks(DWORD dwPort, CSetting *pSetting);   // Karaoke patch (114097)
  void FillInVoiceMaskValues(DWORD dwPort, CSetting *pSetting); // Karaoke patch (114097)
  void FillInResolutions(CSetting *pSetting, bool playbackSetting);
  void FillInVSyncs(CSetting *pSetting);
  void FillInMode3D(CSetting *pSetting);
  void FillInDeinterlacingPolicy(CSetting *pSetting);
  void FillInShowsDefault(CSetting *pSetting);
  void FillInMoviesDefault(CSetting *pSetting);
  void FillInAppsDefault(CSetting *pSetting);
  void SetShowStarter(CSetting *pSetting);
  void FillInScreenSavers(CSetting *pSetting);
  void FillInRegions(CSetting *pSetting);
  void FillInFTPServerUser(CSetting *pSetting);
  void FillInStartupWindow(CSetting *pSetting);
  void FillInLogLevels(CSetting *pSetting);
  void FillInViewModes(CSetting *pSetting, int windowID);
  void FillInSortMethods(CSetting *pSetting, int windowID);
  bool SetFTPServerUserPass();

  void FillInSkinThemes(CSetting *pSetting);
  void FillInSkinColors(CSetting *pSetting);
  
  void FillInNetworkInterfaces(CSetting *pSetting);
  void NetworkInterfaceChanged(void);
  void ApplyPersonalFeeds();


  void FillInScrapers(CGUISpinControlEx *pControl, const CStdString& strSelected, const CStdString& strContent);

  void FillInAudioDevices(CSetting* pSetting, bool Passthrough = false);
  void FillInWeatherPlugins(CGUISpinControlEx *pControl, const CStdString& strSelected);
  void FillInTimeFormat(CSetting *pSetting);
  void FillInTempScale(CSetting *pSetting);

  void FillInHDMIOutput(CSetting *pSetting);
  void FillInHDMIPixelDepth(CSetting *pSetting);

  void FillInPrefferedLanguage(CSetting *pSetting, CStdString id);

#ifdef HAS_EMBEDDED
  void UpdateDownloadStatus();
  void CheckTimezoneSettings();
  void ReadTimezoneData();
  void ReadTimezoneCountries();
  void ReadTimezoneCities();
#endif

#ifdef HAS_DVB
  void UpdateOtaStatus();
#endif

  void UpdateKeyboard(int nKeyboard, CBaseSettingControl *pSettingControl);
  
  virtual void SetupControls();
  void CreateSettings();
  void CreateRunTimeSettings();
  void UpdateSettings();
  void UpdateRealTimeSettings();
  void CheckForUpdates();
  void FreeSettingsControls();

  void NetflixClear();
  void VuduClear();
  void SpotifyClear();


  virtual void FreeControls();
  virtual void OnClick(CBaseSettingControl *pSettingControl);
  virtual void OnSettingChanged(CBaseSettingControl *pSettingControl);
  void AddSetting(CSetting *pSetting, float width, int &iControlID);
  CBaseSettingControl* GetSetting(const CStdString &strSetting);

  void JumpToSection(int windowID, const CStdString &section);
  void JumpToPreviousSection();
#if defined(_LINUX) && !defined(__APPLE__)
  void GenSoundLabel(const CStdString& device, const CStdString& card, const int labelValue, CGUISpinControlEx* pControl, bool Passthrough);
#endif //defined(_LINUX) && !defined(__APPLE__)

  std::vector<CBaseSettingControl *> m_vecSettings;
  int m_iSection;
  int m_iScreen;
  RESOLUTION m_NewResolution;
  vecSettingsCategory m_vecSections;
  CGUISpinControlEx *m_pOriginalSpin;
  CGUIRadioButtonControl *m_pOriginalRadioButton;
  CGUIButtonControl *m_pOriginalCategoryButton;
  CGUIButtonControl *m_pOriginalButton;
  CGUIEditControl *m_pOriginalEdit;
  CGUIImage *m_pOriginalImage;
  CFileItemList m_availableLanguages;
  // Network settings
  int m_iNetworkAssignment;
  CStdString m_strNetworkIPAddress;
  CStdString m_strNetworkSubnet;
  CStdString m_strNetworkGateway;
  CStdString m_strNetworkDNS;
  // look + feel settings (for delayed loading)
  CStdString m_strNewSkinFontSet;
  CStdString m_strNewSkin;
  CStdString m_strNewLanguage;
  CStdString m_strNewSkinTheme;
  CStdString m_strNewSkinColors;

  CStdString m_strErrorMessage;

  CStdString m_strOldTrackFormat;
  CStdString m_strOldTrackFormatRight;

  bool m_bBoxeeRemoteEnabled;

#ifdef HAS_EMBEDDED
  CStdString m_strUpdateStatus;
  int  m_iUpdateSection;
  bool m_bDownloadingUpdate;
  bool m_bSmbServerEnable;
  CStdString  m_smbPassword;
  CStdString  m_smbWorkgroup;
  CStdString  m_smbHostname;
  CStdString m_tzChosenCountryName;
  CStdString m_tzChosenCityName;
  bool m_needToLoadTimezoneCitiesControl;
  std::map<CStdString, CStdString> m_tzCodeToCountryMap;
  std::map<CStdString, CStdString> m_tzCountryToCodeMap;
  std::multimap<CStdString, CStdString> m_tzCodeToCityMultimap;
  std::map<CStdString, CStdString> m_tzCityToPathMap;
  int m_iNetworkInterface;
#endif

#ifdef HAS_DVB
  int m_iOtaScanSection;
#endif

  // state of the window saved in JumpToSection()
  // to get to the previous settings screen when
  // using JumpToPreviousSection()
  int m_iSectionBeforeJump;
  int m_iControlBeforeJump;
  int m_iWindowBeforeJump;

  bool m_returningFromSkinLoad; // true if we are returning from loading the skin
  bool m_containRuntimeCategory;

#ifdef HAS_BOXEE_HAL
  CHalWirelessAuthType m_networkEnc;
  CStdString m_networkKey;

  CStdString m_device;
  CStdString m_deviceSn;
  CStdString m_ethernetMacAddress;
  CStdString m_wirelessMacAddress;
  CStdString m_firmware;
#endif
};

#ifdef HAS_BOXEE_HAL
class CWaitNetworkUpBG : public IRunnable
{
public:
  CWaitNetworkUpBG(int ifaceId);
  virtual ~CWaitNetworkUpBG() {}
  virtual void Run();

  static bool IsNetworkUp(int ifaceId);

private:
  int m_ifaceId;
};

class CVpnOperationBG : public IRunnable
{
public:
  CVpnOperationBG();
  virtual ~CVpnOperationBG() {}
  virtual void Run();
};
#endif

