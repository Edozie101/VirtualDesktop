#include "Rift.h"

#include "../ibex.h"

OVR::Ptr<OVR::DeviceManager>	pManager;
OVR::Ptr<OVR::HMDDevice>	pHMD;
OVR::Ptr<OVR::SensorDevice>	pSensor;
OVR::SensorFusion		FusionResult;
OVR::HMDInfo			Info;
bool				InfoLoaded = false;
bool				riftConnected = false;

int riftX = 0;
int riftY = 0;
int riftResolutionX = 0;
int riftResolutionY = 0;

void initRift() {
  OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));

  pManager = *OVR::DeviceManager::Create();

  //pManager->SetMessageHandler(this);

  pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();

  if (pHMD)
    {
      pSensor = *pHMD->GetSensor();

      InfoLoaded = pHMD->GetDeviceInfo(&Info);

      strncpy(Info.DisplayDeviceName, RiftMonitorName, 32);

      RiftDisplayId = Info.DisplayId;

      EyeDistance = Info.InterpupillaryDistance;
      DistortionK[0] = Info.DistortionK[0];
      DistortionK[1] = Info.DistortionK[1];
      DistortionK[2] = Info.DistortionK[2];
      DistortionK[3] = Info.DistortionK[3];
    }
  else
    {
      pSensor = *pManager->EnumerateDevices<OVR::SensorDevice>().CreateDevice();
    }

  if (pSensor)
    {
      FusionResult.AttachToSensor(pSensor);
      FusionResult.SetPredictionEnabled(true);
      float motionPred = FusionResult.GetPredictionDelta(); // adjust in 0.01 increments
      if(motionPred < 0) motionPred = 0;
      FusionResult.SetPrediction(motionPred);

      if(InfoLoaded) {
	riftConnected = true;

	riftX = Info.DesktopX;
	riftY = Info.DesktopY;

	riftResolutionX = Info.HResolution;
	riftResolutionY = Info.VResolution;
      }
    }

#ifdef WIN32
  getRiftDisplay();
#endif
}
void cleanUpRift() {
  pSensor.Clear();
  pManager.Clear();

  OVR::System::Destroy();
}
