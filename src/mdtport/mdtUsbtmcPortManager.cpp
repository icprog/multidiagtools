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
#include "mdtUsbtmcPortManager.h"
#include "mdtError.h"
#include "mdtPortConfig.h"
#include "mdtFrame.h"
#include "mdtFrameUsbControl.h"
#include "mdtFrameUsbTmc.h"
#include "mdtUsbDeviceDescriptor.h"
#include "mdtUsbConfigDescriptor.h"
#include "mdtUsbInterfaceDescriptor.h"
#include "mdtFrameCodecScpi.h"
#include <QString>
#include <QApplication>
#include <libusb-1.0/libusb.h>

#include <QDebug>

mdtUsbtmcPortManager::mdtUsbtmcPortManager(QObject *parent)
 : mdtUsbPortManager(parent)
{
  Q_ASSERT(pvPort != 0);

  // Port setup
  pvPort->config().setFrameType(mdtFrame::FT_USBTMC);

  // USBTMC specific
  pvCurrentWritebTag = 0;

  // Enable transactions support
  setTransactionsEnabled();
}

mdtUsbtmcPortManager::~mdtUsbtmcPortManager()
{
}

QList<mdtPortInfo*> mdtUsbtmcPortManager::scan()
{
  QList<mdtPortInfo*> portInfoList;
  mdtDeviceInfo *deviceInfo;
  int i, j;

  // Search all USBTMC devices (bInterfaceClass == 0xFE and bInterfaceSubClass == 0x03)
  portInfoList = mdtUsbPortManager::scan(0xFE, 0x03, -1);

  // Update some informations in found devices interfaces
  for(i=0; i<portInfoList.size(); i++){
    for(j=0; j<portInfoList.at(i)->deviceInfoList().size(); j++){
      deviceInfo = portInfoList.at(i)->deviceInfoList().at(j);
      Q_ASSERT(deviceInfo != 0);
      if(deviceInfo->protocolId() == 0x00){
        deviceInfo->setDisplayText("USBTMC");
      }else if(deviceInfo->protocolId() == 0x01){
        deviceInfo->setDisplayText("USBTMC USB488");
      }else{
        deviceInfo->setDisplayText("Unknow");
      }
    }
  }

  return portInfoList;
}

int mdtUsbtmcPortManager::sendCommand(const QByteArray &command, int timeout)
{
  // Wait until data can be sent
  if(!waitOnWriteReady(timeout)){
    return mdtAbstractPort::WritePoolEmpty;
  }
  // Send query
  return writeData(command);
}

QByteArray mdtUsbtmcPortManager::sendQuery(const QByteArray &query, int writeTimeout, int readTimeout)
{
  int bTag;
  mdtPortTransaction *transaction;

  // Wait until data can be sent
  if(!waitOnWriteReady(writeTimeout)){
    return QByteArray();
  }
  // Send query
  bTag = writeData(query);
  if(bTag < 0){
    return QByteArray();
  }
  // Wait until more data can be sent
  if(!waitOnWriteReady(writeTimeout)){
    return QByteArray();
  }
  // Setup transaction
  transaction = getNewTransaction();
  transaction->setType(MDT_FC_SCPI_UNKNOW);
  transaction->setQueryReplyMode(true);
  // Send read request
  bTag = sendReadRequest(transaction);
  if(bTag < 0){
    return QByteArray();
  }
  ///qDebug() << "mdtUsbtmcPortManager::sendQuery() , bTag: " << bTag << " , query: " << query;
  // Wait on response
  if(!waitOnFrame(bTag, readTimeout)){
    return QByteArray();
  }

  return readenFrame(bTag);
}

