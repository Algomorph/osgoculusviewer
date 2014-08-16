/*
 * oculusdevice.h
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSDEVICE_H_
#define _OSG_OCULUSDEVICE_H_

// Include the OculusVR SDK
#include "OVR.h"

#include <osg/Geode>
#include <osg/Texture2D>

class OculusDevice : public osg::Referenced {

	public:
		enum Eye
		{
			LEFT = 0,
			RIGHT = 1,
			COUNT = 2
		};
		OculusDevice(float nearClip, float farClip, bool useTimewarp);

		unsigned int hScreenResolution() const;
		unsigned int vScreenResolution() const;

		unsigned int hRenderTargetSize() const;
		unsigned int vRenderTargetSize() const;

		int displayId() const { return m_hmdDevice->DisplayId; }
		osg::Vec2i windowPos() const { return osg::Vec2i(m_hmdDevice->WindowsPos.x, m_hmdDevice->WindowsPos.y);  }

		osg::Matrix projectionMatrixCenter() const;
		osg::Matrix projectionMatrixLeft() const;
		osg::Matrix projectionMatrixRight() const;

		osg::Matrix projectionOffsetMatrixLeft() const;
		osg::Matrix projectionOffsetMatrixRight() const;

		osg::Matrix viewMatrixLeft() const;
		osg::Matrix viewMatrixRight() const;

		float nearClip() const { return m_nearClip;	}
		float farClip() const { return m_farClip; }
		bool useTimewarp() const { return m_useTimeWarp; }

		void resetSensorOrientation() const;
		void updatePose(unsigned int frameIndex = 0);
		
		osg::Vec3 position() const { return m_position; }
		osg::Quat orientation() const { return m_orientation;  }
		
		int renderOrder(Eye eye) const;
		osg::Geode* distortionMesh(Eye eye, osg::Program* program, int x, int y, int w, int h, bool splitViewport=false);
		osg::Vec2f eyeToSourceUVScale(Eye eye) const;
		osg::Vec2f eyeToSourceUVOffset(Eye eye) const;
		osg::Matrixf eyeRotationStart(Eye eye) const;
		osg::Matrixf eyeRotationEnd(Eye eye) const;

		void beginFrameTiming(unsigned int frameIndex=0);
		void endFrameTiming() const;
		void waitTillTime();

		osg::Camera* createRTTCamera(osg::Texture* texture, OculusDevice::Eye eye, osg::Transform::ReferenceFrame referenceFrame, osg::GraphicsContext* gc = 0) const;
		osg::Camera* createWarpOrthoCamera(double left, double right, double bottom, double top, osg::GraphicsContext* gc) const;
		void applyShaderParameters(osg::StateSet* stateSet, osg::Program* program, osg::Texture2D* texture, OculusDevice::Eye eye) const;
		osg::GraphicsContext::Traits* graphicsContextTraits() const;

	protected:
		~OculusDevice(); // Since we inherit from osg::Referenced we must make destructor protected
		

		ovrHmd m_hmdDevice;
		ovrSizei m_resolution;
		ovrSizei m_renderTargetSize;
		ovrEyeRenderDesc m_eyeRenderDesc[2];
		ovrVector2f m_UVScaleOffset[2][2];
		ovrFrameTiming m_frameTiming;
		ovrPosef m_headPose[2];
		ovrMatrix4f m_timeWarpMatrices[2][2];

		osg::Matrixf m_leftEyeProjectionMatrix;
		osg::Matrixf m_rightEyeProjectionMatrix;
		osg::Vec3f m_leftEyeAdjust;
		osg::Vec3f m_rightEyeAdjust;

		osg::Vec3 m_position;
		osg::Quat m_orientation;

		float m_nearClip;
		float m_farClip;
		bool m_useTimeWarp;
	private:
		OculusDevice(const OculusDevice&); // Do not allow copy
		OculusDevice& operator=(const OculusDevice&); // Do not allow assignment operator.
};

class WarpCameraPreDrawCallback : public osg::Camera::DrawCallback
{
public:
	WarpCameraPreDrawCallback(osg::ref_ptr<OculusDevice> device) : m_device(device) {}
	virtual void operator()(osg::RenderInfo& renderInfo) const;
protected:
	osg::observer_ptr<OculusDevice> m_device;
};

class OculusSwapCallback : public osg::GraphicsContext::SwapCallback {
public:
	OculusSwapCallback(osg::ref_ptr<OculusDevice> device) : m_device(device), m_frameIndex(0) {}
	void swapBuffersImplementation(osg::GraphicsContext *gc);
	int frameIndex() const { return m_frameIndex; }
private:
	osg::observer_ptr<OculusDevice> m_device;
	int m_frameIndex;
};

class EyeRotationCallback : public osg::Uniform::Callback
{
public:
	enum Mode
	{
		START,
		END
	};
	EyeRotationCallback(Mode mode, const OculusDevice* device, OculusDevice::Eye eye) : m_mode(mode), m_device(device), m_eye(eye) {}
	virtual void operator()	(osg::Uniform* uniform, osg::NodeVisitor* nv);
protected:
	Mode m_mode;
	const OculusDevice* m_device;
	OculusDevice::Eye m_eye;
};

#endif /* _OSG_OCULUSDEVICE_H_ */
