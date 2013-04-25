/****************************************************************************
 **
 ** Copyright (C) 2011-2013 Philippe Steinmann.
 **
 ** This file is part of multiDiagTools library.
 **
 ** multiDiagTools is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** multiDiagTools is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with multiDiagTools.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
#ifndef MDT_DEVICE_H
#define MDT_DEVICE_H

#include "mdtDeviceIos.h"
#include "mdtAbstractPort.h"
#include "mdtPortManager.h"
#include "mdtDeviceInfo.h"
#include <QObject>
#include <QByteArray>
#include <QVariant>
#include <QString>
#include <QStringList>

class QTimer;

/// \todo Adapt get...() methods like getAnalogInputValue() 
/*! \brief Base class for a device connected to a port
 *
 * Querying a device can be done several ways.
 *  Some devices sends a confirmation after each query.
 *
 * Some queries (f.ex. read a input value or state) give
 *  a result after query. In some applications, we need
 *  to display the reult priodically. In other application,
 *  we need to send the query and wait on result before
 *  we can continue. This base class gives a interface for
 *  these two cases.
 *
 * At basis, all query methods are not blocking by default.
 *  The normal way is to connect mdtPortManager::newReadenFrame()
 *  to decodeReadenFrame() slot (must be done in subclass).
 *  This way, a query will be sent to device and method returns.
 *  When device returns a result, mdtPortManager will send
 *  the newReadenFrame() signal, and decoding will be processed
 *  in decodeReadenFrame() and I/O's are updated.
 *
 * Because the blocking behaviour is sometimes needed, the concept
 *  of transaction is implemented. At first, we tell the query method
 *  that we will wait on a reply by passing a timeout > 0.
 *  Then, the query is sent to device and a transaction ID is added
 *  to a list with addTransaction(). At next, waitTransactionDone()
 *  is called, and finally result is returned.
 *  Note that waitTransactionDone() will not breack the Qt's event loop
 *  because it uses the mdtPortManager::wait() method (see documentation 
 *  of mdtPortManager for details).
 * 
 * Give some helper methods for event handling, ....
 *
 * If it is needed to continiusly update the representation
 *  of device's inputs (digital/analog I/O, voltage, temperature, ...),
 *  two things can happen:
 *  - 1) Used port can receive data continiusly (f.ex. RS232) and device send data automatically
 *  - 2) Used port need that a request is sent for each read (TCP, USB, ...) or device need such request
 * Continous update is typical usefull for a GUI representing a real device.
 *
 * A other usage is automated sequence. In this case, the sequence will tell when it need
 *  to set a output and when a input value is reuqierd.
 *
 * A device can accept one or many request befor results must be read. This depends most case
 *  on used port and protocol.
 *
 * It can be useful to find on witch port a device is attached (f.ex. setup of a automated test application).
 *
 * A device can have several states (ready, busy, disconnected, ...). To help the application programmer
 *  to keep coherence, this states are updated in this class.
 */
class mdtDevice : public QObject
{
 Q_OBJECT

 public:

  /*! \brief State of device
   */
  enum state_t {
                Ready = 0,              /*!< Device is connected and ready to receive queries */
                Disconnected,           /*!< Device is not connected */
                Connecting,             /*!< Trying to connect to device */
                Busy,                   /*!< Device is connected but cannot accept requests for the moment */
                Warning,                /*!< Device or port communication handled error occured */
                Error                   /*!< Device or port communication unhandled error occured */
               };

  mdtDevice(QObject *parent = 0);
  virtual ~mdtDevice();

  /*! \brief Set the device name
   */
  void setName(const QString &name);

  /*! \brief Get device name
   */
  QString name() const;

  /*! \brief Search and connect to physical device.
   *
   * Will scan available ports and open the first port that
   *  has device attached maching request.
   *
   * \param devInfo Requested device's informations.
   * \return A error listed in mdtAbstractPort::error_t (NoError on success)
   *
   * Note: default implementation does nothing and returns allways a UnhandledError.
   *        See subclass documentation for more details.
   */
  virtual mdtAbstractPort::error_t connectToDevice(const mdtDeviceInfo &devInfo);

