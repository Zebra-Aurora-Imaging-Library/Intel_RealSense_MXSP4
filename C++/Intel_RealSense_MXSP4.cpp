/******************************************************************************/
/*
 * File name: Intel_RealSense_MXSP4.cpp
 *
 * Synopsis: This program contains an example of 3d acquisition from
 *           an Intel RealSense depth camera, such as a D435 or an L515.
 *           See the PrintHeader() function below for detailed description.
 *
 * Copyright © 1992-2024 Zebra Technologies Corp. and/or its affiliates
 * All Rights Reserved
 */
 
#include <mil.h>
#include <iostream>             // for cout

//*****************************************************************************
// Structure representing a 3d point cloud, with position and intensity data.
//*****************************************************************************
struct SPoint3d
{
	MIL_FLOAT x;
	MIL_FLOAT y;
	MIL_FLOAT z;
};

struct SColor
{
	void ConvertRGBBGR(const SColor& OtherColor)
	{
		x = OtherColor.z;
		y = OtherColor.y;
		z = OtherColor.x;
	}

	MIL_UINT8 x;
	MIL_UINT8 y;
	MIL_UINT8 z;
	MIL_UINT8 a;
};

// Once the RealSense SDK is installed and the project is configured (seePrintHeader()), 
// set REALSENSESDK_INSTALLED to 1 to enable the RealSense-specific code.
// You need an actual camera connected to your computer.
#define REALSENSESDK_INSTALLED  0

// Enable the specific 3d display settings to adjust the view for this example.
#define DISPLAY_ADJUST_SPECIFIC  1

//Set 1 to extract confidence if available 
#define EXTRACT_CONFIDENCE 0

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("Intel_RealSense_MXSP4\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program acquires a 3d point cloud using an Intel RealSense sensor\n")
      MIL_TEXT("using the Intel RealSense SDK. It then converts the point cloud to the MIL\n")
      MIL_TEXT("format and displays the result.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, buffer, 3D display,\n")
      MIL_TEXT("              3D graphics, 3D processing.\n\n"));
   }

//*****************************************************************************
// RealSense-specific 
//*****************************************************************************
#if REALSENSESDK_INSTALLED
// Include RealSense Cross Platform API
#include <librealsense2/rs.hpp> 
#pragma comment(lib, "realsense2.lib")

// 2D Texture points structure
struct SPoint2d
   {
   MIL_FLOAT u;
   MIL_FLOAT v;
   };

// Function to allocate and get components.
template <class T>
MIL_ID GetMILContainerComponent(MIL_ID MilSystem, MIL_ID MilContainer, MIL_INT ComponentIdFlag,
   MIL_INT NbBands, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT Attribute,
   T* pData, MIL_INT* pPitch);

// Function to extract data then display from a RealSense Depth camera.
int InterfaceRealSense(void);
#endif

int main(void) 
   {
   PrintHeader();

#if !REALSENSESDK_INSTALLED
   MosPrintf(MIL_TEXT("This example is designed to be used with Intel RealSense depth camera and\n"));
   MosPrintf(MIL_TEXT("the Intel RealSense SDK. To run the example:\n\n"));
   MosPrintf(MIL_TEXT("- Install the Intel RealSense SDK 2.0.\n\n"));
   MosPrintf(MIL_TEXT("- Connect the camera to your computer.\n"));
   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("- Add a System Environment Variable named RealsenseSDK,\n"));
   MosPrintf(MIL_TEXT("  with the path to the RealsenseSDK 2.0 directory.\n"));
   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("- Add the path to the dll folder (...\\Intel RealSense SDK 2.0\\bin\\x64)\n")); 
   MosPrintf(MIL_TEXT("  to the PATH System Environment Variable,\n"));
   MosPrintf(MIL_TEXT("Note: you must restart Visual Studio after changing / setting System Environment Variables.\n"));
   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("- Add the paths to the header and library files of the Realsense SDK 2.0\n"));
   MosPrintf(MIL_TEXT("  to the example project files: in Visual Studio, open the Property Page\n"));
   MosPrintf(MIL_TEXT("  Under Configuration Properties,\n"));
   MosPrintf(MIL_TEXT("  - Add\n"));
   MosPrintf(MIL_TEXT("      $(RealsenseSDK)\\include\n"));
   MosPrintf(MIL_TEXT("    to\n"));
   MosPrintf(MIL_TEXT("      C/C++->General->Additional Include Directories\n"));
   MosPrintf(MIL_TEXT("  - Add \n"));
   MosPrintf(MIL_TEXT("      $(RealsenseSDK)\\lib\\x64\n"));
   MosPrintf(MIL_TEXT("    to \n"));
   MosPrintf(MIL_TEXT("      Linker->General->Additional Library Directories\n\n"));
   
   MosPrintf(MIL_TEXT("- Update the example code:\n"));
   MosPrintf(MIL_TEXT("  - Set the REALSENSESDK_INSTALLED define to 1.\n"));
   MosPrintf(MIL_TEXT("  - If Confidence stream is available on the connecting camera, optionally \n"));
   MosPrintf(MIL_TEXT("    set the EXTRACT_CONFIDENCE define to 1 to extract confidence.\n"));
   MosPrintf(MIL_TEXT("  - Recompile the example.\n\n"));
   
   MosPrintf(MIL_TEXT("The example has been tested with the following setup:\n"));
   MosPrintf(MIL_TEXT("- Windows 10 64-bit and VS2017.\n"));
   MosPrintf(MIL_TEXT("- MIL X Version 1911, SP4 build 647.\n"));
   MosPrintf(MIL_TEXT("- Realsense SDK 2.36.\n"));
   MosPrintf(MIL_TEXT("- Intel RealSense D435 and L515.\n"));
   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();
