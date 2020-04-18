#include "wav_data.h"
#include "kws_ds_cnn.h"
#include "plot_utils.h"

#include <stdio.h>
#include <string.h>
#include "main.h"

extern UART_HandleTypeDef UartHandle;

KWS_F469ni *kws;

int16_t audio_buffer[16000] = WAVE_DATA;
char txbuf[50];
char lcd_output_string[64];
// Tune the following three parameters to improve the detection accuracy
//  and reduce false positives
// Longer averaging window and higher threshold reduce false positives
//  but increase detection latency and reduce true positive detections.

// (recording_win*frame_shift) is the actual recording window size
int recording_win = 10;
// Averaging window for smoothing out the output predictions
int averaging_window_len = 6;
int detection_threshold = 92;  //in percent

char output_class[12][8] = {"Silence", "Unknown","yes","no","up","down","left","right","on","off","stop","go"};

extern uint16_t recbuff[INTERNAL_BUFF_SIZE];

void run_kws(void);


typedef enum {
  BUFFER_OFFSET_NONE = 0,
  BUFFER_OFFSET_HALF,
  BUFFER_OFFSET_FULL,
}BUFFER_StateTypeDef;

typedef struct {
  BUFFER_StateTypeDef state;
  int fptr;
}Audio_BufferTypeDef;

Audio_BufferTypeDef Audiostate;

void kws_main_static()
{

  KWS_DS_CNN kws(audio_buffer);

  kws.extract_features();   //extract mfcc features
  kws.classify();	        //classify using dnn

  int max_ind = kws.get_top_class(kws.output);
  //pc.printf("Total time : %d us\r\n",end-start);s
  sprintf(txbuf,"Detected %s (%d%%)\r\n",output_class[max_ind],((int)kws.output[max_ind]*100/128));

  HAL_UART_Transmit(&UartHandle, (uint8_t*)txbuf ,strlen(txbuf), 0xFFFF);

}

void kws_main_realtime()
{
  kws = new KWS_F469ni(recording_win,averaging_window_len);
  init_plot();
  kws->start_kws();

  Audiostate.state =  BUFFER_OFFSET_NONE;
  Audiostate.fptr =0;

  while (1)
  {
	  if(Audiostate.state == BUFFER_OFFSET_HALF )
	  {
		  memset((q7_t *)kws->audio_buffer_out,0, kws->audio_block_size*4);
		  arm_copy_q7((q7_t *)kws->audio_buffer_in, (q7_t *)kws->audio_buffer_out, kws->audio_block_size*4);

		  if(kws->frame_len!=kws->frame_shift)
		  {
		    //copy the last (frame_len - frame_shift) audio data to the start

		    arm_copy_q7((q7_t *)(kws->audio_buffer)+2*(kws->audio_buffer_size-(kws->frame_len-kws->frame_shift)), (q7_t *)kws->audio_buffer, 2*(kws->frame_len-kws->frame_shift));
		  }
		  // copy the new recording data
		  for (int i=0;i< kws->audio_block_size;i++)
		  {
		    kws->audio_buffer[kws->frame_len-kws->frame_shift+i] = (int16_t)(kws->audio_buffer_in[i*2])/4;
		  }
		  run_kws();
	  }
	  if(Audiostate.state == BUFFER_OFFSET_FULL)
	  {
		  memset((q7_t *)kws->audio_buffer_out + kws->audio_block_size*4,0, kws->audio_block_size*4);
		  arm_copy_q7((q7_t *)kws->audio_buffer_in + kws->audio_block_size*4, (q7_t *)kws->audio_buffer_out + kws->audio_block_size*4, kws->audio_block_size*4);

		  if(kws->frame_len != kws->frame_shift)
		  {
		    //copy the last (frame_len - frame_shift) audio data to the start
		    arm_copy_q7((q7_t *)(kws->audio_buffer)+2*(kws->audio_buffer_size-(kws->frame_len-kws->frame_shift)), (q7_t *)kws->audio_buffer, 2*(kws->frame_len-kws->frame_shift));
		  }
		  // copy the new recording data
		  for (int i=0;i< kws->audio_block_size;i++)
		  {
		    kws->audio_buffer[kws->frame_len-kws->frame_shift+i] = (int16_t)(kws->audio_buffer_in[2*kws->audio_block_size+i*2])/4;
		  }
		  Audiostate.state = BUFFER_OFFSET_NONE;
		  run_kws();
	  }
  }
}

void run_kws()
{
  kws->extract_features();    //extract mfcc features
  kws->classify();	      //classify using dnn
  kws->average_predictions();
  plot_mfcc();
  plot_waveform();
  int max_ind = kws->get_top_class(kws->averaged_output);
  if(kws->averaged_output[max_ind]>detection_threshold*128/100)
  {
	  sprintf(lcd_output_string,"%d%% %s",((int)kws->averaged_output[max_ind]*100/128),output_class[max_ind]);
	 // HAL_UART_Transmit(&UartHandle, (uint8_t*)lcd_output_string ,strlen(lcd_output_string), 0xFFFF);
  }

  BSP_LCD_ClearStringLine(30);//lcd.ClearStringLine(8);
  //BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_SetFont(&Font12);

  BSP_LCD_DisplayStringAt(0, LINE(30), (uint8_t *) lcd_output_string, CENTER_MODE);//lcd.DisplayStringAt(0, LINE(8), (uint8_t *) lcd_output_string, CENTER_MODE);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
}

/*
 * The audio recording works with two ping-pong buffers.
 * The data for each window will be tranfered by the DMA, which sends
 * sends an interrupt after the transfer is completed.
 */

// Manages the DMA Transfer comp+lete interrupt.
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
  /* PDM to PCM data convert */
  BSP_AUDIO_IN_PDMToPCM((uint16_t*)&recbuff[INTERNAL_BUFF_SIZE/2],(uint16_t*) (kws->audio_buffer_in + Audiostate.fptr));
  Audiostate.fptr+= INTERNAL_BUFF_SIZE/4/2;

  if( Audiostate.fptr == kws-> audio_block_size * 2)
  {
	  Audiostate.state = BUFFER_OFFSET_HALF;
  }

  if( Audiostate.fptr  == kws->audio_block_size * 4)
  {
	  Audiostate.state = BUFFER_OFFSET_FULL;
	  Audiostate.fptr = 0;
  }
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
  BSP_AUDIO_IN_PDMToPCM((uint16_t*)&recbuff[0],(uint16_t*) (kws->audio_buffer_in + Audiostate.fptr));
  Audiostate.fptr+= INTERNAL_BUFF_SIZE/4/2;

  if( Audiostate.fptr == kws-> audio_block_size * 2)
  {
	  Audiostate.state = BUFFER_OFFSET_HALF;
  }

  if( Audiostate.fptr  == kws->audio_block_size * 4)
  {
	  Audiostate.state = BUFFER_OFFSET_FULL;
	  Audiostate.fptr = 0;
  }
}
