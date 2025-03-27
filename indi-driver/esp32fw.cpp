#include "config.h"
#include "esp32fw.h"

#include "indicom.h"

#include <cstring>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
//#include <termios.h>


// We declare an auto pointer to esp32fw.
static std::unique_ptr<ESP32FW> esp32fw(new ESP32FW());

ESP32FW::ESP32FW()
{
    setVersion(0, 1);
    setFilterConnection(CONNECTION_TCP);
}

const char *ESP32FW::getDefaultName()
{
    return "ESP32 Filter Wheel";
}

void ESP32FW::ISGetProperties(const char *dev)
{
    INDI::FilterWheel::ISGetProperties(dev);

    // Only read value when we're offline
    if (isConnected() == false)
    {
        double maxCount = 5;
        IUGetConfigNumber(getDeviceName(), "MAX_FILTER", "Count", &maxCount);
        FilterSlotNP[0].setMax(maxCount);
        if (FilterNameTP.size() != maxCount)
        {
            char filterName[MAXINDINAME];
            char filterLabel[MAXINDILABEL];

            FilterNameTP.resize(0);

            for (int i = 0; i < maxCount; i++)
            {
                snprintf(filterName, MAXINDINAME, "FILTER_SLOT_NAME_%d", i + 1);
                snprintf(filterLabel, MAXINDILABEL, "Filter#%d", i + 1);

                INDI::WidgetText oneText;
                oneText.fill(filterName, filterLabel, filterLabel);
                FilterNameTP.push(std::move(oneText));
            }

            FilterNameTP.fill(getDeviceName(), "FILTER_NAME", "Filter",
                              FilterSlotNP.getGroupName(), IP_RW, 0, IPS_IDLE);
            FilterNameTP.shrink_to_fit();
        }
    }
    defineProperty(&MaxFilterNP);
}

bool ESP32FW::initProperties()
{
    INDI::FilterWheel::initProperties();

    HomeSP[0].fill("Find", "Find", ISS_OFF);
    HomeSP.fill(getDeviceName(),"HOME", "Home", FILTER_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);

    IUFillNumber(&MaxFilterN[0], "Count", "Count", "%.f", 1, 16, 1, 5);
    IUFillNumberVector(&MaxFilterNP, MaxFilterN, 1, getDeviceName(), "MAX_FILTER", "# of filters", MAIN_CONTROL_TAB, IP_RW, 60,
                       IPS_IDLE);

    CurrentFilter = 1;
    FilterSlotNP[0].setMin(1);
    FilterSlotNP[0].setMax(5);

    setPollingPeriodRange(1000, 30000);
    setDefaultPollingPeriod(2000);

    addAuxControls();

    return true;
}

bool ESP32FW::updateProperties()
 {
    INDI::FilterWheel::updateProperties();

    if (isConnected())
        defineProperty(HomeSP);
    else
        deleteProperty(HomeSP);

    return true;
 }

bool ESP32FW::SelectFilter(int f)
{
    int digits = 2;
    std::string TargetFilter = std::to_string(f);
    int zeros =  digits - TargetFilter.length();
    int timeout = 70;

    TargetFilter = std::string(2 - std::min(2, zeros), '0') + TargetFilter;

    char res[30] = {0};
    std::string cmd = "SF*";
    cmd = cmd + TargetFilter + "\n";
    int rc = -1, nbytes_written = 0;

    LOGF_DEBUG("CMD = SF*%s#", TargetFilter);

    if (isSimulation())
        snprintf(res, 8, "%d", f - 1);
    else
    {
        if ((rc = tty_write_string(PortFD, cmd.c_str(), &nbytes_written)) != TTY_OK)
        {
            char error_message[ERRMSG_SIZE];
            tty_error_msg(rc, error_message, ERRMSG_SIZE);

            LOGF_ERROR("Sending command select filter failed: %s", error_message);
            return false;
        }
       FilterSlotNP[0].setValue(-1); // Signal filter is being changed
       FilterSlotNP.setState(IPS_BUSY);
       FilterSlotNP.apply();
       f = -1;
       while (f < 1)
        {
         std::this_thread::sleep_for(std::chrono::seconds(1));
         timeout--;
         f = QueryFilter(true);
         if (timeout < 1)
          {
           LOGF_ERROR("Timeout trying to change filter %s", TargetFilter);
           return false;
          }
        }
       return true;
    }
    return false;
}

