#include <SmartResponseXE.h>

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#define MAX_THRESHOLD 15
#define MIN_THRESHOLD 0

#define MAX_CHANNEL 26
#define MIN_CHANNEL 11

int cca_threshold = 15;
int cca_status = -1;
int cca_channel = 11;
int done = 0;
void setup()
{
  screen_setup();
  //ghost bars
  for(int x=0;x<16;x++)
      draw_bar(x, 15,1);
  //initial setup
  rfBeginSniff(11);
}

void loop()
{
  //char lcd_out[64];
  byte keypressed = 0x00;
  int ctr = 0;
  int watch_dog = 0;
  set_CCA();
  while(1)
  {
    //delay(5);
    if(ctr > 10000)
    {
      keypressed = SRXEGetKey();
      //sprintf(lcd_out,"button pressed:%d %02X",keypressed,keypressed);
      //SRXEWriteString(0,10,lcd_out, FONT_NORMAL, 3, 0);
      if(keypressed == 0xF0)//top left button
      {
        SRXESleep();
      }
      ctr=0;
    }

    if(done == 1)//if done
    {
      watch_dog = 0;
      done = 0;
      if(cca_status == 0)//we have it
      {
        //clear out the previous
        draw_bar((cca_channel - MIN_CHANNEL),cca_threshold,1);
        //draw the bar graph
        draw_bar((cca_channel - MIN_CHANNEL),cca_threshold,3);
        //reset the threshold
        cca_threshold = MAX_THRESHOLD;
      }
      else
        cca_threshold--;//drop the threshold and check again

      //if the threshold is too low reset to the max(which triggers a channel change)
      if(cca_threshold < MIN_THRESHOLD)
      {
        cca_threshold = MAX_THRESHOLD;
        //clear out the previous
        draw_bar((cca_channel - MIN_CHANNEL),cca_threshold,1);
      }

      if(cca_threshold == MAX_THRESHOLD)
      {
        cca_channel++;
        if(cca_channel > MAX_CHANNEL)
        {
          cca_channel = MIN_CHANNEL;
          //channel reset so reset the bar
          //ghost bars
          //for(int x=0;x<16;x++)
          //    draw_bar(x, 15,1);
        }
        //change the channel
        rfChannel(cca_channel);
      }
      //listen again
      set_CCA();
    }
    else
    {
      watch_dog++;
      if(watch_dog > 10000)
      {
        //reset!?!
        watch_dog = 0;
        cca_threshold = 15;
        cca_status = -1;
        cca_channel = 11;
        done = 0;
        setup();
        set_CCA();
      }
    }
    ctr++;
  }//main loop
/**/
}