  /*! \brief Set the I/O's container
   *
   * \param ios A pointer to a mdtDeviceIos object
   * \param autoOutputUpdate If true, mdtAnalogIo::valueChanged() signal will be connected to
   *                                setAnalogOutputValue(int, int) slot for each analog output
   *                            and mdtDigitalIo::stateChanged() signal will be connected to
   *                                setDigitalOutputState(int, bool) slot for each digital output.
   *
   * \pre ios must be a valid pointer
   */
  void setIos(mdtDeviceIos *ios, bool autoOutputUpdate = false);

  /*! \brief Set back to ready state timeout
   *
   * After some event (device disconnected, response timeout, ...)
   *  the state can be changed from Ready to another.
   *
   * Note: if state is different than Ready, the slots setAnalogOutputValue(int)
   *  and setDigitalOutputState(int) will return immediately, without
   *  trying to send a request.
   *
   * In some application, it can be usefull that state comes back
   *  automatically to ready.
   *
   * \param timeout Time before go back to ready state [ms]
   *                 If timeout is < 0, device still in current state,
   *                 and setStateReady() can be used.
   */
  void setBackToReadyStateTimeout(int timeout);

  /*! \brief Set analog output address offset
   *
   * Some devices has a offset for output addresses.
   * F.ex. the Wago 750 system has a offset of 0x0200
   */
  ///void setAnalogOutputAddressOffset(int offset);

  /*! \brief Set digital output address offset
   *
   * Some devices has a offset for output addresses.
   * F.ex. the Wago 750 system has a offset of 0x0200
   */
  ///void setDigitalOutputAddressOffset(int offset);

  /*! \brief Get internal port manager instance
   *
   * Note that port manager is set by subclass,
   *  and that a Null pointer can be returned.
   *  (See subclass documentation for details)
   */
  virtual mdtPortManager *portManager();

  /*! \brief Start periodic device querying
   *
   * Internally, the runQueries() slot will be called each queryInterval of time.
   *
   * \param queryInterval Query interval [ms]
   */
  void start(int queryInterval);

  /*! \brief Stop periodic device querying
   */
  void stop();

  /*! \brief Get analog input value
   *
   * Once result is available, the internal analog input is updated.
   *  (See mdtDeviceIos for details).
   *
   * \param address Depending on device organisation and protocol,
   *                 this can be a relative or absolute address (f.ex. MODBUS queries),
   *                 a input number, etc...
   * \param realValue If true, the real value (as double) is returned, else the device's specific integer value
   *                   (see mdtAnalogIo::value() for details).
   * \param queryDevice If true, value is readen from device by calling readAnalogInput(), else cached value is returned.
   *                      Behaviour of this method can vary, depending on device specific subclass.
   *                      (See sublcass readAnalogInput() for details).
   * \param waitOnReply If true, this method will wait until reply comes in, or timeout (See waitTransactionDone() ),
   *                     else it will return immediately after the query was sent.
   * 
   * \return A valid value if timeout is > 0 and on successfull query/reply process.
   *          (See the Qt's QVariant documentation to know how to check validity).
   */
  QVariant getAnalogInputValue(int address, bool realValue, bool queryDevice, bool waitOnReply);

  /*! \brief Read all analog inputs on physical device and update (G)UI representation
   *
   * Will call readAnalogInputs(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass readAnalogInputs() for details).
   *
   * Once results are available, the internal analog inputs are updated.
   *  (See mdtDeviceIos for details).
   *
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \return 0 or a ID on success. If no I/O's are set, on timeout ar other error, a value < 0 is returned.
   */
  int getAnalogInputs(int timeout);

