/*!
\file GUITexture.h
\brief 
*/

#ifndef GUILIB_GUITEXTURE_H
#define GUILIB_GUITEXTURE_H

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

#include "TextureManager.h"
#include "AdvancedSettings.h"
#include "gui3d.h"
#include "Geometry.h"
#include "TransformMatrix.h"
#include "WindowingFactory.h"
#include "system.h" // HAS_GL, HAS_DX, etc

typedef uint32_t color_t;

// image alignment for <aspect>keep</aspect>, <aspect>scale</aspect> or <aspect>center</aspect>
#define ASPECT_ALIGN_CENTER  0
#define ASPECT_ALIGN_LEFT    1
#define ASPECT_ALIGN_RIGHT   2
#define ASPECT_ALIGNY_CENTER 0
#define ASPECT_ALIGNY_TOP    4
#define ASPECT_ALIGNY_BOTTOM 8
#define ASPECT_ALIGN_MASK    3
#define ASPECT_ALIGNY_MASK  ~3

#ifdef _DEBUG
#define RATIO 2.25f
#define PS_SINGLE 0
#define PS_SINGLE_COLOR 1
#define PS_MULTI 2
#define PS_MULTI_COLOR 3
#endif

typedef struct _ImageVertex
{
  float x, y, z;
  float u1, v1;
  float u2, v2;
} ImageVertex;

typedef struct _ImageCoords
{
  ImageVertex m_pCoords[4];
} ImageCoords;

class CAspectRatio
{
public:
  enum ASPECT_RATIO { AR_STRETCH = 0, AR_SCALE, AR_KEEP, AR_CENTER };
  CAspectRatio(ASPECT_RATIO aspect = AR_STRETCH)
  {
    ratio = aspect;
    align = ASPECT_ALIGN_CENTER | ASPECT_ALIGNY_CENTER;
    scaleDiffuse = true;
  };
  bool operator!=(const CAspectRatio &right) const
  {
    if (ratio != right.ratio) return true;
    if (align != right.align) return true;
    if (scaleDiffuse != right.scaleDiffuse) return true;
    return false;
  };

  ASPECT_RATIO ratio;
  uint32_t     align;
  bool         scaleDiffuse;
};

class CTextureInfo
{
public:
  CTextureInfo();
  CTextureInfo(const CStdString &file);
  void operator=(const CTextureInfo &right);
  bool       useLarge;
  CRect      border;      // scaled  - unneeded if we get rid of scale on load
  CRect      srcBorder;   // non scaled
  int        orientation; // orientation of the texture (0 - 7 == EXIForientation - 1)
  CStdString diffuse;     // diffuse overlay texture
  CStdString filename;    // main texture file
  bool       blendCenter;
  bool       blendBorder;
};

class CGUITextureBase
{
public:
  CGUITextureBase(float posX, float posY, float width, float height, const CTextureInfo& texture, CBaseTexture* textureData = NULL);
  CGUITextureBase(const CGUITextureBase &left);
  virtual ~CGUITextureBase(void);
  static void PrintPixelCount();

  void Render();

  void DynamicResourceAlloc(bool bOnOff);
  void AllocResources();
  void FreeResources(bool immediately = false);

  void SetVisible(bool visible);
  void SetAlpha(unsigned char alpha);
  void SetDiffuseColor(color_t color);
  void SetPosition(float x, float y);
  void SetWidth(float width);
  void SetHeight(float height);
  void SetFileName(const CStdString &filename);
  void SetLoadingAnimation(const CStdString& strLoadingAnimation) { m_loadingAnimation = strLoadingAnimation; };
  void SetAspectRatio(const CAspectRatio &aspect);
  void SetRenderBorderOnly(bool renderBorderOnly);

  const CStdString& GetFileName() const { return m_info.filename; };
  float GetTextureWidth() const { return m_frameWidth; };
  float GetTextureHeight() const { return m_frameHeight; };
  float GetWidth() const { return m_width; };
  float GetHeight() const { return m_height; };
  float GetXPosition() const { return m_posX; };
  float GetYPosition() const { return m_posY; };
  int GetOrientation() const;
  const CRect &GetRenderRect() const { return m_vertex; };
  bool IsLazyLoaded() const { return m_info.useLarge; };

  bool HitTest(const CPoint &point) const { return CRect(m_posX, m_posY, m_posX + m_width, m_posY + m_height).PtInRect(point); };
  bool IsAllocated() const { return m_isAllocated != NO; };
  bool FailedToAlloc() const { return m_isAllocated == NORMAL_FAILED || m_isAllocated == LARGE_FAILED; };
  bool ReadyToRender() const;

protected:
  void CalculateSize();
  void LoadDiffuseImage();
  void AllocateOnDemand();
  void UpdateAnimFrame();
  void BuildVertexCoords(ImageCoords& coords, float left, float top, float bottom, 
          float right, float u1, float v1, float u2, float v2, float u3, float v3);
  void BuildColorData();
  void OrientateTexture(CRect &rect, float width, float height, int orientation);

  void DrawRectangleElements();

  // functions that our implementation classes handle
  virtual void Allocate() {}; ///< called after our textures have been allocated
  virtual void Free() {};     ///< called after our textures have been freed
  virtual void Begin() = 0;
  virtual void Draw(ImageCoords& coords) = 0;
  virtual void End() = 0;

  virtual bool LoadShaders() = 0;
  virtual bool SelectShader() = 0;

  bool    m_visible;
  color_t m_diffuseColor; // the original diffuse color
  color_t m_diffuseColorBlended;// diffuse color after blending
  bool    m_bNeedBlending;

  float m_posX;         // size of the frame
  float m_posY;
  float m_width;
  float m_height;

  CRect m_vertex;       // vertex coords to render
  bool m_invalid;       // if true, we need to recalculate
  float m_globalAlpha;  // track alpha changes from global effects (e.g fadein-fadeout)

  // left, center and right coordinates
  ImageCoords m_vertexCoords[9];
  unsigned char m_alpha;

  float m_frameWidth, m_frameHeight;          // size in pixels of the actual frame within the texture
  float m_texCoordsScaleU, m_texCoordsScaleV; // scale factor for pixel->texture coordinates

  // animations
  int m_currentLoop;
  unsigned int m_currentFrame;
  uint32_t m_frameCounter;

  float m_diffuseU, m_diffuseV;           // size of the diffuse frame (in tex coords)
  float m_diffuseScaleU, m_diffuseScaleV; // scale factor of the diffuse frame (from texture coords to diffuse tex coords)
  CPoint m_diffuseOffset;                 // offset into the diffuse frame (it's not always the origin)

  bool m_allocateDynamically;
  enum ALLOCATE_TYPE { NO = 0, NORMAL, LARGE, NORMAL_FAILED, LARGE_FAILED };
  ALLOCATE_TYPE m_isAllocated;

  CTextureInfo m_info;
  CAspectRatio m_aspect;
  CStdString m_loadingAnimation;

  CTextureArray m_diffuse;
  CTextureArray m_texture;

  bool m_renderBorderOnly;
};


#if defined(HAS_GLES)
#include "GUITextureGLES.h"
#define CGUITexture CGUITextureGLES
#elif defined(HAS_GL2)
#include "GUITextureGL.h"
#define CGUITexture CGUITextureGL
#elif defined(HAS_DX)
#include "GUITextureD3D.h"
#define CGUITexture CGUITextureD3D
#endif

#endif
