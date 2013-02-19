/**
 * An Mirf example which copies back the data it recives.
 *
 * Pins:
 * Hardware SPI:
 * MISO -> 12
 * MOSI -> 11
 * SCK -> 13
 *
 * Configurable:
 * CE -> 8
 * CSN -> 7
 *
 */

#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

void setup(){
//  Serial.begin(9600);
  /*
   * Set the SPI Driver.
   */
  Mirf.cePin = 9;
  Mirf.csnPin = 10;

  Mirf.spi = &MirfHardwareSpi;
  
  /*
   * Setup pins / SPI.
   */
   
  Mirf.init();
  
  /*
   * Configure reciving address.
   */
   
  Mirf.setRADDR((byte *)"serv1");
  
  /*
   * Set the payload length to sizeof(unsigned long) the
   * return type of millis().
   *
   * NB: payload on client and server must be the same.
   */
   
//  Mirf.payload = sizeof(unsigned long);
  Mirf.payload = 1;
  Mirf.channel = 90; // Just because   
  Mirf.config();
  
}

void loop(){
   
  byte data[Mirf.payload];
  
  /*
   * If a packet has been recived.
   *
   * isSending also restores listening mode when it 
   * transitions from true to false.
   */
   
  if(!Mirf.isSending() && Mirf.dataReady()){
    
    /*
     * Get load the packet into the buffer.
     */
    char c;
    Mirf.getData((byte *) &c);
//    Serial.print("gotten data is:");
//    Serial.println(c);
    
    /*
     * Set the send address.
     */
     
     
    Mirf.setTADDR((byte *)"clie1");
    
    /*
     * Send the data back to the client.
     */
     
    Mirf.send((byte *) &c);
    
    /*
     * Wait untill sending has finished
     *
     * NB: isSending returns the chip to receving after returning true.
     */
      
  }
}
