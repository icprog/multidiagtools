/****************************************************************************
 **
 ** Copyright (C) 2011-2012 Philippe Steinmann.
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
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QMap>

class QTimer;

/*! \brief Base class for a device connected to a port
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
                Busy,                   /*!< Device is connected but cannot accept requests for the moment */
                Unknown                 /*!< Unknown (and unhandled) state */
               };

  mdtDevice(QObject *parent = 0);
  virtual ~mdtDevice();

  /*! \brief Set the device name
   */
  void setName(const QString &name);

  /*! \brief Get device name
   */
  QString name() const;

  /*! \brief Set the I/O's container
   *
   * \param ios A pointer to a mdtDeviceIos object
   * \param autoAnalogOutputUpdate If true, mdtAnalogIo::valueChanged() signal will be connected to
   *                                setAnalogOutputValue() slot for each analog output
   *
   * \pre ios must be a valid pointer
   */
  void setIos(mdtDeviceIos *ios, bool autoAnalogOutputUpdate = false);

  /*! \brief Set reply timeout for outputs requests
   *
   * If timeout is < 0, no reply is expected.
   *
   * \param timeout Timeout [ms]
   */
  void setOutputWriteReplyTimeout(int timeout);

  /*! \brief Set output address offset
   *
   * Some devices has a offset for output addresses.
   * F.ex. the Wago 750 system has a offset of 0x0200
   */
  void setAnalogOutputAddressOffset(int offset);

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

  /*! \brief Read one analog input on physical device and update (G)UI representation
   *
   * \param address Depending on device organisation and protocol,
   *                 this can be a relative or absolute address (f.ex. MODBUS queries),
   *                 a input number, etc...
   * \return true on success, false else.
   *
   * Subclass notes:<br>
   *  - This default implementation does nothing and returns allways false.
   *  - If device handled by subclass has analog inputs, this method should be implemented.
   *  - The subclass must handle and document the behaviour of calling this method without any I/O's container set.
   *  - To update (G)UI, the mdtDeviceIos::updateAnalogInputValue() should be used.
   */
  virtual bool readAnalogInput(int address);

  /// \todo Version "blocante" (ret: QVariant, attente avec portmanager, ...)

  /*! \brief Read all analog inputs on physical device and update (G)UI representation
   *
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   *
   * Subclass notes:<br>
   *  - This default implementation does nothing and returns allways -1.
   *  - If device handled by subclass has analog inputs, this method should be implemented.
   *  - The subclass must handle and document the behaviour of calling this method without any I/O's container set.
   *  - To update (G)UI, mdtDeviceIos::updateAnalogInputValues() should be used.
   *  - Helper method setStateFromPortError() can be used to update device state on error.
   */
  virtual int readAnalogInputs();

  /// \todo Version "blocante" (ret: QList<QVariant>, attente avec portmanager, ...)

  /*! \brief Read all analog outputs on physical device and update (G)UI representation
   *
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   *
   * Subclass notes:<br>
   *  - This default implementation does nothing and returns allways -1.
   *  - If device handled by subclass has analog inputs, this method should be implemented.
   *  - The subclass must handle and document the behaviour of calling this method without any I/O's container set.
   *  - To update (G)UI, mdtDeviceIos::updateAnalogOutputValues() should be used.
   *  - Helper method setStateFromPortError() can be used to update device state on error.
   */
  virtual int readAnalogOutputs();

  /*! \brief Set value on a analog output on physical device
   *
   * \param address Output address
   * \param value Value encoded regarding device format
   * \param confirmationTimeout If > 0, a confirmation frame is expected from device, else not.
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   *
   * Subclass notes:<br>
   *  - This default implementation does nothing and returns allways -1.
   *  - If device handled by subclass has analog inputs, this method should be implemented.
   *  - If device returns a confirmation, helper method mdtPortManager::waitOnFrame() can be used
   *  - The subclass must handle and document the behaviour of calling this method without any I/O's container set.
   *  - To update (G)UI, mdtDeviceIos::updateAnalogOutputValue() should be used.
   *  - Helper method setStateFromPortError() can be used to update device state on error.
   */
  virtual int writeAnalogOutputValue(int address, int value, int confirmationTimeout);

  /*! \brief Get device state
   */
  state_t state() const;

 signals:

  /*! \brief Emited when state has changed
   */
  void stateChanged(state_t newState);

 public slots:

  /*! \brief Set value on a analog output on physical device
   *
   * Checks device's state and send request if Ready.
   *  Internally, writeAnalogOutputValue() is called.
   */
  void setAnalogOutputValue(int address, int value);

  /*! \brief Decode incoming frames
   *
   * Subclass notes:<br>
   *  - This default implementation does nothing.
   *  - This slot should be connected with mdtPortManager::newReadenFrame(int, QByteArray) signal.
   *  - In this class, this connection is not made, it is the sublcass responsability to do this.
   *  - The incoming frames are available with mdtPortManager::readenFrames().
   */
  virtual void decodeReadenFrames(int id, QByteArray data);

  /*! \brief Queries to send periodically
   *
   * This slot is periodically called after a call of start().
   */
  void runQueries();

 protected slots:

  /*! \brief Set the ready state
   *
   * Emit stateChanged() if current state was not Ready.
   */
  void setStateReady();

 protected:

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

  /*! \brief Set the disconnected state
   *
   * Emit stateChanged() if current state was not Disconnected.
   */
  void setStateDisconnected();

  /*! \brief Set the busy state
   *
   * Busy state can be used when physical device or computer (this software) cannot process more requests.
   * Emit stateChanged() if current state was not Busy.
   *
   * \param retryTimeout If >= 0, device will automatically go back to ready state after this timeout [ms]
   */
  void setStateBusy(int retryTimeout = -1);

  /*! \brief Set the unknown state
   *
   * Will add a error to mdtError system.
   * Emit stateChanged() if current state was not Unknown.
   */
  void setStateUnknown();

  /*! \brief Update device state regarding port error.
   *
   * If error is unknown, the state is changed to Unknown.
   */
  void setStateFromPortError(int error);

  mdtDeviceIos *pvIos;    // I/O's container
  QMap<int, mdtAnalogIo*> pvAoPendingTransactions;
  int pvOutputWriteReplyTimeout;
  int pvAnalogOutputAddressOffset;

 private:

  Q_DISABLE_COPY(mdtDevice);

  state_t pvCurrentState;
  QString pvName;
  QTimer *pvQueryTimer;
};

#endif  // #ifndef MDT_DEVICE_H