  /*! \brief Read one analog output on physical device and update (G)UI representation
   *
   * Will call readAnalogOutput(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass readAnalogOutput() for details).
   *
   * Once result is available, the internal analog output is updated.
   *  (See mdtDeviceIos for details).
   *
   * \param address Depending on device organisation and protocol,
   *                 this can be a relative or absolute address (f.ex. MODBUS queries),
   *                 a input number, etc...
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 If < 0, no query is sent to device and cached value is returned.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \param realValue If true, the real value (as double) is returned, else the device's specific integer value
   *                   (see mdtAnalogIo::value() for details).
   * \return A valid value if timeout is > 0 and on successfull query/reply process.
   *          (See the Qt's QVariant documentation to know how to check validity).
   */
  QVariant getAnalogOutputValue(int address, int timeout, bool realValue = true);

  /*! \brief Read all analog outputs on physical device and update (G)UI representation
   *
   * Will call readAnalogOutputs(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass readAnalogOutputs() for details).
   *
   * Once results are available, the internal analog outputs are updated.
   *  (See mdtDeviceIos for details).
   *
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \return 0 or a ID on success. If no I/O's are set, on timeout ar other error, a value < 0 is returned.
   */
  int getAnalogOutputs(int timeout);

  /*! \brief Write one analog output to physical device and update (G)UI representation
   *
   * Will call writeAnalogOutput(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass writeAnalogOutput() for details).
   *
   * Once result is available, the internal analog output is updated.
   *  (See mdtDeviceIos for details).
   *
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 If < 0, the analog output is updated, but no query is sent to device.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \param value The value to set. If type is a integer, value will be passed as is.
   *               If type is bool, it will be converted by mdtAnalogIo.
   * \param address Depending on device organisation and protocol,
   *                 this can be a relative or absolute address (f.ex. MODBUS queries),
   *                 a output number, etc...
   * \return 0 or a ID on success. If no I/O's are set, on timeout, on invalid value or other error, a value < 0 is returned.
   */
  int setAnalogOutputValue(int address, QVariant value, int timeout);

  /*! \brief Write all analog outputs on physical device and update (G)UI representation
   *
   * To send the analog outputs values once, it's possible to set them without sending query.
   *  See mdtDeviceIos and mdtAnalogIo for details.
   *  Then, call this method to sent all analog outputs values to device.
   *
   * writeAnalogOutputs() will be called, witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass writeAnalogOutputs() for details).
   *
   * Once results are available, the internal analog outputs are updated.
   *  (See mdtDeviceIos for details).
   *
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \return 0 or a ID on success. If no I/O's are set, on timeout ar other error, a value < 0 is returned.
   */
  int setAnalogOutputs(int timeout);

  /*! \brief Read one digital input on physical device and update (G)UI representation
   *
   * Will call readDigitalInput(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass readDigitalInput() for details).
   *
   * Once result is available, the internal digital input is updated.
   *  (See mdtDeviceIos for details).
   *
   * \param address Depending on device organisation and protocol,
   *                 this can be a relative or absolute address (f.ex. MODBUS queries),
   *                 a input number, etc...
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 If < 0, no query will be sent to device and the cached state is returned.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \return A valid state if timeout is > 0 and on successfull query/reply process.
   *          (See the Qt's QVariant documentation to know how to check validity).
   */
  QVariant getDigitalInputState(int address, int timeout);

  /*! \brief Read all digital inputs on physical device and update (G)UI representation
   *
   * Will call readDigitalInputs(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass readDigitalInputs() for details).
   *
   * Once results are available, the internal digital inputs are updated.
   *  (See mdtDeviceIos for details).
   *
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \return 0 or a ID on success. If no I/O's are set, on timeout ar other error, a value < 0 is returned.
   */
  int getDigitalInputs(int timeout);

  /*! \brief Read one digital output on physical device and update (G)UI representation
   *
   * Will call readDigitalOutput(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass readDigitalOutput() for details).
   *
   * Once result is available, the internal digital output is updated.
   *  (See mdtDeviceIos for details).
   *
   * \param address Depending on device organisation and protocol,
   *                 this can be a relative or absolute address (f.ex. MODBUS queries),
   *                 a input number, etc...
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 If < 0, no query will be sent to device and the cached state is returned.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \return A valid state if timeout is > 0 and on successfull query/reply process.
   *          (See the Qt's QVariant documentation to know how to check validity).
   */
  QVariant getDigitalOutputState(int address, int timeout);