int ESP32FW::QueryFilter(bool internal)
 {
  char res[16] = {0};
  int pos = -1;
  int f = 0;
  char stop = {'#'};
  std::string wh = "";
  std::string cmd = "WH*\n";
  int rc = -1, nbytes_written = 0, nbytes_read = 0;

  if (isSimulation())
   {
    snprintf(res, 8, "%d", 1);
    return 1;
   }
  if ((rc = tty_write_string(PortFD, cmd.c_str(), &nbytes_written)) != TTY_OK)
   {
    char error_message[ERRMSG_SIZE];
    tty_error_msg(rc, error_message, ERRMSG_SIZE);
    LOGF_ERROR("Sending command query filter failed: %s", error_message);
    return -2;
   }
  if ((rc = tty_read_section(PortFD, res, stop, 1, &nbytes_read)) == TTY_OK)
   {
    LOGF_DEBUG("RES <%s>", res);
    std::string resS = res;
    pos = resS.find("WH*");
    f = std::stoi(resS.substr(pos+3,2));
if (f == 0) //need to "fix" this in filter wheel firmware
 {
  f = -1;
 }
    if (!internal)
     {
      LOGF_DEBUG("internal =  <%d>", internal);
      FilterSlotNP[0].setValue(f);
     }
    if (f > 0)
     {
      if (!internal)
       {
        HomeSP.reset();
        HomeSP.setState(IPS_OK);
        HomeSP.apply();
        FilterSlotNP.setState(IPS_OK);
        FilterSlotNP.apply();
       }
      LOGF_DEBUG("f > 0, f = <%d>", f);
      return f;
     }
    if (!internal)
     {
      FilterSlotNP.setState(IPS_BUSY);
      FilterSlotNP.apply();
     }
    return f;
   }
  else
   {
    LOGF_DEBUG("Could not read position from filter wheel", res);
    return -2;
   }
 }

bool ESP32FW::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (!strcmp(name, MaxFilterNP.name))
        {
            if (values[0] != MaxFilterN[0].value)
            {
                IUUpdateNumber(&MaxFilterNP, values, names, n);
                saveConfig();
                LOG_INFO("Max number of filters updated. You must reconnect for this change to take effect.");
            }
            IDSetNumber(&MaxFilterNP, nullptr);
            return true;
        }

    }

    return INDI::FilterWheel::ISNewNumber(dev, name, values, names, n);
}

bool ESP32FW::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
 {
  if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
   {
    if (HomeSP.isNameMatch(name))
     {
      FilterSlotNP.setState(IPS_BUSY);
      FilterSlotNP.apply();
      if (home())
       {
        LOG_INFO("Filter wheel sent to find home position");
       }
      else
       {
        HomeSP.setState(IPS_ALERT);
        HomeSP.apply();
       }
      return true;
     }
   }
  return INDI::FilterWheel::ISNewSwitch(dev, name, states, names, n);
 }

bool ESP32FW::saveConfigItems(FILE *fp)
{
    FilterWheel::saveConfigItems(fp);

    IUSaveConfigNumber(fp, &MaxFilterNP);

    return true;
}

bool ESP32FW::home()
 {
  char res[16] = {0};
  std::string cmd = "CMD = HM*\n";
  int rc = -1, nbytes_written = 0;

  LOGF_DEBUG("*HM",nbytes_written);
  HomeSP.setState(IPS_BUSY);
  HomeSP.apply();
  FilterSlotNP.setState(IPS_BUSY);
  FilterSlotNP.apply();

  if (isSimulation())
   {
    snprintf(res, 8, "%d", 1);
    return true;
   }
  else
   {
    if ((rc = tty_write_string(PortFD, cmd.c_str(), &nbytes_written)) != TTY_OK)
     {
      char error_message[ERRMSG_SIZE];
      tty_error_msg(rc, error_message, ERRMSG_SIZE);
      LOGF_ERROR("Sending command to move to home position failed: %s", error_message);
      return false;
     }
   }
  return true;
 }

void ESP32FW::TimerHit()
 {
  if (!isConnected())
   {
    SetTimer(getCurrentPollingPeriod());
    return;
   }

  // Update filter wheel position - also used as keepalive
  QueryFilter(false);

  // Restart timer
  SetTimer(getCurrentPollingPeriod());
 }
