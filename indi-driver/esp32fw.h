#pragma once

#include "indifilterwheel.h"

class ESP32FW : public INDI::FilterWheel
 {
  public:
    ESP32FW();
    virtual void ISGetProperties(const char *dev) override;
    bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;

  protected:
    const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool saveConfigItems(FILE *fp) override;
    virtual bool Handshake() override
     {
      return true;
     }
    virtual bool SelectFilter(int) override;
    virtual int QueryFilter(bool);
    virtual void TimerHit() override;
    virtual bool updateProperties() override;

  private:
    INumber MaxFilterN[1];
    INumberVectorProperty MaxFilterNP;
    bool home();
//    ISwitch HomeS[1];
    INDI::PropertySwitch HomeSP {1};
 };