  /*! \brief Read all digital outputs on physical device and update (G)UI representation
   *
   * Will call readDigitalOutputs(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass readDigitalOutputs() for details).
   *
   * Once results are available, the internal digital outputs are updated.
   *  (See mdtDeviceIos for details).
   *
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \return 0 or a ID on success. If no I/O's are set, on timeout ar other error, a value < 0 is returned.
   */
  int getDigitalOutputs(int timeout);

  
  
  /*! \brief Set a digital output state
   *
   * Once result is available, the internal digital output is updated.
   *  (See mdtDeviceIos for details).
   *
   * \param address Depending on device organisation and protocol,
   *                 this can be a relative or absolute address (f.ex. MODBUS queries),
   *                 a input number, etc...
   * \param state State to set (ON/OFF).
   * \param writeToDevice If true, value is written to device by calling writeDigitalOutput(), else state is only cached.
   *                      Behaviour of this method can vary, depending on device specific subclass.
   *                      (See sublcass writeDigitalOutput() for details).
   * \param waitOnReply If true, this method will wait until reply comes in, or timeout (See waitTransactionDone() ),
   *                     else it will return immediately after the query was sent.
   * \return 0 or a ID on success. If no I/O's are set, on timeout, on invalid value or other error, a value < 0 is returned.
   */
  int setDigitalOutputState(int address, bool state, bool writeToDevice, bool waitOnReply);

  /*! \brief Set a digital output state
   *
   * Once result is available, the internal digital output is updated.
   *  (See mdtDeviceIos for details).
   *
   * \param labelShort I/O's short label.
   * \param state State to set (ON/OFF).
   * \param writeToDevice If true, value is written to device by calling writeDigitalOutput(), else state is only cached.
   *                      Behaviour of this method can vary, depending on device specific subclass.
   *                      (See sublcass writeDigitalOutput() for details).
   * \param waitOnReply If true, this method will wait until reply comes in, or timeout (See waitTransactionDone() ),
   *                     else it will return immediately after the query was sent.
   * \return 0 or a ID on success. If no I/O's are set, on timeout, on invalid value or other error, a value < 0 is returned.
   */
  int setDigitalOutputState(const QString &labelShort, bool state, bool writeToDevice, bool waitOnReply);

  
  
  /*! \brief Write all digital outputs to physical device and update (G)UI representation
   *
   * To send the digital outputs states once, it's possible to set them without sending query.
   *  See mdtDeviceIos and mdtDigitalIo for details.
   *  Then, call this method to sent all digital outputs states to device.
   *
   * writeDigitalOutputs() will be called, witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass writeDigitalOutputs() for details).
   *
   * Once results are available, the internal digital outputs are updated.
   *  (See mdtDeviceIos for details).
   *
   * \param timeout If 0, the request is sent and this method returns immediately.
   *                 Else it will wait until a reply comes in, or timeout.
   *                 (See waitTransactionDone() ).
   * \return 0 or a ID on success. If no I/O's are set, on timeout ar other error, a value < 0 is returned.
   */
  int setDigitalOutputs(int timeout);

  /*! \brief Get device state
   */
  state_t state() const;

 public slots:

  /*! \brief Set value on a analog output on physical device
   *
   * Checks device's state and send request if Ready.
   *
   * This slot is used by mdtDeviceIos to notify that a value has changed.
   *
   * Will call writeAnalogOutput(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass writeAnalogOutput() for details).
   *
   * Once result is available, the internal analog output is updated.
   *  (See mdtDeviceIos for details).
   *
   * \param address Depending on device organisation and protocol,
   *                 this can be a relative or absolute address (f.ex. MODBUS queries),
   *                 a output number, etc...
   */
  void setAnalogOutputValue(int address);

