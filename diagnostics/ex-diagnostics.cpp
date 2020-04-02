/*
 * Copyright (C) 2018 ifm syntron gmbh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distribted on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// ex-diagnostics.cpp
//
//  Shows how to change imager exposure times on the fly while streaming in
//  pixel data and validating the setting of the exposure times registered to
//  the frame data.
//

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <ifm3d/camera.h>
#include <ifm3d/fg.h>
#include <ifm3d/image.h>

float read_uptime(ifm3d::Camera::Ptr cam)
{
	float fUpTime = -1.0;
	try
	{
		fUpTime = std::stof(cam->DeviceParameter("UpTime"));
		std::cout << "UpTime (h): " << fUpTime << std::endl;
	}
	catch (const ifm3d::error_t& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
	}
	return fUpTime;
}

int main(int argc, const char **argv)
{
	   // instantiate the camera and set the configuration
  ifm3d::Camera::Ptr cam = std::make_shared<ifm3d::Camera>();


	// create json that will receive configuration
	json jsonConfig;

	try
	{
		// get the JSON configuration data from the camera
		jsonConfig = cam->ToJSON();

	}
	catch (const ifm3d::error_t& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return -1;
	}

  // Reading info from Device related to Diagnostics from JSON dump
  std::cout << "Reading camera Device info from internal configuration: " << std::endl
	  << "TemperatureIMX6: " << jsonConfig["ifm3d"]["Device"]["TemperatureIMX6"].get<std::string>() << " \370"<< "C" << std::endl
	  << "TemperatureIllu: " << jsonConfig["ifm3d"]["Device"]["TemperatureIllu"].get<std::string>() << " \370" << "C" << std::endl
	  << "UpTime: " << jsonConfig["ifm3d"]["Device"]["UpTime"].get<std::string>() << " hours since last reboot" << std::endl ;
	  

  // create our image buffer to hold frame data from the camera
  ifm3d::ImageBuffer::Ptr img = std::make_shared<ifm3d::ImageBuffer>();

  // instantiate our framegrabber and be sure to explicitly tell it to
  // stream back illumination temperatue through PCIC here
  ifm3d::FrameGrabber::Ptr fg = 
    std::make_shared<ifm3d::FrameGrabber>(
      cam, ifm3d::DEFAULT_SCHEMA_MASK|ifm3d::ILLU_TEMP);

	
	float fUpTime = -1.0;
	float fOldUpTime = -1.0;
  float fIlluTemp = -1.0;

// now we start looping over the image data, after 500 frames we will exit.
  int iFrame = 0;
	int iMissFrame = 0;
  while (true)
  {
		if (!fg->WaitForFrame(img.get(), 1000))
		{
			// We have lost a frame !
			std::cerr << "Timeout waiting for camera!" << std::endl;

			//1. Case 1 : Loss of connectivity due to camera OF/ON
			fUpTime = read_uptime(cam);
			if (fUpTime > 0)
			{
				if (fOldUpTime >= fUpTime)
				{
					//We can now be sure we had a camera reboot here
					std::cerr << "Camera reboot detected!" << std::endl;
					std::cout << "Leaving the application : " << std::endl;
					break;
				}
			}
			else std::cerr << "UpTime not available " << fUpTime << std::endl;

			//Anyway after 100 attempts we consider we have to leave the application
			if (iMissFrame < 100)
			{
				iMissFrame++;
			}
			else
			{
				std::cout << "Leaving the application : " << std::endl;
				break;
			}

			continue;
		}
		else
		{
			// Everything's OK here, I received camera frames as exepcted
			fIlluTemp = img->IlluTemp();
			std::cout << "TemperatureIllu : " << fIlluTemp << std::endl;

			fUpTime = read_uptime(cam);
			if (fUpTime > 0) fOldUpTime = fUpTime;
			else std::cerr << "UpTime not available " << fUpTime << std::endl;
		}
		
		

		if (iFrame == 500)
		{
			break;
		}
		
  }

  std::cout << "et voila." << std::endl;
  return 0;
}