int mdtUsbtmcPortManager::writeData(QByteArray data)
{
  Q_ASSERT(pvPort != 0);

  mdtFrameUsbTmc *frame;

  // Get a frame in pool
  pvPort->lockMutex();
  if(pvPort->writeFramesPool().size() < 1){
    pvPort->unlockMutex();
    mdtError e(MDT_PORT_IO_ERROR, "No frame available in write frames pool", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
    e.commit();
    return mdtAbstractPort::WritePoolEmpty;
  }
  frame = dynamic_cast<mdtFrameUsbTmc*> (pvPort->writeFramesPool().dequeue());
  Q_ASSERT(frame != 0);
  frame->clear();
  frame->clearSub();
  // Store data and add frame to write queue
  frame->setWaitAnAnswer(false);
  frame->setMsgID(mdtFrameUsbTmc::DEV_DEP_MSG_OUT);
  // Increment bTag and enshure it's <> 0
  pvCurrentWritebTag++;
  if(pvCurrentWritebTag == 0){
    pvCurrentWritebTag++;
  }
  frame->setbTag(pvCurrentWritebTag);
  frame->setMessageData(data);
  frame->setEOM(true);
  frame->encode();
  pvPort->addFrameToWrite(frame);
  pvPort->unlockMutex();

  return pvCurrentWritebTag;
}

int mdtUsbtmcPortManager::sendReadRequest(mdtPortTransaction *transaction)
{
  Q_ASSERT(pvPort != 0);
  Q_ASSERT(transaction != 0);
  Q_ASSERT(config().readFrameSize() > 13);

  mdtFrameUsbTmc *frame;

  // Get a frame in pool
  pvPort->lockMutex();
  if(pvPort->writeFramesPool().size() < 1){
    pvPort->unlockMutex();
    mdtError e(MDT_PORT_IO_ERROR, "No frame available in write frames pool", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
    e.commit();
    return mdtAbstractPort::WritePoolEmpty;
  }
  frame = dynamic_cast<mdtFrameUsbTmc*> (pvPort->writeFramesPool().dequeue());
  Q_ASSERT(frame != 0);
  frame->clear();
  frame->clearSub();
  // Store data, encode and add frame to write queue
  frame->setWaitAnAnswer(true);
  frame->setMsgID(mdtFrameUsbTmc::DEV_DEP_MSG_IN);
  // Increment bTag and enshure it's <> 0
  pvCurrentWritebTag++;
  if(pvCurrentWritebTag == 0){
    pvCurrentWritebTag++;
  }
  frame->setbTag(pvCurrentWritebTag);
  frame->setTransferSize(config().readFrameSize()-13);
  frame->encode();
  pvPort->addFrameToWrite(frame);
  // Add transaction
  transaction->setId(pvCurrentWritebTag);
  addTransaction(transaction);
  pvPort->unlockMutex();

  return pvCurrentWritebTag;
}

int mdtUsbtmcPortManager::sendReadRequest(bool enqueueResponse)
{
  mdtPortTransaction *transaction;

  // Get a transaction from pool and setup it
  transaction = getNewTransaction();
  transaction->setQueryReplyMode(enqueueResponse);

  return sendReadRequest(transaction);
}

/// \todo finish !
int mdtUsbtmcPortManager::sendReadStatusByteRequest()
{
  Q_ASSERT(pvPort != 0);

  mdtFrameUsbControl frame;
  int retVal;

  frame.setDirectionDeviceToHost(true);
  frame.setRequestType(mdtFrameUsbControl::RT_CLASS);
  frame.setRequestRecipient(mdtFrameUsbControl::RR_INTERFACE);
  frame.setbRequest(128); // READ_STATUS_BYTE
  // Ajust bTag to:  2 <= bTag <= 127
  if(pvCurrentWritebTag > 127){
    pvCurrentWritebTag = 2;
  }
  while(pvCurrentWritebTag < 2){
    pvCurrentWritebTag++;
  }
  frame.setwValue(pvCurrentWritebTag);
  frame.setwLength(0x3);
  frame.encode();
  // In this query, wIndex is the interface number, let USB port fill it.
  retVal = sendControlRequest(frame, true);
  if(retVal < 0){
    return retVal;
  }
  return pvCurrentWritebTag;
  ///m.sendControlRequest(f);
  /// End Sandbox !

}

int mdtUsbtmcPortManager::abortBulkIn(quint8 bTag)
{
  Q_ASSERT(pvPort != 0);

  int retVal;
  QList<mdtFrameUsbControl> frames;
  mdtFrameUsbControl frame;
  quint8 status;
  int maxTry;

  // Flush all endpoints + internal buffers
  flush();
  // Send the INITIATE_ABORT_BULK_IN request and wait on response
  status = 0x81;  // STATUS_TRANSFER_NOT_IN_PROGRESS
  maxTry = 10;
  while((status == 0x81)&&(maxTry > 0)){
    ///qDebug() << "*-* send INITIATE_ABORT_BULK_IN ...";
    retVal = sendInitiateAbortBulkInRequest(bTag);
    if(retVal < 0){
      // Error logged by USB port manager
      return retVal;
    }
    if(!waitReadenControlResponse()){
      mdtError e(MDT_USB_IO_ERROR, "Timeout by INITIATE_ABORT_BULK_IN request", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::ControlTimeout;
    }
    frames = readenControlResponses();
    if(frames.size() != 1){
      mdtError e(MDT_USB_IO_ERROR, "INITIATE_ABORT_BULK_IN returned unexpected amount of responses", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::UnhandledError;
    }
    frame = frames.at(0);
    if(frame.size() != 2){
      mdtError e(MDT_USB_IO_ERROR, "INITIATE_ABORT_BULK_IN returned unexpected response size (expected 2, received " + QString::number(frame.size()) + ")", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::UnhandledError;
    }
    // Check status
    status = frame.at(0);
    if(status == 0x81){ // STATUS_TRANSFER_NOT_IN_PROGRESS
      wait(1000);
    }
    maxTry--;
  }
  if(maxTry < 1){
    mdtError e(MDT_USB_IO_ERROR, "INITIATE_ABORT_BULK_IN still pending after 10 try", mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
    e.commit();
    // Continue working (possibly, nothing was to abort)
    return 0;
  }
  // Check returned status
  if(status != 0x01){ // 0x01: STATUS_SUCCESS
    // No transfer in progress (device FIFO empty)
    return 0;
  }
  // Flush device
  readUntilShortPacketReceived();
  // Send the CHECK_ABORT_BULK_IN_STATUS request and wait on response
  status = 0x02;  // STATUS_PENDING
  while(status == 0x02){
    ///qDebug() << "*-* send CHECK_ABORT_BULK_IN_STATUS ...";
    retVal = sendCheckAbortBulkInStatusRequest(bTag);
    if(retVal < 0){
      // Error logged by USB port manager
      return retVal;
    }
    if(!waitReadenControlResponse()){
      mdtError e(MDT_USB_IO_ERROR, "Timeout by CHECK_ABORT_BULK_IN_STATUS request", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::ControlTimeout;
    }
    frames = readenControlResponses();
    if(frames.size() != 1){
      mdtError e(MDT_USB_IO_ERROR, "CHECK_ABORT_BULK_IN_STATUS returned unexpected amount of responses", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::UnhandledError;
    }
    frame = frames.at(0);
    if(frame.size() != 8){
      mdtError e(MDT_USB_IO_ERROR, "CHECK_ABORT_BULK_IN_STATUS returned unexpected response size (expected 8, received " + QString::number(frame.size()) + ")", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::UnhandledError;
    }
    // Check bmAbortBulkIn.D0
    if((quint8)frame.at(1) & 0x01){
      qDebug() << "*-* Flushing device ...";
      readUntilShortPacketReceived();
    }
    // Check status
    status = frame.at(0);
    if(status == 0x02){ // STATUS_PENDING
      wait(1000);
    }
  }
  // We are finished here, cancel portmanager wait methods
  cancelReadWait();

  return 0;
}

int mdtUsbtmcPortManager::sendInitiateAbortBulkInRequest(quint8 bTag)
{
  Q_ASSERT(pvPort != 0);

  mdtFrameUsbControl frame;
  mdtUsbPort *port;
  int retVal;

  // We need a mdtUsbPort object
  port = dynamic_cast<mdtUsbPort*>(pvPort);
  Q_ASSERT(port != 0);

  // Setup frame
  frame.setDirectionDeviceToHost(true);
  frame.setRequestType(mdtFrameUsbControl::RT_CLASS);
  frame.setRequestRecipient(mdtFrameUsbControl::RR_ENDPOINT);
  frame.setbRequest(3);   // INITIATE_ABORT_BULK_IN
  frame.setwValue(bTag);  // D7..D0: bTag
  port->lockMutex();
  frame.setwIndex(port->currentReadEndpointAddress());   // Endpoint IN 2
  port->unlockMutex();
  frame.setwLength(0x02);
  frame.encode();
  retVal = sendControlRequest(frame);
  if(retVal < 0){
    return retVal;
  }

  return bTag;
}

int mdtUsbtmcPortManager::sendCheckAbortBulkInStatusRequest(quint8 bTag)
{
  Q_ASSERT(pvPort != 0);

  mdtFrameUsbControl frame;
  mdtUsbPort *port;
  int retVal;

  // We need a mdtUsbPort object
  port = dynamic_cast<mdtUsbPort*>(pvPort);
  Q_ASSERT(port != 0);

  // Setup frame
  frame.setDirectionDeviceToHost(true);
  frame.setRequestType(mdtFrameUsbControl::RT_CLASS);
  frame.setRequestRecipient(mdtFrameUsbControl::RR_ENDPOINT);
  frame.setbRequest(4);   // CHECK_ABORT_BULK_IN_STATUS
  frame.setwValue(0);
  port->lockMutex();
  frame.setwIndex(port->currentReadEndpointAddress());   // Endpoint IN 2
  port->unlockMutex();
  frame.setwLength(0x08);
  frame.encode();
  retVal = sendControlRequest(frame);
  if(retVal < 0){
    return retVal;
  }

  return bTag;
}

/// \todo Finish and test
int mdtUsbtmcPortManager::abortBulkOut(quint8 bTag)
{
  Q_ASSERT(pvPort != 0);

  int retVal;
  QList<mdtFrameUsbControl> frames;
  mdtFrameUsbControl frame;
  quint8 status;

  // Flush all endpoints + internal buffers
  flush();
  // Send the INITIATE_ABORT_BULK_OUT request and wait on response
  status = 0x81;  // STATUS_TRANSFER_NO_IN_PROGRESS
  while(status == 0x81){
    qDebug() << "*-* send INITIATE_ABORT_BULK_OUT ...";
    retVal = sendInitiateAbortBulkOutRequest(bTag);
    if(retVal < 0){
      // Error logged by USB port manager
      return retVal;
    }
    if(!waitReadenControlResponse()){
      mdtError e(MDT_USB_IO_ERROR, "Timeout by INITIATE_ABORT_BULK_OUT request", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::ControlTimeout;
    }
    frames = readenControlResponses();
    if(frames.size() != 1){
      mdtError e(MDT_USB_IO_ERROR, "INITIATE_ABORT_BULK_OUT returned unexpected amount of responses", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::UnhandledError;
    }
    frame = frames.at(0);
    if(frame.size() != 2){
      mdtError e(MDT_USB_IO_ERROR, "INITIATE_ABORT_BULK_OUT returned unexpected response size (expected 2, received " + QString::number(frame.size()) + ")", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::UnhandledError;
    }
    // Check status
    status = frame.at(0);
    if(status == 0x81){ // STATUS_TRANSFER_NO_IN_PROGRESS
      wait(100);
    }
  }
  qDebug() << "offset 0: 0x" << hex << (quint8)frame.at(0);
  qDebug() << "offset 1: 0x" << hex << (quint8)frame.at(1);
  
  if(status != 0x01){ // 0x01: STATUS_SUCCESS
    // No transfer in progress
    qDebug() << "*-* Device FIFO empty";
    return 0;
  }
  // Send the CHECK_ABORT_BULK_OUT_STATUS request and wait on response
  status = 0x02;  // STATUS_PENDING
  while(status == 0x02){
    qDebug() << "*-* send CHECK_ABORT_BULK_OUT_STATUS ...";
    retVal = sendCheckAbortBulkOutStatusRequest(bTag);
    if(retVal < 0){
      // Error logged by USB port manager
      return retVal;
    }
    if(!waitReadenControlResponse()){
      mdtError e(MDT_USB_IO_ERROR, "Timeout by CHECK_ABORT_BULK_OUT_STATUS request", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::ControlTimeout;
    }
    frames = readenControlResponses();
    if(frames.size() != 1){
      mdtError e(MDT_USB_IO_ERROR, "CHECK_ABORT_BULK_OUT_STATUS returned unexpected amount of responses", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::UnhandledError;
    }
    frame = frames.at(0);
    if(frame.size() != 5){
      mdtError e(MDT_USB_IO_ERROR, "CHECK_ABORT_BULK_OUT_STATUS returned unexpected response size (expected 5, received " + QString::number(frame.size()) + ")", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      return mdtAbstractPort::UnhandledError;
    }
    // Check bmAbortBulkIn.D0
    if((quint8)frame.at(1) & 0x01){
      qDebug() << "*-* Flushing device ...";
      readUntilShortPacketReceived();
    }
    // Check status
    status = frame.at(0);
    if(status == 0x02){ // STATUS_PENDING
      wait(100);
    }
  }
  // Clear halt
  /// \todo Implement clear halt

}

int mdtUsbtmcPortManager::sendInitiateAbortBulkOutRequest(quint8 bTag)
{
  Q_ASSERT(pvPort != 0);

  mdtFrameUsbControl frame;
  mdtUsbPort *port;
  int retVal;

  // We need a mdtUsbPort object
  port = dynamic_cast<mdtUsbPort*>(pvPort);
  Q_ASSERT(port != 0);

  // Setup frame
  frame.setDirectionDeviceToHost(true);
  frame.setRequestType(mdtFrameUsbControl::RT_CLASS);
  frame.setRequestRecipient(mdtFrameUsbControl::RR_ENDPOINT);
  frame.setbRequest(1);   // INITIATE_ABORT_BULK_OUT
  frame.setwValue(bTag);  // D7..D0: bTag
  port->lockMutex();
  frame.setwIndex(port->currentWriteEndpointAddress());   // Endpoint OUT
  port->unlockMutex();
  frame.setwLength(0x02);
  frame.encode();
  retVal = sendControlRequest(frame);
  if(retVal < 0){
    return retVal;
  }

  return bTag;
}

int mdtUsbtmcPortManager::sendCheckAbortBulkOutStatusRequest(quint8 bTag)
{
  Q_ASSERT(pvPort != 0);

  mdtFrameUsbControl frame;
  mdtUsbPort *port;
  int retVal;

  // We need a mdtUsbPort object
  port = dynamic_cast<mdtUsbPort*>(pvPort);
  Q_ASSERT(port != 0);

  // Setup frame
  frame.setDirectionDeviceToHost(true);
  frame.setRequestType(mdtFrameUsbControl::RT_CLASS);
  frame.setRequestRecipient(mdtFrameUsbControl::RR_ENDPOINT);
  frame.setbRequest(2);   // CHECK_ABORT_BULK_OUT_STATUS
  frame.setwValue(0);
  port->lockMutex();
  frame.setwIndex(port->currentWriteEndpointAddress());   // Endpoint OUT
  port->unlockMutex();
  frame.setwLength(0x05);
  frame.encode();
  retVal = sendControlRequest(frame);
  if(retVal < 0){
    return retVal;
  }

  return bTag;
}

/// \todo On complete frame, check bTag/bTagInverse, and INITIATE_CLEAR on error.
void mdtUsbtmcPortManager::fromThreadNewFrameReaden()
{
  Q_ASSERT(pvPort != 0);

  mdtFrameUsbTmc *frame;
  mdtPortTransaction *transaction;

  // Get frames in readen queue
  pvPort->lockMutex();
  while(pvPort->readenFrames().size() > 0){
    frame = dynamic_cast<mdtFrameUsbTmc*> (pvPort->readenFrames().dequeue());
    Q_ASSERT(frame != 0);
    // Check if frame is complete
    if(frame->isComplete()){
      // If we have a pending transaction, remove it
      transaction = pendingTransaction(frame->bTag());
      if(transaction == 0){
        mdtError e(MDT_USB_IO_ERROR, "Received a frame with unexpected bTag", mdtError::Warning);
        MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
        e.commit();
        // Put frame back into pool
        pvPort->readFramesPool().enqueue(frame);
        // Try to abort the transaction
        pvPort->unlockMutex();
        abortBulkIn(frame->bTag());
        return;
      }else{
        // Check about MsgID
        if(frame->MsgID() != mdtFrameUsbTmc::DEV_DEP_MSG_IN){
          mdtError e(MDT_USB_IO_ERROR, \
                     "Received a frame with unhandled MsgID number " + QString::number(frame->MsgID()) + " , will be ignored", \
                     mdtError::Warning);
          MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
          e.commit();
        }else{
          // Here we should have a valid frame
          transaction->setId(frame->bTag());
          transaction->setData(frame->messageData());
          enqueueTransactionRx(transaction);
        }
      }
    }else{
      mdtError e(MDT_USB_IO_ERROR, "Received a uncomplete frame", mdtError::Warning);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortManager");
      e.commit();
      // Put frame back into pool
      pvPort->readFramesPool().enqueue(frame);
      // Try to abort the transaction
      pvPort->unlockMutex();
      abortBulkIn(frame->bTag());
      return;
    }
    // Put frame back into pool
    pvPort->readFramesPool().enqueue(frame);
  };
  pvPort->unlockMutex();
  // Commit
  commitFrames();
}