  /*! \brief Set state on a digital output on physical device
   *
   * Checks device's state and send request if Ready.
   *
   * This slot is used by mdtDeviceIos to notify that a state has changed.
   *
   * Will call writeDigitalOutput(), witch also sends the request to device.
   *  Depending on result, device's state can be updated.
   *
   * Behaviour of this method can vary, depending on device specific subclass.
   *  (See sublcass writeDigitalOutput() for details).
   *
   * Once result is available, the internal digital output is updated.
   *  (See mdtDeviceIos for details).
   *
   * \param address Depending on device organisation and protocol,
   *                 this can be a relative or absolute address (f.ex. MODBUS queries),
   *                 a output number, etc...
   */
  void setDigitalOutputState(int address);

  /*! \brief Queries to send periodically
   *
   * This slot is periodically called after a call of start().
   */
  void runQueries();

 protected slots:

  /*! \brief Decode incoming frames
   *
   * Subclass notes:
   *  - This default implementation does nothing.
   *  - This slot should be connected with mdtPortManager::newReadenFrame(mdtPortTransaction) signal.
   *  - In this class, this connection is not made, it is the sublcass responsability to do this.
   *  - The (G)UI should be updated using transaction's I/O (see mdtPortTransaction).
   */
  virtual void decodeReadenFrame(mdtPortTransaction transaction);

 protected:

  /*! \brief Read one analog input on physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has analog inputs, this method should be implemented.
   *
   * This method is called from getAnalogInput().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details).
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int readAnalogInput(mdtPortTransaction *transaction);

  /*! \brief Read all analog inputs on physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has analog inputs, this method should be implemented.
   *
   * This method is called from getAnalogInputs().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int readAnalogInputs(mdtPortTransaction *transaction);

  /*! \brief Read one analog output on physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has analog inputs, this method should be implemented.
   *
   * This method is called from getAnalogOutputValue().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int readAnalogOutput(mdtPortTransaction *transaction);

  /*! \brief Read all analog outputs on physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has analog inputs, this method should be implemented.
   *
   * This method is called from getAnalogOutputValue().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int readAnalogOutputs(mdtPortTransaction *transaction);

  /*! \brief Write value on a analog output to physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has analog outputs, this method should be implemented.
   *
   * This method is called from setAnalogOutputValue().
   *
   * \param value Value encoded regarding device format.
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int writeAnalogOutput(int value, mdtPortTransaction *transaction);

  /*! \brief Write all analog outputs to physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has analog outputs, this method should be implemented.
   *
   * This method is called from setAnalogOutputs().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int writeAnalogOutputs(mdtPortTransaction *transaction);

  /*! \brief Read one digital input on physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has digital inputs, this method should be implemented.
   *
   * This method is called from getDigitalInputState().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int readDigitalInput(mdtPortTransaction *transaction);

  /*! \brief Read all digital inputs on physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has digital inputs, this method should be implemented.
   *
   * This method is called from getDigitalInputs().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int readDigitalInputs(mdtPortTransaction *transaction);

  /*! \brief Read one digital output on physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has digital outputs, this method should be implemented.
   *
   * This method is called from getDigitalOutputState().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int readDigitalOutput(mdtPortTransaction *transaction);

  /*! \brief Read all digital outputs on physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has digital outputs, this method should be implemented.
   *
   * This method is called from getDigitalOutputs().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int readDigitalOutputs(mdtPortTransaction *transaction);

  /*! \brief Write state on a digital output to physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has digital outputs, this method should be implemented.
   *
   * This method is called from setDigitalOutputState().
   *
   * \param state State (ON/OFF).
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int writeDigitalOutput(bool state, mdtPortTransaction *transaction);

  /*! \brief Write all digital outputs to physical device
   *
   * This is the device specific implementation to send the query.
   *  If device handled by subclass has digital outputs, this method should be implemented.
   *
   * This method is called from setDigitalOutputs().
   *
   * \param transaction Contains some flags used during query/reply process (address, id, I/O object, ...).
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   * \pre I/O's must be set with setIos().
   * \pre transaction must be a valid pointer.
   */
  virtual int writeDigitalOutputs(mdtPortTransaction *transaction);

