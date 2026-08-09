#ifndef WATER_H
#define WATER_H

#include "utils.h"
#include "detection.h"
#include "sky.h"
#include "clouds.h"
#include "noise.h"

// Fresnel - Schlick's approximation
float calculateFresnel(float cosR, float r0) {
  float a = 1.0-cosR;
  float a2 = a*a;
  return r0+(1.0-r0)*a2*a2*a;
}

vec4 nlWater(
  inout vec4 color,
  inout vec3 wPos,
  nl_skycolor skycol,
  nl_environment env,
  vec4 COLOR,
  vec3 viewDir,
  vec3 cPos,
  vec3 tiledCpos,
  vec3 gPos,
  vec3 CAMERA_POS,
  vec3 light,
  vec3 torchColor,
  vec2 lit,
  float fractCposY,
  float camDist,
  highp float t
) {

  // Water surface noise
  vec2 bump = vec2_splat(
    movingNoise2D(
      gPos.xz+gPos.yy,
      NL_WATER_WAVE_SPEED*t,
      0.6
    )
  );

  vec3 nrm;

  // Top water plane
  if (fractCposY > 0.0) {
    nrm.xz = bump*NL_WATER_BUMP;
    nrm.y = -1.0;
  }

  // Side water plane
  else {
    bump *= 0.5+0.5*sin(
      3.0*t*NL_WATER_WAVE_SPEED+cPos.y*PI_HALF
    );

    nrm.xz =
      normalize(viewDir.xz)+
      bump.y*(1.0-viewDir.xz*viewDir.xz)*NL_WATER_BUMP;

    nrm.y = bump.x*NL_WATER_BUMP;
  }

  nrm = normalize(nrm);

  // Reflection direction
  float cosR = dot(nrm,viewDir);
  viewDir = viewDir-2.0*cosR*nrm;

  // Sky reflection
  vec3 waterRefl = nlRenderSky(
    skycol,
    env,
    viewDir,
    t,
    false
  );

  // Cloud and aurora reflection
  #if defined(NL_CLOUD_AURORA_REFLECTION)
    if (viewDir.y < 0.0) {
      vec4 cloudRefl = nlCloudAuroraReflection(
        skycol,
        env,
        viewDir,
        wPos,
        CAMERA_POS,
        t
      );

      waterRefl = mix(
        waterRefl,
        cloudRefl.rgb,
        cloudRefl.a
      );
    }
  #endif

  // Torch reflection
  float tc = 0.5+0.5*sin(16.0*viewDir.x)*sin(16.0*viewDir.z);
  tc *= tc;

  waterRefl +=
    torchColor*
    NL_TORCHLIGHT_INTENSITY*
    lit.x*
    tc;

  // Reflection lighting
  if (!env.end) {
    waterRefl *= 0.08+lit.y*1.08;
  }

  // Optional reflection mask
  #ifdef NL_WATER_REFL_MASK
    float mask = 0.05+0.05*sin(viewDir.x*12.0)*sin(viewDir.z*6.0);

    waterRefl *= smoothstep(
      mask-0.2,
      mask+0.13,
      viewDir.y*viewDir.y
    );
  #endif

  // Fresnel reflection
  cosR = abs(cosR);
  float fresnel = calculateFresnel(cosR,0.055);
  float opacity = 1.0-cosR;

  // Water body color
  float waterDepth = 1.0-fresnel;

  vec3 waterTint =
    NL_WATER_TINT*
    (0.18+0.82*waterDepth);

  // Cool fantasy tint
  waterTint *= vec3(
    0.92,
    1.02,
    1.04
  );

  color.rgb *= waterTint;

  // Thick water transparency
  color.a = mix(
    COLOR.a*NL_WATER_TRANSPARENCY,
    1.0,
    opacity*opacity
  );

  // Water waves
  #ifdef NL_WATER_WAVE
    if (camDist < 14.0) {
      float waveHeight =
        0.5*(bump.x+0.5)*
        NL_WATER_BUMP;

      wPos.y -= waveHeight;
    }
  #endif

  return vec4(
    waterRefl,
    fresnel
  );
}

#endif