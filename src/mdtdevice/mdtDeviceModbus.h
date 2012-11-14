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
#ifndef MDT_DEVICE_MODBUS_H
#define MDT_DEVICE_MODBUS_H

#include "mdtDevice.h"
#include <QMap>
#include <QVariant>
#include <QList>
#include <QByteArray>

class mdtFrameCodecModbus;
class mdtModbusTcpPortManager;

/*! \brief Represent a MODBUS device
 *
 * References:
 *  - MODBUS Application Protocol Specification V1.1b
 *  - http://www.Modbus-IDA.org
 */
class mdtDeviceModbus : public mdtDevice
{
 Q_OBJECT

 public:

  mdtDeviceModbus(QObject *parent = 0);

  virtual ~mdtDeviceModbus();

  /*! \brief Get the value of a analog input
   *
   * When delaying the request, the currently stored value is returned.
   *
   * \param address Address as describe in MODBUS specification
   *                 (first input of a node has address 0, second has address 1, ...)
   *                 If address is out of range, a invalid value is returned (see setupAnalogInputs() ).
   * \param readDirectly If true, the request is sent to device directly, else it can be sent with readAnalogInputs()
   * \return The value of requested input, or an invalid value on error.
   */
  QVariant getAnalogInputValue(int address, bool readDirectly);

  /*! \brief Read all analog inputs on physical device and update (G)UI representation
   *
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   *
   * \pre I/O's container must be set with setIos()
   */
  int readAnalogInputs();

  /*! \brief Get the value of a analog output
   *
   * \param address Address as describe in MODBUS specification
   *                 (first output of a node has address 0, second has address 1, ...)
   *                 If address is out of range, a invalid value is returned (see setupAnalogOutputs() ).
   * \param readDirectly If true, the request is sent to device directly, else it can be sent with writeAnalogOutputs()
   * \return The value of requested output, or an invalid value on error.
   */
  virtual QVariant getAnalogOutputValue(int address, bool readDirectly);

  /*! \brief Write analog output values to device
   *
   * \return True on success.
   */
  //bool writeAnalogOutputs();

  /*! \brief Read all analog outputs on physical device and update (G)UI representation
   *
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   *
   * \pre I/O's container must be set with setIos()
   */
  int readAnalogOutputs();

  /*! \brief Set value on a analog output on physical device
   *
   * \param address Output address
   * \param value Value encoded regarding device format
   * \param confirmationTimeout If > 0, a confirmation frame is expected from device, else not.
   * \return 0 or a ID on success, value < 0 on error (see mdtPortManager::writeData() for details)
   *
   * Subclass notes:<br>
   *  - If device returns a confirmation, helper method mdtPortManager::waitOnFrame() can be used
   *  - The subclass must handle and document the behaviour of calling this method without any I/O's container set.
   *  - To update (G)UI, mdtDeviceIos::updateAnalogOutputValue() should be used.
   *  - Helper method setStateFromPortError() can be used to update device state on error.
   */
  int writeAnalogOutputValue(int address, int value, int confirmationTimeout);

  /*! \brief Get the state of a digital input
   *
   * When delaying the request, the currently stored state is returned.
   *
   * \param address Address as describe in MODBUS specification
   *                 (first input of a node has address 0, second has address 1, ...)
   *                 If address is out of range, a invalid value is returned (see setupDigitalInputs() ).
   * \param readDirectly If true, the request is sent to device directly, else it can be sent with readDigitalInputs()
   * \return The state of requested input, or an invalid value on error.
   */
  QVariant getDigitalInputState(int address, bool readDirectly);

  /*! \brief Request states from device
   *
   * Request states of all digital inputs.
   *
   * \return True on success.
   */
  bool readDigitalInputs();

  /*! \brief Set the number of digital outputs
   *
   * \param count Number of digital outputs
   * \pre count must be in range from 1 to 1968
   */
  ///void setupDigitalOutputs(int count);

  /*! \brief Set the state of a digital output
   *
   * \param address Address as describe in MODBUS specification
   *                 (first output of a node has address 0, second has address 1, ...)
   *                 If address is out of range, a invalid value is returned (see setupDigitalOutputs() ).
   * \param readDirectly If true, the request is sent to device directly, else it can be sent with writeDigitalOutputs()
   * \return True on success.
   */
  bool setDigitalOutput(int address, bool state, bool writeDirectly);

  /*! \brief Get the state of a digital output
   *
   * When delaying the request, the currently stored state is returned.
   *
   * \param address Address as describe in MODBUS specification
   *                 (first output of a node has address 0, second has address 1, ...)
   *                 If address is out of range, a invalid value is returned (see setupDigitalOutputs() ).
   * \param readDirectly If true, the request is sent to device directly, else it can be sent with readDigitalInputs()
   * \return The state of requested output, or an invalid value on error.
   */
  virtual QVariant getDigitalOutputState(int address, bool readDirectly);

  /*! \brief Write digital output states to device
   *
   * \return True on success.
   */
  bool writeDigitalOutputs();

  /*! \brief Request states from device
   *
   * Request states of all digital outputs.
   *
   * \todo Offset sur adresse départ selon marque
   *
   * \return True on success.
   */
  virtual bool readDigitalOutputs();

 public slots:

  /*! \brief Decode incoming frames
   *
   * \pre I/O's container must be set with setIos()
   *
   * Subclass notes:<br>
   *  - This default implementation does nothing.
   *  - This slot should be connected with mdtPortManager::newReadenFrame() signal.
   *  - In this class, this connection is not made, it is the sublcass responsability to do this.
   *  - The incoming frames are available with mdtPortManager::readenFrames().
   */
  void decodeReadenFrames(int id, QByteArray pdu);

 private:

  // Sequence of periodic queries
  bool queriesSequence();

  mdtModbusTcpPortManager *pvTcpPortManager;
  mdtFrameCodecModbus *pvCodec;
};

#endif  // #ifndef MDT_DEVICE_MODBUS_H