#else
   InterfaceRealSense();
#endif
   return 0;
}

#if REALSENSESDK_INSTALLED
int InterfaceRealSense(void)
{
   try
   {
       MosPrintf(MIL_TEXT("Initialization...\n"));
      // Declare pointcloud object, for calculating pointclouds and texture mappings
      rs2::pointcloud pc;
      // We want the points object to be persistent so we can display the last cloud when a frame drops
      rs2::points points;
      // Create a Pipeline - this serves as a top-level API for streaming and processing frames
      rs2::pipeline p;
   
      // Create a config object
      rs2::config cfg;

      // Enable the streams
      cfg.enable_stream(RS2_STREAM_DEPTH);
      cfg.enable_stream(RS2_STREAM_COLOR);

      if (EXTRACT_CONFIDENCE)
         cfg.enable_stream(RS2_STREAM_CONFIDENCE);
         
      auto profile = p.start(cfg);
      
      // Retrieve dummy Intel RealSense components to allocate the application's objects.
      auto frames = p.wait_for_frames();
      auto color = frames.get_color_frame();
      auto depth = frames.get_depth_frame();

      MIL_INT PointCloudsWidth = depth.get_width();
      MIL_INT PointCloudsHeight = depth.get_height();

      MIL_INT TextureWidth = 0, TextureHeight = 0;

      // Allocate the MIL application.
      MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

      // Allocate a host system.
      MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

      // Restore a 3D point cloud of the object.
      MIL_UNIQUE_BUF_ID PointCloudContainer = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);

      SPoint3d* pPointCloud = nullptr;
      MIL_INT PointCloudPitch;
      MIL_ID RangeComponent = GetMILContainerComponent(MilSystem, PointCloudContainer, M_COMPONENT_RANGE,
         3, PointCloudsWidth, PointCloudsHeight, 32 + M_FLOAT, M_IMAGE + M_PROC + M_PACKED + M_RGB96,
         &pPointCloud, &PointCloudPitch);

      MbufControlContainer(PointCloudContainer, M_COMPONENT_RANGE, M_3D_INVALID_DATA_FLAG, M_TRUE);

      // Get the "reflectance" component if RGB color is available.
      SColor* pReflectance = nullptr;
      MIL_INT ReflectancePitch;
      MIL_ID ReflectanceComponent = M_NULL;

      if (color)
        {
         if (color.get_profile().format() == RS2_FORMAT_RGB8)
            {
            TextureWidth = color.get_width();
            TextureHeight = color.get_height();
            ReflectanceComponent = GetMILContainerComponent(MilSystem, PointCloudContainer, M_COMPONENT_REFLECTANCE,
               3, PointCloudsWidth, PointCloudsHeight, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_RGB32 + M_PACKED,
               &pReflectance, &ReflectancePitch);
            }
         }

      // Get the "confidence" component into the confidence.
      unsigned char* pConfidence = nullptr;
      MIL_INT ConfidencePitch;
      MIL_ID ConfidenceComponent = M_NULL;
      if (EXTRACT_CONFIDENCE)
         {
         ConfidenceComponent = GetMILContainerComponent(MilSystem, PointCloudContainer, M_COMPONENT_CONFIDENCE,
            1, PointCloudsWidth, PointCloudsHeight, 8 + M_UNSIGNED, M_IMAGE + M_PROC,
            &pConfidence, &ConfidencePitch);
         }

      // Allocate a container for display
      MIL_UNIQUE_BUF_ID ContainerDisp = MbufClone(PointCloudContainer, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

      // Allocate a 3D Display 
      MIL_UNIQUE_3DDISP_ID M3dDisplay = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

      if (!M3dDisplay)
         {
         MosPrintf(MIL_TEXT("The current system does not support the 3D display.\n")
            MIL_TEXT("Press <Enter> to finish.\n"));
         MosGetch();
         return 0;
         }

      // MIL 3D display settings, to coordinate with Real Sense outputs.
      MIL_INT64 PtCldLabel = M3ddispSelect(M3dDisplay, ContainerDisp, M_SELECT, M_DEFAULT);
      M3ddispSetView(M3dDisplay, M_INTEREST_POINT, 0, 0, 1, M_DEFAULT);
      M3ddispSetView(M3dDisplay, M_VIEWPOINT, 0, 0, -1, M_DEFAULT);
      M3ddispSetView(M3dDisplay, M_UP_VECTOR, 0, -1, 0, M_DEFAULT);

      if (DISPLAY_ADJUST_SPECIFIC)
         {
         MIL_ID GraListId = M3ddispInquire(M3dDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
         M3dgraControl(GraListId, PtCldLabel, M_THICKNESS, 2);

         M3ddispControl(M3dDisplay, M_FOV_HORIZONTAL_ANGLE, 60);
         M3ddispSetView(M3dDisplay, M_TRANSLATE, 0, 0, 1, M_DEFAULT);
         }

      MIL_INT stride = 0;
      unsigned char* ColorData = nullptr;
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      while (!MosKbhit())
         {
         // Wait until new frames are acquired.
         frames = p.wait_for_frames();

         // Retrieve Intel RealSense components.
         color = frames.get_color_frame();
         pc.map_to(color);
         depth = frames.get_depth_frame();
         points = pc.calculate(depth);

         auto vertices = points.get_vertices();
         auto tex_coords = points.get_texture_coordinates();

         stride = color.as<rs2::video_frame>().get_stride_in_bytes();
         ColorData = (unsigned char *)color.get_data();

         unsigned char *ConfidenceData = nullptr;
         if (EXTRACT_CONFIDENCE)
            {
            rs2::frame confidence = frames.first(RS2_STREAM_CONFIDENCE);
            ConfidenceData = (unsigned char *)confidence.get_data();
            }

         MIL_INT p = 0;
         MIL_UINT mappedX = 0, mappedY = 0;
         for (MIL_INT y = 0; y < PointCloudsHeight; y++)
            {
            for (MIL_INT x = 0; x < PointCloudsWidth; x++)
               {
               // If RGB color is available, apply 2D texture mapping onto the point clouds that have corresponding colors in 2D frame.
               if (color)
                  {
                  if (tex_coords[p].u >= 0 && tex_coords[p].u <= 1 && tex_coords[p].v >= 0 && tex_coords[p].v <= 1)
                     {
                     pPointCloud[p].x = vertices[p].x;
                     pPointCloud[p].y = vertices[p].y;
                     pPointCloud[p].z = vertices[p].z;

                     mappedX = (MIL_UINT)(tex_coords[p].u * TextureWidth) % TextureWidth;
                     mappedY = (MIL_UINT)(tex_coords[p].v * TextureHeight) % TextureHeight;
                     pReflectance[p].x = ColorData[(mappedY * stride + 3 * mappedX)];
                     pReflectance[p].y = ColorData[(mappedY * stride + 3 * mappedX) + 1];
                     pReflectance[p].z = ColorData[(mappedY * stride + 3 * mappedX) + 2];
                     }
                  }
               else
                  {
                  pPointCloud[p].x = vertices[p].x;
                  pPointCloud[p].y = vertices[p].y;
                  pPointCloud[p].z = vertices[p].z;
                  }

               if (EXTRACT_CONFIDENCE)
                  {
                  pConfidence[x + y * ConfidencePitch] = ConfidenceData[p];
                  }
               p++;
               }
            }
         // Convert the container to display.
         MbufConvert3d(PointCloudContainer, ContainerDisp, M_NULL, M_DEFAULT, M_DEFAULT);
         }
      }
   catch (const rs2::error & e)
      {
      std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
      MosPrintf(MIL_TEXT("Press <Enter> to finish.\n"));
      MosGetch();
      return EXIT_FAILURE;
      }
   catch (const std::exception & e)
      {
      std::cerr << e.what() << std::endl;
      MosPrintf(MIL_TEXT("Press <Enter> to finish.\n"));
      MosGetch();
      return EXIT_FAILURE;
      }

   return EXIT_SUCCESS;
   }

//*****************************************************************************
// Retrieve a component from a MIL container.
// If not present, the component is allocated.
//*****************************************************************************
template <class T>
MIL_ID GetMILContainerComponent(MIL_ID MilSystem, MIL_ID MilContainer, MIL_INT ComponentIdFlag,
   MIL_INT NbBands, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT Attribute,
   T* pData, MIL_INT* pPitch)
   {
   MIL_ID MilComponent = M_NULL;
   MilComponent = MbufInquireContainer(MilContainer, ComponentIdFlag, M_COMPONENT_ID, M_NULL);

   if (!MilComponent)
      {
      MbufAllocComponent(MilContainer, NbBands, SizeX, SizeY, Type, Attribute, ComponentIdFlag, &MilComponent);
      }
   MbufInquire(MilComponent, M_HOST_ADDRESS, ((void**)pData));
   MbufInquire(MilComponent, M_PITCH, pPitch);
   return MilComponent;
   }
#endif