uint8_t rfBeginSniff(uint8_t channel)
{

  // Transceiver Pin Register -- TRXPR.
  // This register can be used to reset the transceiver, without
  // resetting the MCU.
  TRXPR |= (1<<TRXRST);   // TRXRST = 1 (Reset state, resets all registers)

  // Transceiver Interrupt Enable Mask - IRQ_MASK
  // This register disables/enables individual radio interrupts.
  // First, we'll disable IRQ and clear any pending IRQ's
  IRQ_MASK = 0;  // Disable all IRQs
  
  // Transceiver State Control Register -- TRX_STATE
  // This regiseter controls the states of the radio.
  // First, we'll set it to the TRX_OFF state.
  TRX_STATE = (TRX_STATE & 0xE0) | TRX_OFF;  // Set to TRX_OFF state
  delay(1);
  
  // Transceiver Status Register -- TRX_STATUS
  // This read-only register contains the present state of the radio transceiver.
  // After telling it to go to the TRX_OFF state, we'll make sure it's actually
  // there.
  if ((TRX_STATUS & 0x1F) != TRX_OFF) // Check to make sure state is correct
    return 0;  // Error, TRX isn't off

  // Transceiver Control Register 1 - TRX_CTRL_1
  // We'll use this register to turn on automatic CRC calculations.
  TRX_CTRL_1 |= (1<<TX_AUTO_CRC_ON);  // Enable automatic CRC calc. 

  XAH_CTRL_1 |= (1<<AACK_PROM_MODE); //promiscuous mode

  CSMA_SEED_1 |= (1<<AACK_DIS_ACK); //disable generation of acknowledgment

  CSMA_SEED_1 |= (1<<AACK_FVN_MODE0); //ACK behaviour FCF
  CSMA_SEED_1 |= (1<<AACK_FVN_MODE1); //ACK behaviour FCF
  
  // Enable RX start/end and TX end interrupts
  IRQ_MASK = (1<<RX_START_EN) | (1<<RX_END_EN) | (1<<CCA_ED_DONE);// | (1<<TX_END_EN);
  
  // Transceiver Clear Channel Assessment (CCA) -- PHY_CC_CCA
  // This register is used to set the channel. CCA_MODE should default
  // to Energy Above Threshold Mode.
  // Channel should be between 11 and 26 (2405 MHz to 2480 MHz)
  if ((channel < 11) || (channel > 26)) channel = 11;  
  PHY_CC_CCA = (PHY_CC_CCA & 0xE0) | channel;
  
  // Finally, we'll enter into the RX_ON state. Now waiting for radio RX's, unless
  // we go into a transmitting state.
  TRX_STATE = (TRX_STATE & 0xE0) | RX_ON; // Default to receiver

  return 1;
}
//clear channel assessment (CCA)
void set_CCA()
{
  //cca_status = -1;
  //set the ED threshold
  //C7 0b1100 0111
  if(CHECK_BIT(cca_threshold,0))
    CCA_THRES |= (1 << CCA_ED_THRES0);//set
  else
    CCA_THRES &= ~(1 << CCA_ED_THRES0);//unset

  if(CHECK_BIT(cca_threshold,1))
    CCA_THRES |= (1 << CCA_ED_THRES1);//set
  else
    CCA_THRES &= ~(1 << CCA_ED_THRES1);//unset

  if(CHECK_BIT(cca_threshold,2))
    CCA_THRES |= (1 << CCA_ED_THRES2);//set
  else
    CCA_THRES &= ~(1 << CCA_ED_THRES2);//unset

  if(CHECK_BIT(cca_threshold,3))
    CCA_THRES |= (1 << CCA_ED_THRES3);//set
  else
    CCA_THRES &= ~(1 << CCA_ED_THRES3);//unset

  int cca_thres = CCA_THRES;
  char lcd_out[64];memset(lcd_out,0x00,64);
  //sprintf(lcd_out,"c:%d cca_t:%02X cca_thr:%d done:%d\n",cca_channel,cca_thres,cca_threshold,done);
  sprintf(lcd_out,"channel:%d",cca_channel);
  SRXEWriteString(0,0,lcd_out, FONT_NORMAL, 3, 0);
  
  //cca_settings.channel = 11;
  //cca_settings.cca_mode = 2;
  //cca_settings.cca_request = 1;
  
  PHY_CC_CCA |= (1 << CCA_MODE0);//set
  //PHY_CC_CCA &= ~(1 << CCA_MODE0);//unset
  //PHY_CC_CCA |= (1 << CCA_MODE1);//set
  PHY_CC_CCA &= ~(1 << CCA_MODE1);//unset
  
  //int phy_cc_cca = PHY_CC_CCA;
  //char lcd_out[64];memset(lcd_out,0x00,64);
  //sprintf(lcd_out,"c:%d phy_cc_cca:%02X \n",cca_channel,phy_cc_cca);
  //SRXEWriteString(0,0,lcd_out, FONT_NORMAL, 3, 0);
  //__reg_PHY_CC_CCA cca_settings;
  //set the CCA request
  PHY_CC_CCA |= (1 << CCA_REQUEST);
}

ISR(TRX24_CCA_ED_DONE_vect)
{
  //SRXEWriteString(10,10,"CCA End", FONT_NORMAL, 3, 0);

  cca_status = (TRX_STATUS >> CCA_STATUS) & 1;
  int cca_done = (TRX_STATUS >> CCA_DONE) & 1;
  int trx_status = TRX_STATUS;
  //C6 = 0x11000110
  done = 1;
  //char lcd_out[64];memset(lcd_out,0x00,64);
  //sprintf(lcd_out,"c:%d TRX_S:%02X status:%d done:%d done:%d\n",cca_channel,trx_status,cca_status,cca_done,done);
  //SRXEWriteString(0,10,lcd_out, FONT_NORMAL, 3, 0);
  //clear the request?
  //PHY_CC_CCA |= (0 << CCA_REQUEST);
  PHY_CC_CCA &= ~(1 << CCA_REQUEST);

}

void draw_bar(uint8_t channel, uint8_t threshold, uint8_t color)
{
  int x_start = 30 + (channel * 4);
  int y_offset = threshold * 5;
  int y_max = 120;
  int y_start = y_max - y_offset;

  SRXERectangle(x_start, y_start, 2, y_offset, color, 1);
}

uint8_t rfChannel(uint8_t channel)
{
  // Transceiver Clear Channel Assessment (CCA) -- PHY_CC_CCA
  // This register is used to set the channel. CCA_MODE should default
  // to Energy Above Threshold Mode.
  // Channel should be between 11 and 26 (2405 MHz to 2480 MHz)
  if ((channel < 11) || (channel > 26)) channel = 11;
  PHY_CC_CCA = (PHY_CC_CCA & 0xE0) | channel; // Set the channel to 11
  
  return 1;
}

void screen_setup()
{
  // CS, D/C, RESET
  SRXEInit(0xe7, 0xd6, 0xa2); // initialize display
}
