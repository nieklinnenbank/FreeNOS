/**
 * Serial port console debug log for Intel.
 *
 * @author Niek Linnenbank
 * @date 5 march 2015
 */

#ifndef _INTEL_SERIAL_H
#define _INTEL_SERIAL_H

#include <Log.h>

/** Forward declare */
class IntelIO;

/**
 * Intel Serial port debug log class.
 *
 * @note This class is a singleton
 */
class IntelSerial : public Log
{
  public:

    /**
     * Constructor.
     */
    IntelSerial(u16 base);

  protected:

    /**
     * Write to the actual output device.
     */
    virtual void write(const char *str);

  private:

    /**
     * Constants used to communicate with the UART.
     */
    enum IntelSerialReg
    {
        TRANSMIT     = 0,
        RECEIVE      = 0,
        DIVISORLOW   = 0,
        DIVISORHIGH  = 1,
        RXREADY      = 1,
        IRQCONTROL   = 1,
        IRQSTATUS    = 2,
        FIFOCONTROL  = 2,
        LINECONTROL  = 3,
        MODEMCONTROL = 4,
        LINESTATUS   = 5,
        TXREADY      = 0x20,
        DLAB         = 0x80,
        BAUDRATE     = 9600,
    };

    /** I/O instance */
    IntelIO m_io;
};

#endif /* _INTEL_SERIAL_H */
