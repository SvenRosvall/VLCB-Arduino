#include <CBUSConfig.h>

class CBUSParams
{
public:
  CBUSParams(CBUSConfig config)
    : params(new unsigned char[21])
  {
    params[0] = 20;
    params[1] = MANU_MERG;
    params[4] = config.EE_MAX_EVENTS;   //  4 num events
    params[5] = config.EE_NUM_EVS;      //  5 num evs per event
    params[6] = config.EE_NUM_NVS;      //  6 num NVs
    params[10] = PB_CAN;
    params[11] = 0x00;
    params[12] = 0x00;
    params[13] = 0x00;
    params[14] = 0x00;
  }

  void setVersion(char major, char minor, char beta)
  {
    params[7] = major;
    params[2] = minor;
    params[20] = beta;
  }

  void setModuleId(byte id)
  {
    params[3] = id;
  }

  void setFlags(byte flags)
  {
    params[8] = flags;
  }

  void setProcessor(byte manufacturer, byte id, char const * name)
  {
    params[9] = id;
    params[19] = manufacturer;
    strncpy(params + 15, name, 4);
  }

  unsigned char * getParams()
  {
    return params;
  }
private:
  // Memory for the params is allocated on the heap and handed over to CBUS.setParams().
  // This memory will never be freed.
  unsigned char * params;
};
