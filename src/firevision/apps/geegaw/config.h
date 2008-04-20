
/***************************************************************************
 *  config.h - config helper - generated by genconfig 0.3
 *
 *  Config class generated: Sun Apr 20 15:18:56 2008
 *  Used template: ../../../../../src/modules/tools/genconfig/config.template.h
 *  Template created: Thu Apr 29 14:26:23 2004
 *  Copyright  2004  Tim Niemueller
 *  niemueller@i5.informatik.rwth-aachen.de
 *
 *  $Id$
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */
 
#ifndef ___GEEGAWCONFIG_H_
#define ___GEEGAWCONFIG_H_

/// @cond RCSOFTX_CONFIG

#include <utils/config_reader/config_reader.h>
#include <string>

class GeegawConfig
{
 public:
  GeegawConfig(CConfigReader *configFile);
  ~GeegawConfig();

  /** ScanlineGridYOffset value */
  int ScanlineGridYOffset;
  /** ScanlineModel value */
  std::string ScanlineModel;
  /** HorizontalViewingAngle value */
  float HorizontalViewingAngle;
  /** CameraOrientation value */
  float CameraOrientation;
  /** ScanlineGridXOffset value */
  int ScanlineGridXOffset;
  /** CameraOffsetY value */
  float CameraOffsetY;
  /** PanMinChange value */
  float PanMinChange;
  /** ColormapDirectory value */
  std::string ColormapDirectory;
  /** ForwardDelay value */
  float ForwardDelay;
  /** TiltMax value */
  float TiltMax;
  /** ForwardPan value */
  float ForwardPan;
  /** ShrinkerType value */
  std::string ShrinkerType;
  /** ClassifierType value */
  std::string ClassifierType;
  /** CameraType value */
  std::string CameraType;
  /** ForwardTilt value */
  float ForwardTilt;
  /** ColormapFilestem value */
  std::string ColormapFilestem;
  /** CameraHeight value */
  float CameraHeight;
  /** ColorModel value */
  std::string ColorModel;
  /** VerticalViewingAngle value */
  float VerticalViewingAngle;
  /** TiltMinChange value */
  float TiltMinChange;
  /** CameraOffsetX value */
  float CameraOffsetX;
  /** CameraControlType value */
  std::string CameraControlType;


 protected:
  /** Config reader */
  CConfigReader * m_pXMLConfigFile;
  /** Current config prefix. */
  std::string m_sPrefix;

};

/// @endcond

#endif /* ___GEEGAWCONFIG_H_ */

