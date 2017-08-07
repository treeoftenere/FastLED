#ifndef __INC_TENERE_CONTROLLER_H
#define __INC_TENERE_CONTROLLER_H


FASTLED_NAMESPACE_BEGIN

#define REVC_PORT_MASK 0xE0F8

template<uint8_t CLOCK_PIN, EOrder RGB_ORDER, uint8_t SPI_SPEED>
class CTenereController : public CPixelLEDController<RGB_ORDER, 8, 0xFF> {

  typedef typename FastPin<CLOCK_PIN>::port_ptr_t data_ptr_t;
  typedef typename FastPin<CLOCK_PIN>::port_t data_t;

public:
  CTenereController() { }

  virtual void init() { 
    // diable jtag so we get 2 extra pins
    afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY);

      FastPin<CLOCK_PIN>::setOutput();
    // switch (REVISION) {
    //   case TENERE_REVB:
      //   FastPin<PB13>::setOutput();
      //   FastPin<PB14>::setOutput();
      //   FastPin<PB15>::setOutput();
      //   FastPin<PB5>::setOutput(); 
      //   FastPin<PB6>::setOutput(); 
      //   FastPin<PB7>::setOutput(); 
      //   FastPin<PB8>::setOutput(); 
      //   FastPin<PB9>::setOutput();
      //   break; 
      // case TENERE_REVC:
        FastPin<PB7>::setOutput(); 
        FastPin<PB6>::setOutput(); 
        FastPin<PB5>::setOutput(); 
        FastPin<PB4>::setOutput(); 
        FastPin<PB3>::setOutput(); 
        FastPin<PB15>::setOutput();
        FastPin<PB14>::setOutput();
        FastPin<PB13>::setOutput();
    //     break;
    // }
  }

  typedef union {
    uint8_t bytes[8];
    uint32_t raw[2];
  } Lines;

  virtual void showPixels(PixelController<RGB_ORDER, 8, 0xFF> & pixels) {
    
    writeByte(0x00); 
    writeByte(0x00); 
    writeByte(0x00); 
    writeByte(0x00);    
    
    while(pixels.has(1)) {
      Lines b;
      Lines b2;

      writeByte(0xFF);

      int i;
      for(i = 0; i < 8; i++) { b.bytes[i] = pixels.loadAndScale0(i); }
      transpose8x1_MSB(b.bytes,b2.bytes);
      writeByte(b2);
      for(i = 0; i < 8; i++) { b.bytes[i] = pixels.loadAndScale1(i); }
      transpose8x1_MSB(b.bytes,b2.bytes);
      writeByte(b2);
      for(i = 0; i < 8; i++) { b.bytes[i] = pixels.loadAndScale2(i); }
      transpose8x1_MSB(b.bytes,b2.bytes);
      writeByte(b2);

      pixels.stepDithering();
      pixels.advanceData();
    }

    int nDWords = (pixels.size()/32); 
    do { 
        writeByte(0xFF); 
        writeByte(0x00); 
        writeByte(0x00); 
        writeByte(0x00); 
    } while(nDWords--); 

  }


  // Writes the same byte to all lanes
  __attribute__((always_inline)) inline static void writeByte(uint8_t b) {
    writeBit<7>(b);
    writeBit<6>(b);
    writeBit<5>(b);
    writeBit<4>(b);
    writeBit<3>(b);
    writeBit<2>(b);
    writeBit<1>(b);
    writeBit<0>(b);
  }

  #define MIN_DELAY (NS(35) - 3)

  #define CLOCK_HI_DELAY delaycycles<MIN_DELAY>(); delaycycles<(((SPI_SPEED-6) / 2) - MIN_DELAY)>();
  #define CLOCK_LO_DELAY delaycycles<(((SPI_SPEED-6) / 4))>();


  template <uint8_t BIT> __attribute__((always_inline, hot)) inline static void writeBit(uint8_t b)
  {
    // this implicitly assumes clock port is the same as data pins port, which it
    // is for tenere
    if(b & (1 << BIT)) {
      *FastPin<CLOCK_PIN>::sport() = REVC_PORT_MASK;
      FastPin<CLOCK_PIN>::hi(); CLOCK_HI_DELAY;
      FastPin<CLOCK_PIN>::lo(); CLOCK_LO_DELAY;
    } else {
      *FastPin<CLOCK_PIN>::cport() = REVC_PORT_MASK;
      FastPin<CLOCK_PIN>::hi(); CLOCK_HI_DELAY;
      FastPin<CLOCK_PIN>::lo(); CLOCK_LO_DELAY;
    }

  }

  // Writes transposed data to all lanes
  __attribute__((always_inline)) inline static void writeByte(Lines b) {
    uint32_t d;    
      for(int i = 0; i < 8 ; i++) {
        // This makes so many assumptions on for tenere rev c. Oh well.
        d = *FastPin<CLOCK_PIN>::port();
        d &= ~REVC_PORT_MASK;
        d |= (b.bytes[i] & 31) << 3;
        d |= ((b.bytes[i] & 224) >> 5) << 13;
        FastPin<CLOCK_PIN>::fastset(FastPin<CLOCK_PIN>::port(), d);
        FastPin<CLOCK_PIN>::hi(); CLOCK_HI_DELAY;
        FastPin<CLOCK_PIN>::lo(); CLOCK_LO_DELAY;
      }
  }
};

FASTLED_NAMESPACE_END

#endif
