#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "vad.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/*
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {"UNDEF", "S", "V", "MV", "MS", "INIT"};

const char *state2str(VAD_STATE st) { return state_str[st]; }

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

Features compute_features(const float *x, int N) {
  Features feat;
  feat.zcr = 0.0f;
  feat.am = 0.0f;
  float p = 0.0f;
  for (int i = 0; i < N; i++) {
    p += x[i] * x[i];
  }
  p /= N;
  if (p == 0.0f) {
    feat.p = -120.0f;
  } else {
    feat.p = 10.0f * log10f(p);
  }
  return feat;
}

VAD_DATA *vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->p0 = 0.0f;
  vad_data->alpha1 = 15.0f;
  vad_data->contador_inicial = 0;
  vad_data->contador_voz = 0;
  vad_data->contador_silencio = 0;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  VAD_STATE state = vad_data->state;
  if (state == ST_MAYBE_SILENCE)
    state = ST_VOICE;
  if (state == ST_MAYBE_VOICE)
    state = ST_SILENCE;
  if (state == ST_INIT)
    state = ST_SILENCE;
  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

VAD_STATE vad(VAD_DATA *vad_data, float *x) {
  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
  case ST_INIT:
    vad_data->p0 += f.p;
    vad_data->contador_inicial++;
    if (vad_data->contador_inicial >= 10) {
      vad_data->p0 /= 10.0f;
      vad_data->state = ST_SILENCE;
    }
    break;

  case ST_SILENCE:
    if (f.p > vad_data->p0 + vad_data->alpha1) {
      vad_data->state = ST_MAYBE_VOICE;
      vad_data->contador_voz = 1;
    }
    break;

  case ST_MAYBE_VOICE:
    if (f.p > vad_data->p0 + vad_data->alpha1) {
      vad_data->contador_voz++;
      if (vad_data->contador_voz >= 3) {
        vad_data->state = ST_VOICE;
      }
    } else {
      vad_data->state = ST_SILENCE;
    }
    break;

  case ST_VOICE:
    if (f.p <= vad_data->p0 + vad_data->alpha1) {
      vad_data->state = ST_MAYBE_SILENCE;
      vad_data->contador_silencio = 1;
    }
    break;

  case ST_MAYBE_SILENCE:
    if (f.p <= vad_data->p0 + vad_data->alpha1) {
      vad_data->contador_silencio++;
      if (vad_data->contador_silencio >= 20) {
        vad_data->state = ST_SILENCE;
      }
    } else {
      vad_data->state = ST_VOICE;
    }
    break;

  case ST_UNDEF:
    break;
  }

  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE)
    return vad_data->state;
  else if (vad_data->state == ST_MAYBE_VOICE)
    return ST_SILENCE;
  else if (vad_data->state == ST_MAYBE_SILENCE)
    return ST_VOICE;
  else
    return ST_SILENCE;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
