#include "Rift.h"

#include "../ibex.h"

OVR::Ptr<OVR::DeviceManager>	pManager;
OVR::Ptr<OVR::HMDDevice>	pHMD;
OVR::Ptr<OVR::SensorDevice>	pSensor;
OVR::SensorFusion*		pFusionResult = 0;
OVR::HMDInfo			Info;
bool				InfoLoaded = false;
bool				riftConnected = false;

int riftX = 0;
int riftY = 0;
int riftResolutionX = 0;
int riftResolutionY = 0;

void initRift() {
  OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));

  pFusionResult = new OVR::SensorFusion();
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
        for(int i = 0; i < 4; ++i) {
            DistortionK[i] = Info.DistortionK[i];
            DistortionChromaticAberration[i] = Info.ChromaAbCorrection[i];
        }
    }
  else
    {
      pSensor = *pManager->EnumerateDevices<OVR::SensorDevice>().CreateDevice();
    }

  if (pSensor)
    {
      pFusionResult->AttachToSensor(pSensor);
      pFusionResult->SetPredictionEnabled(true);
      float motionPred = pFusionResult->GetPredictionDelta(); // adjust in 0.01 increments
      if(motionPred < 0) motionPred = 0;
      pFusionResult->SetPrediction(motionPred);

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
    pHMD.Clear();
    pManager.Clear();

    delete pFusionResult;

    OVR::System::Destroy();
}

double orientationRift[16] = {1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1};
double *getRiftOrientation() {
    if(pFusionResult!= 0 && !pFusionResult->IsAttachedToSensor()) return orientationRift;
    OVR::Quatf quaternion = pFusionResult->GetPredictedOrientation();//FusionResult.GetOrientation();
    
    //float yaw, pitch, roll;
    //quaternion.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
    
    //std::cout << " Yaw: " << RadToDegree(yaw) <<
    //	", Pitch: " << RadToDegree(pitch) <<
    //	", Roll: " << RadToDegree(roll) << std::endl;
    
    OVR::Matrix4f hmdMat(quaternion);
    for(int i = 0; i < 16; ++i) {
        orientationRift[i] = ((float*)hmdMat.M)[i];
    }
    return orientationRift;
}
