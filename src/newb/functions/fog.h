#ifndef FOG_H
#define FOG_H

float nlRenderFogFade(float relativeDist, vec3 FOG_COLOR, vec2 FOG_CONTROL) {
  #ifdef NL_FOG

  // ------------------------------------------------------------
  // REALISTIC + FANTASY ATMOSPHERIC FOG
  // ------------------------------------------------------------

  // Base distance fog
  float fade = smoothstep(FOG_CONTROL.x, FOG_CONTROL.y, relativeDist);

  // Thick atmospheric mist
  float density = NL_MIST_DENSITY * (19.0 - 18.0 * FOG_COLOR.g);
  float mist = 1.0 - exp(-relativeDist*relativeDist*density);

  // Softer near-field mist
  float nearMist = 1.0 - exp(-relativeDist*density*0.18);

  // Combine distance fog + thick mist
  float atmosphericFog = mix(fade, mist, 0.72);
  atmosphericFog += (1.0 - atmosphericFog)*nearMist*0.35;

  // Prevent the fog from becoming completely opaque too quickly
  atmosphericFog = smoothstep(0.0, 1.0,atmosphericFog);

  // ------------------------------------------------------------
  // COOL FANTASY ATMOSPHERIC DEPTH
  // ------------------------------------------------------------

  float fantasyFog = smoothstep(0.15, 0.85, relativeDist);

  // Slight blue/cyan atmospheric influence
  vec3 fantasyTint = vec3(0.08, 0.16, 0.22);
  float fogBrightness = dot(FOG_COLOR, vec3(0.299, 0.587, 0.114));
  fantasyTint *= 0.55 + 0.45*fogBrightness;

  // Add subtle atmospheric thickness
  atmosphericFog += fantasyFog*0.12;
  return NL_FOG*clamp(atmosphericFog,0.0,1.0);

  #else
    return 0.0;
  #endif
}

  float nlRenderGodRayIntensity(vec3 cPos, vec3 worldPos, float t, vec2 uv1 ,float relativeDist, vec3 FOG_COLOR)
  
{
  // offset wPos (only works upto 16 blocks)
  vec3 offset = cPos*16.0*fract(worldPos*0.0625);
  offset = abs(2.0*fract(offset*0.0625) - 1.0);
  offset = offset*offset*(3.0 - 2.0*offset);
  vec3 nrmof = normalize(worldPos);
  float u = nrmof.z/length(nrmof.zy);
  float diff = dot(offset, vec3(0.1, 0.2, 1.0)) + 0.07*t;
  float mask = nrmof.x*nrmof.x;
  float vol = sin(7.0*u + 1.5*diff)*sin(3.0*u + diff);
  vol *= vol*mask*uv1.y*(1.0 - mask*mask);
  vol *= relativeDist*relativeDist;

  // Dawn / dusk mask
  vol *= clamp(3.0*(FOG_COLOR.r - FOG_COLOR.b), 0.0, 1.0);
  vol = smoothstep(0.0, 0.1, vol);

  return vol;
}

#endif
