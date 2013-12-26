#include "Rift.h"

#include "../ibex.h"

OVR::Ptr<OVR::DeviceManager>	pManager = 0;
OVR::Ptr<OVR::HMDDevice>	pHMD = 0;
OVR::Util::Render::StereoConfig stereo;
OVR::Ptr<OVR::SensorDevice>	pSensor = 0;
OVR::SensorFusion*		pFusionResult = 0;
OVR::HMDInfo			Info;
bool				InfoLoaded = false;
bool				riftConnected = false;
float               renderScale;
bool                renderScaleChanged = false;

OVR::Util::Render::StereoEyeParams  leftEye  = stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
OVR::Util::Render::StereoEyeParams  rightEye = stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);

// Left eye rendering parameters
OVR::Util::Render::Viewport         leftVP         = leftEye.VP;
OVR::Matrix4f                       leftProjection = leftEye.Projection;
OVR::Matrix4f                       leftViewAdjust = leftEye.ViewAdjust;

// Right eye rendering parameters
OVR::Util::Render::Viewport         rightVP         = leftEye.VP;
OVR::Matrix4f                       rightProjection = leftEye.Projection;
OVR::Matrix4f                       rightViewAdjust = leftEye.ViewAdjust;


int riftX = 0;
int riftY = 0;
int riftResolutionX = 0;
int riftResolutionY = 0;

#ifdef WIN32
BOOL CALLBACK MonitorEnumProc(
  _In_  HMONITOR hMonitor,
  _In_  HDC hdcMonitor,
  _In_  LPRECT lprcMonitor,
  _In_  LPARAM dwData
) {
	MONITORINFOEX lpmi;
	MONITORINFO l;
	lpmi.cbSize = sizeof(MONITORINFOEX);
	char name[32];
	DISPLAY_DEVICE d;
	
	if(GetMonitorInfo(hMonitor, &lpmi)) {
		
		//strncpy(name, (const char*)lpmi.szDevice, 32);
		//if(strstr(name, "Rift") != NULL) {
		if( ((lpmi.rcMonitor.right-lpmi.rcMonitor.left) == riftResolutionX) &&
					((lpmi.rcMonitor.bottom-lpmi.rcMonitor.top) == riftResolutionY)) {
			riftX = lpmi.rcMonitor.left;
			riftY = lpmi.rcMonitor.top;
		}
	}
	return true;
}

static void getRiftDisplay() {
	if(InfoLoaded) {
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
	}
}
#endif

void initRift() {
    OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
    
    pFusionResult = new OVR::SensorFusion();
    pManager = *OVR::DeviceManager::Create();
    
    //pManager->SetMessageHandler(this);
    
    pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
    
	stereo.Set2DAreaFov(OVR::DegreeToRad(85.0f));
    stereo.SetFullViewport(OVR::Util::Render::Viewport(0,0, width, height));
    stereo.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
    stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
    renderScale = stereo.GetDistortionScale();
    
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
        
        stereo.SetHMDInfo(Info);
        stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
        renderScale = stereo.GetDistortionScale();
    }
    else
    {
        pSensor = *pManager->EnumerateDevices<OVR::SensorDevice>().CreateDevice();
    }
    
    textureWidth = width * renderScale;
    textureHeight = height * renderScale;
    
    leftEye  = stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
    rightEye = stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);
    
    // Left eye rendering parameters
    leftVP         = leftEye.VP;
    leftProjection = leftEye.Projection;
    leftViewAdjust = leftEye.ViewAdjust;
    
    // Right eye rendering parameters
    rightVP         = leftEye.VP;
    rightProjection = leftEye.Projection;
    rightViewAdjust = leftEye.ViewAdjust;
    
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
	if(pSensor) pSensor.Clear();
    if(pHMD) pHMD.Clear();
    if(pManager) pManager.Clear();
    
    if(pFusionResult) delete pFusionResult;
    
    OVR::System::Destroy();
}

static double orientationRift[16] = {1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1};
double *getRiftOrientation() {
    if(pFusionResult!= 0 && !pFusionResult->IsAttachedToSensor()) return orientationRift;
    OVR::Quatf quaternion = pFusionResult->GetPredictedOrientation();
    
    OVR::Matrix4f hmdMat(quaternion);
    for(int i = 0; i < 16; ++i) {
        orientationRift[i] = ((float*)hmdMat.M)[i];
    }
    return orientationRift;
}

static OVR::Matrix4f IdentityMatrix4f;
const OVR::Matrix4f getRiftOrientationNative() {
    if(pFusionResult!= 0 && !pFusionResult->IsAttachedToSensor()) return IdentityMatrix4f;
    
    return OVR::Matrix4f(pFusionResult->GetPredictedOrientation());
}


void setRenderScale(float renderScale_) {
    renderScale = renderScale_;
    
    textureWidth = width * renderScale;
    textureHeight = height * renderScale;
    
    renderScaleChanged = true;
}
void toggleRenderScale() {
    if(renderScale != 1) {
        setRenderScale(1);
    } else {
        setRenderScale(stereo.GetDistortionScale());
    }
}