  /*! \brief Sequence of queries to send periodically
   *
   * This method is called from runQueries().
   *
   * \return true if all queries are sent successfully.
   *
   * Subclass notes:<br>
   *  - This default implementation does nothing and allways returns false.
   *  - This method can be reimplemented periodic queries must be sent to device.
   */
  virtual bool queriesSequence();

  /*! \brief Get a new transaction
   *
   * \sa mdtPortManager::getNewTransaction()
   *
   * \pre portManager must be set before calling this method
   */
  mdtPortTransaction *getNewTransaction();

  /*! \brief Restore a transaction into pool
   *
   * \sa mdtPortManager::restoreTransaction()
   *
   * \pre portManager must be set before calling this method
   */
  void restoreTransaction(mdtPortTransaction *transaction);

  /*! \brief Wait until a transaction is done without break the GUI's event loop
   *
   * \todo Adapt, comment
   *
   * This is a helper method that provide a blocking wait.
   *  Internally, a couple of sleep and event processing
   *  is done, avoiding freesing the GUI.
   *
   * Internally, mdtPortManager::waitOnFrame() is called.
   *
   * \param id Id returned by query method
   * \param timeout Timeout [ms]
   *                 If 0, device's defined read timeout is used (see mdtPortManager::waitOnFrame(int, int, int) for details).
   * \param granularity Sleep time between each call of event processing [ms]<br>
   *                     A little value needs more CPU and big value can freese the GUI.
   *                     Should be between 50 and 100, and must be > 0.
   *                     Note that msecs must be a multiple of granularity.
   * \return True on success, false on timeout. If id was not found in transactions list,
   *           a warning will be generated in mdtError system, and false will be returned.
   * \pre granularity must be > 0.
   */
  bool waitTransactionDone(int id, int timeout = 0, int granularity = 50);

  mdtDeviceIos *pvIos;    // I/O's container
  ///int pvDigitalOutputAddressOffset;
  ///int pvAnalogOutputAddressOffset;

 public slots:

  /*! \brief Used to show a message in status bar
   *
   * \param message Message to show
   * \param timeout If > 0, message will be cleared after timeout [ms]
   */
  void showStatusMessage(const QString &message, int timeout = 0);

  /*! \brief Used to show a message and details in status bar
   *
   * \param message Message to show
   * \param details Details to show
   * \param timeout If > 0, message will be cleared after timeout [ms]
   */
  void showStatusMessage(const QString &message, const QString &details, int timeout = 0);

 signals:

  /*! \brief Emitted when state has changed
   *
   * Typically used with mdtDeviceStatusWidget
   */
  void stateChanged(int state);

  /*! \brief Emitted when a new status message is to display
   *
   * Typically used with mdtDeviceStatusWidget
   */
  void statusMessageChanged(const QString &message, const QString &details, int timeout);

 private slots:

  /*! \brief Set the device state from port manager state
   */
  void setStateFromPortManager(int portManagerState);

  /*! \brief Set the disconnected state
   *
   * Emit stateChanged() if current state was not Disconnected.
   */
  void setStateDisconnected();

  /*! \brief Set the connecting state
   *
   * Emit stateChanged() if current state was not Connecting.
   */
  void setStateConnecting();

  /*! \brief Set the ready state
   *
   * Emit stateChanged() if current state was not Ready.
   */
  void setStateReady();

  /*! \brief Set the busy state
   *
   * Busy state can be used when physical device or computer (this software) cannot process more requests.
   * Emit stateChanged() if current state was not Busy.
   */
  void setStateBusy();

  /*! \brief Set the warning state
   *
   * Emit stateChanged() if current state was not Warning.
   */
  void setStateWarning();

  /*! \brief Set the error state
   *
   * Emit stateChanged() if current state was not Error.
   */
  void setStateError();

 private:

  Q_DISABLE_COPY(mdtDevice);

  QString pvName;
  QTimer *pvQueryTimer;
  bool pvAutoQueryEnabled;  // Flag used for state handling
  int pvBackToReadyStateTimeout;
  QTimer *pvBackToReadyStateTimer;
  // State flag
  state_t pvCurrentState;
};

#endif  // #ifndef MDT_DEVICE_H
