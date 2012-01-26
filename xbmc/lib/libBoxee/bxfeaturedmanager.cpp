//
// C++ Implementation: bxfeaturedmanager
//
// Description:
//
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "bxfeaturedmanager.h"
#include "boxee.h"
#include "bxconfiguration.h"
#include "bxexceptions.h"
#include "logger.h"
#include "GUIWindowManager.h"
#include "GUIUserMessages.h"
#include "GUIWindowBoxeeMain.h"
#include "../../Application.h"

namespace BOXEE
{

BXFeaturedManager::BXFeaturedManager()
{
  m_featuredListGuard = SDL_CreateMutex();
  m_featuredList.Clear();
}

BXFeaturedManager::~BXFeaturedManager()
{
  SDL_DestroyMutex(m_featuredListGuard);
  
}

bool BXFeaturedManager::Initialize()
{
  m_featuredList.Clear();
  
  return true;
}

bool BXFeaturedManager::UpdateFeaturedList(unsigned long executionDelayInMS, bool repeat)
{
  RequestFeaturedListFromServerTask* reqFeaturedListTask = new RequestFeaturedListFromServerTask(this,executionDelayInMS,repeat);

  if(reqFeaturedListTask)
  {
    if(executionDelayInMS == 0)
    {
      // In case the request is for immediate execution -> Set the status of the list to NOT LOADED in order 
      // for get function to wait for update

      LockFeaturedList();

      m_featuredList.SetLoaded(false);
      
      UnLockFeaturedList();      
    }
    
    Boxee::GetInstance().GetBoxeeScheduleTaskManager().AddScheduleTask(reqFeaturedListTask);
    return true;
  }
  else
  {
    LOG(LOG_LEVEL_ERROR,"CBoxeeClientServerComManager::AddRequestForFeaturedListTask - FAILED to allocate RequestFeaturedListFromServerTask (feat)");
    return false;
  }
}

void BXFeaturedManager::LockFeaturedList()
{
  SDL_LockMutex(m_featuredListGuard);
}

void BXFeaturedManager::UnLockFeaturedList()
{
  SDL_UnlockMutex(m_featuredListGuard);
}

void BXFeaturedManager::CopyFeaturedList(const BXBoxeeFeed& featuredList)
{
  LockFeaturedList();
  
  m_featuredList = featuredList;
  m_featuredList.SetLoaded(true);
  
  UnLockFeaturedList();
}

void BXFeaturedManager::SetFeaturedListIsLoaded(bool isLoaded)
{
  LockFeaturedList();

  m_featuredList.SetLoaded(isLoaded);

  UnLockFeaturedList();
}

bool BXFeaturedManager::GetFeaturedList(BXBoxeeFeed& featuredList)
{
  LOG(LOG_LEVEL_DEBUG,"BXFeaturedManager::GetFeaturedList - Enter function (shares)");

  if(BOXEE::Boxee::GetInstance().IsInOfflineMode())
  {
    LOG(LOG_LEVEL_DEBUG,"BXFeaturedManager::GetFeaturedList - In offline mode. Going to return FALSE (feat)");
    return false;
  }

  bool featuredListWasLoaded = false;
  
  LockFeaturedList();

  featuredListWasLoaded = m_featuredList.IsLoaded();
  
  while (!featuredListWasLoaded)
  {
    // FeaturedList ISN'T loaded yet -> UnLock the FeaturedList and wait for it to load
    
    UnLockFeaturedList();

    LOG(LOG_LEVEL_DEBUG,"BXFeaturedManager::GetFeaturedList - FeaturedList is not loaded yet. Going to try again in [%dms] (feat)",DELAY_FOR_CHECK_FEED_LOADED);

    SDL_Delay(DELAY_FOR_CHECK_FEED_LOADED);

    LockFeaturedList();
      
    featuredListWasLoaded = m_featuredList.IsLoaded();      
  }

  featuredList = m_featuredList;

  UnLockFeaturedList();

  LOG(LOG_LEVEL_DEBUG,"BXFeaturedManager::GetFeaturedList - Exit function. After set [FeaturedListSize=%d] (feat)",featuredList.GetNumOfActions());

  return true;
}

/////////////////////////////////////////////////
// RequestFeaturedListFromServerTask functions //
/////////////////////////////////////////////////

BXFeaturedManager::RequestFeaturedListFromServerTask::RequestFeaturedListFromServerTask(BXFeaturedManager* taskHandler, unsigned long executionDelayInMS, bool repeat):BoxeeScheduleTask("RequestFeaturedList",executionDelayInMS,repeat)
{
  m_taskHandler = taskHandler;
}

BXFeaturedManager::RequestFeaturedListFromServerTask::~RequestFeaturedListFromServerTask()
{
  
}

void BXFeaturedManager::RequestFeaturedListFromServerTask::DoWork()
{
  LOG(LOG_LEVEL_DEBUG,"RequestFeaturedListFromServerTask::DoWork - Enter function (feat)");
  
  bool isLoaded = m_taskHandler->m_featuredList.IsLoaded();
  int activeWindowId = g_windowManager.GetActiveWindow();
  bool isConnectedToInternet = g_application.IsConnectedToInternet();

  if (!isConnectedToInternet || (isLoaded && activeWindowId != WINDOW_HOME))
  {
    // set loaded to true so Get() functions won't wait forever
    m_taskHandler->SetFeaturedListIsLoaded(true);

    LOG(LOG_LEVEL_DEBUG,"BXFeaturedManager::RequestFeaturedListFromServerTask::DoWork - [isConnectedToInternet=%d] or [activeWindowId=%d] which isn't WINDOW_HOME -> Exit function (feat)",isConnectedToInternet,activeWindowId);
    return;
  }

  BXBoxeeFeed featuredList;
  
  featuredList.SetCredentials(BOXEE::Boxee::GetInstance().GetCredentials());
  featuredList.SetVerbose(BOXEE::Boxee::GetInstance().IsVerbose());

  std::string strUrl = BXConfiguration::GetInstance().GetURLParam("Boxee.FeaturedListUrl","http://app.boxee.tv/api/get_featured");

  featuredList.LoadFromURL(strUrl);

  long lastRetCode = featuredList.GetLastRetCode();

  LOG(LOG_LEVEL_DEBUG,"RequestFeaturedListFromServerTask::DoWork - After LoadFromURL. [lastRetCode=%d][isLoaded=%d] (feat)",lastRetCode,isLoaded);

  if (!isLoaded || lastRetCode == 200)
  {
    ////////////////////////////////////////////
    // copy return result from the server if: //
    // a)  featuredList isn't loaded          //
    // b) the server returned 200             //
    ////////////////////////////////////////////

    m_taskHandler->CopyFeaturedList(featuredList);

    LOG(LOG_LEVEL_DEBUG,"RequestFeaturedListFromServerTask::DoWork - After copy going to send GUI_MSG_UPDATE to LIST_FEATURES (feat)");

    int activeWindow = g_windowManager.GetActiveWindow();

    if (activeWindow == WINDOW_HOME)
    {
      CGUIMessage refreshHomeFeaturedList(GUI_MSG_UPDATE, WINDOW_HOME, LIST_FEATURES);
      g_windowManager.SendThreadMessage(refreshHomeFeaturedList);
    }
  }

  LOG(LOG_LEVEL_DEBUG,"RequestFeaturedListFromServerTask::DoWork - Exit function (feat)");

  return;
}

}
