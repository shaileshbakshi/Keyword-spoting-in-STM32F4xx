#ifndef __KWS_F469NI_H__
#define __KWS_F469NI_H__

#include "kws_ds_cnn.h"
#include "main.h"
//#include "kws_dnn.h"

// Change the parent class to KWS_DNN to switch to DNN model
//class KWS_F469ni : public KWS_DNN {
class KWS_F469ni : public KWS_DS_CNN {
public:
  KWS_F469ni(int recording_win, int sliding_window_len);
  ~KWS_F469ni();
  void start_kws();
  void set_volume(int vol);
  int16_t* audio_buffer_in;
  //for debugging: microphone to headphone loopback
  int16_t* audio_buffer_out;

};

#endif
