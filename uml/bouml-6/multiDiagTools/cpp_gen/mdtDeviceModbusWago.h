#ifndef _MDTDEVICEMODBUSWAGO_H
#define _MDTDEVICEMODBUSWAGO_H


#include "mdtDeviceModbus.h"

class mdtDeviceInfo;
class mdtDeviceModbusWagoModule;

class mdtDeviceModbusWago : public mdtDeviceModbus {
  public:
    mdtDeviceModbusWago(const QObject & parent);

    ~mdtDeviceModbusWago();

  virtual mdtAbstractPort::error_t connectToDevice(const mdtPortInfo & portInfo);

    mdtAbstractPort::error_t connectToDevice(const mdtDeviceInfo & devInfo);


  private:
    <mdtDeviceModbusWagoModule> pvModules;


  public:
  mdtAbstractPort::error_t connectToDevice(const QList<mdtPortInfo*> & scanResult, int hardwareNodeId, int bitsCount, int startFrom);

};
#endif
