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
#include "mdtUsbtmcPortThread.h"
#include "mdtUsbPort.h"
#include "mdtError.h"
#include "mdtFrameUsbTmc.h"
#include <QApplication>

#include <QDebug>

mdtUsbtmcPortThread::mdtUsbtmcPortThread(QObject *parent)
/// : mdtPortThread(parent)
 : mdtUsbPortThread(parent)
{
}

bool mdtUsbtmcPortThread::isReader() const
{
  return true;
}

bool mdtUsbtmcPortThread::isWriter() const
{
  return true;
}

mdtAbstractPort::error_t mdtUsbtmcPortThread::usbtmcWrite(mdtFrame **writeFrame, bool *waitAnAnswer, QList<quint8> &expectedBulkInbTags)
{
  Q_ASSERT(writeFrame != 0);
  Q_ASSERT(waitAnAnswer != 0);
  Q_ASSERT(pvPort != 0);

  qint64 written;
  mdtAbstractPort::error_t portError;
  mdtUsbPort *port = dynamic_cast<mdtUsbPort*>(pvPort);
  Q_ASSERT(port != 0);

  // Check if we have something to write
  if(*writeFrame == 0){
    if(port->writeFrames().size() < 1){
      return mdtAbstractPort::NoError;
    }
    *writeFrame = port->writeFrames().dequeue();
  }
  Q_ASSERT(*writeFrame != 0);
  emit(writeProcessBegin());
  ///qDebug() << "USBPTHD: to write: " << writeFrame->size();
  // Write (will simply do nothing and return 0 if transfer is pending)
  written = writeDataToPort(*writeFrame);
  if(written < 0){
    // Check about stoping
    if(!pvRunning){
      return mdtAbstractPort::UnhandledError;
    }
    /// \todo Restore frame on error ??
    portError = handleCommonErrors((mdtAbstractPort::error_t)written);
    if(portError != mdtAbstractPort::NoError){
      // Restore frame to pool
      port->writeFramesPool().enqueue(*writeFrame);
      *writeFrame = 0;
      // Errors that must be signaled + stop the thread
      notifyError(portError);
      return mdtAbstractPort::UnhandledError;
    }
  }
  // Check if frame was completly written
  if((*writeFrame)->isEmpty()){
    qDebug() << "USBPTHD: frame written";
    // Update waitAnAnswer flag and add bTag to the incomming expeced ones
    *waitAnAnswer = (*writeFrame)->waitAnAnswer();
    if(*waitAnAnswer){
      expectedBulkInbTags.append((static_cast<mdtFrameUsbTmc*>(*writeFrame))->bTag());
    }
    // Restore frame to pool
    port->writeFramesPool().enqueue(*writeFrame);
    // Check if a new frame is to write
    if(port->writeFrames().size() > 0){
      *writeFrame = port->writeFrames().dequeue();
      Q_ASSERT(*writeFrame != 0);
    }else{
      *writeFrame = 0;
    }
  }
  // Here, if frame is not Null, we have to init a new transfer
  if(*writeFrame != 0){
    // Init a new write transfer (will only init if not pending)
    portError = port->initWriteTransfer((*writeFrame)->data(), (*writeFrame)->size());
    if(portError != mdtAbstractPort::NoError){
      // Check about stoping
      if(!pvRunning){
        return mdtAbstractPort::UnhandledError;
      }
      portError = handleCommonErrors((mdtAbstractPort::error_t)written);
      if(portError != mdtAbstractPort::NoError){
        // Restore frame to pool
        port->writeFramesPool().enqueue(*writeFrame);
        *writeFrame = 0;
        // Errors that must be signaled + stop the thread
        notifyError(portError);
        return mdtAbstractPort::UnhandledError;
      }
    }
  }

  return mdtAbstractPort::NoError;
}

/**
mdtAbstractPort::error_t mdtUsbtmcPortThread::readUntilShortPacketReceived(int maxReadTransfers)
{
  Q_ASSERT(pvPort != 0);

  mdtAbstractPort::error_t portError;
  mdtUsbPort *port = dynamic_cast<mdtUsbPort*>(pvPort);
  Q_ASSERT(port != 0);
  char *buffer = new char[port->readBufferSize()];
  int readen;

  while(maxReadTransfers > 0){
    readen = port->read(buffer, port->readBufferSize());
    if(readen < 0){
      delete[] buffer;
      return (mdtAbstractPort::error_t)readen;
    }
    if(readen < port->readBufferSize()){
      delete[] buffer;
      return mdtAbstractPort::NoError;
    }
    portError = port->initReadTransfer(port->readBufferSize());
    if(portError != mdtAbstractPort::NoError){
      delete[] buffer;
      return portError;
    }
    portError = port->handleUsbEvents();
    if(portError != mdtAbstractPort::NoError){
      delete[] buffer;
      return portError;
    }
    maxReadTransfers--;
  }
  delete[] buffer;

  return mdtAbstractPort::ReadTimeout;
}
*/

mdtAbstractPort::error_t mdtUsbtmcPortThread::abortBulkIn(quint8 bTag)
{
  Q_ASSERT(pvPort != 0);

  mdtUsbPort *port;
  mdtAbstractPort::error_t portError;
  QQueue<mdtFrameUsbControl*> ctlFramesPoolCopy;
  mdtFrameUsbControl *ctlFrame;
  quint8 status;
  int maxTry;

  // We need a mdtUsbPort object
  port = dynamic_cast<mdtUsbPort*>(pvPort);
  Q_ASSERT(port != 0);

  // Flush pending control frames
  while(port->controlQueryFrames().size() > 0){
    // Warn and remove frame
    mdtError e(MDT_USB_IO_ERROR, "Aborting a pending control transfer", mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
    e.commit();
    port->controlFramesPool().enqueue(port->controlQueryFrames().dequeue());
  }
  // Cancel possibly running control transfer
  portError = port->cancelControlTransfer();
  if(portError != mdtAbstractPort::NoError){
    return portError;
  }
  while(port->controlResponseFrames().size() > 0){
    // Warn and remove frame
    mdtError e(MDT_USB_IO_ERROR, "Aborting a pending control transfer", mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
    e.commit();
    port->controlFramesPool().enqueue(port->controlResponseFrames().dequeue());
  }
  // Get a control frame for working
  Q_ASSERT(port->controlFramesPool().size() > 0);
  ctlFrame = port->controlFramesPool().dequeue();
  Q_ASSERT(ctlFrame != 0);
  // Take all control frames from pool, so port manager cannot init a control transfer
  while(port->controlFramesPool().size() > 0){
    ctlFramesPoolCopy.enqueue(port->controlFramesPool().dequeue());
  }
  // Send the INITIATE_ABORT_BULK_IN request and wait on response
  status = 0x81;  // STATUS_TRANSFER_NOT_IN_PROGRESS
  maxTry = 10;
  while((status == 0x81)&&(maxTry > 0)){
    // Check about stoping
    if(!pvRunning){
      // restore frames and return noerror
      port->controlFramesPool().enqueue(ctlFrame);
      while(ctlFramesPoolCopy.size() > 0){
        port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
      }
      return mdtAbstractPort::NoError;
    }
    // Send INITIATE_ABORT_BULK_IN request
    qDebug() << "*-* send INITIATE_ABORT_BULK_IN ...";
    portError = sendInitiateAbortBulkInRequest(bTag, ctlFrame);
    if(portError != mdtAbstractPort::NoError){
      // restore frames and abort
      port->controlFramesPool().enqueue(ctlFrame);
      while(ctlFramesPoolCopy.size() > 0){
        port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
      }
      return portError;
    }
    // more than 1 frame available should only happen if device had a control response queued
    if(port->controlResponseFrames().size() != 1){
      mdtError e(MDT_USB_IO_ERROR, "INITIATE_ABORT_BULK_IN returned unexpected amount of responses", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
      e.commit();
      // restore frames and abort
      port->controlFramesPool().enqueue(ctlFrame);
      while(ctlFramesPoolCopy.size() > 0){
        port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
      }
      return mdtAbstractPort::UnhandledError;
    }
    ctlFrame = port->controlResponseFrames().dequeue();
    if(ctlFrame->size() != 2){
      mdtError e(MDT_USB_IO_ERROR, "INITIATE_ABORT_BULK_IN returned unexpected response size (expected 2, received " + QString::number(ctlFrame->size()) + ")", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
      e.commit();
      // Restore frames and abort
      port->controlFramesPool().enqueue(ctlFrame);
      while(ctlFramesPoolCopy.size() > 0){
        port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
      }
      return mdtAbstractPort::UnhandledError;
    }
    // Check status
    status = ctlFrame->at(0);
    if(status == 0x81){ // STATUS_TRANSFER_NOT_IN_PROGRESS
      msleep(1000);
    }
    maxTry--;
  }
  if(maxTry < 1){
    mdtError e(MDT_USB_IO_ERROR, "INITIATE_ABORT_BULK_IN still pending after 10 try", mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
    e.commit();
    // Continue working (possibly, nothing was to abort)
    port->controlFramesPool().enqueue(ctlFrame);
    while(ctlFramesPoolCopy.size() > 0){
      port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
    }
    if(pvRunning){
      notifyError(mdtAbstractPort::ReadCanceled, true);
    }
    return mdtAbstractPort::NoError;
  }
  // Check returned status
  if(status != 0x01){ // 0x01: STATUS_SUCCESS
    // No transfer in progress (device FIFO empty)
    port->controlFramesPool().enqueue(ctlFrame);
    while(ctlFramesPoolCopy.size() > 0){
      port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
    }
    if(pvRunning){
      notifyError(mdtAbstractPort::ReadCanceled, true);
    }
    return mdtAbstractPort::NoError;
  }
  // Flush device
  portError = readUntilShortPacketReceived(100);
  if(portError != mdtAbstractPort::NoError){
    // restore frames and abort
    port->controlFramesPool().enqueue(ctlFrame);
    while(ctlFramesPoolCopy.size() > 0){
      port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
    }
    return portError;
  }
  // Send the CHECK_ABORT_BULK_IN_STATUS request and wait on response
  status = 0x02;  // STATUS_PENDING
  maxTry = 10;
  while((status == 0x02)&&(maxTry > 0)){
    // Send CHECK_ABORT_BULK_IN_STATUS
    qDebug() << "*-* send CHECK_ABORT_BULK_IN_STATUS ...";
    portError = sendCheckAbortBulkInStatusRequest(bTag, ctlFrame);
    if(portError != mdtAbstractPort::NoError){
      // restore frames and abort
      port->controlFramesPool().enqueue(ctlFrame);
      while(ctlFramesPoolCopy.size() > 0){
        port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
      }
      return portError;
    }
    // more than 1 frame available should only happen if device had a control response queued
    if(port->controlResponseFrames().size() != 1){
      mdtError e(MDT_USB_IO_ERROR, "CHECK_ABORT_BULK_IN_STATUS returned unexpected amount of responses", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
      e.commit();
      // restore frames and abort
      port->controlFramesPool().enqueue(ctlFrame);
      while(ctlFramesPoolCopy.size() > 0){
        port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
      }
      return mdtAbstractPort::UnhandledError;
    }
    ctlFrame = port->controlResponseFrames().dequeue();
    if(ctlFrame->size() != 8){
      mdtError e(MDT_USB_IO_ERROR, "CHECK_ABORT_BULK_IN_STATUS returned unexpected response size (expected 8, received " + QString::number(ctlFrame->size()) + ")", mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
      e.commit();
      // restore frames and abort
      port->controlFramesPool().enqueue(ctlFrame);
      while(ctlFramesPoolCopy.size() > 0){
        port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
      }
      return mdtAbstractPort::UnhandledError;
    }
    // Check bmAbortBulkIn.D0
    if((quint8)ctlFrame->at(1) & 0x01){
      qDebug() << "*-* Flushing device ...";
      // Flush device
      portError = readUntilShortPacketReceived(100);
      if(portError != mdtAbstractPort::NoError){
        // restore frames and abort
        port->controlFramesPool().enqueue(ctlFrame);
        while(ctlFramesPoolCopy.size() > 0){
          port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
        }
        return portError;
      }
    }
    // Check status
    status = ctlFrame->at(0);
    if(status == 0x02){ // STATUS_PENDING
      wait(1000);
    }
    maxTry--;
  }
  if(maxTry < 1){
    mdtError e(MDT_USB_IO_ERROR, "CHECK_ABORT_BULK_IN_STATUS still pending after 10 try", mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
    e.commit();
  }
  // End, restore frames and notify port manager
  port->controlFramesPool().enqueue(ctlFrame);
  while(ctlFramesPoolCopy.size() > 0){
    port->controlFramesPool().enqueue(ctlFramesPoolCopy.dequeue());
  }
  if(pvRunning){
    notifyError(mdtAbstractPort::ReadCanceled, true);
  }

  qDebug() << "*-* abortBulkIn() DONE";
  return mdtAbstractPort::NoError;
}

mdtAbstractPort::error_t mdtUsbtmcPortThread::sendInitiateAbortBulkInRequest(quint8 bTag, mdtFrameUsbControl *ctlFrame)
{
  Q_ASSERT(pvPort != 0);
  Q_ASSERT(ctlFrame != 0);

  mdtAbstractPort::error_t portError;
  mdtUsbPort *port;

  // We need a mdtUsbPort object
  port = dynamic_cast<mdtUsbPort*>(pvPort);
  Q_ASSERT(port != 0);

  // Setup frame
  ctlFrame->setDirectionDeviceToHost(true);
  ctlFrame->setRequestType(mdtFrameUsbControl::RT_CLASS);
  ctlFrame->setRequestRecipient(mdtFrameUsbControl::RR_ENDPOINT);
  ctlFrame->setbRequest(3);   // INITIATE_ABORT_BULK_IN
  ctlFrame->setwValue(bTag);  // D7..D0: bTag
  ctlFrame->setwIndex(port->currentReadEndpointAddress());
  ctlFrame->setwLength(0x02);
  ctlFrame->encode();
  // Submit the control request and wait until transfer is done
  port->addControlRequest(ctlFrame);
  while(port->controlResponseFrames().size() < 1){
    // Check about stoping
    if(!pvRunning){
      return mdtAbstractPort::NoError;
    }
    portError = port->handleUsbEvents();
    if(portError != mdtAbstractPort::NoError){
      return portError;
    }
  }

  return mdtAbstractPort::NoError;
}

mdtAbstractPort::error_t mdtUsbtmcPortThread::sendCheckAbortBulkInStatusRequest(quint8 bTag, mdtFrameUsbControl *ctlFrame)
{
  Q_ASSERT(pvPort != 0);
  Q_ASSERT(ctlFrame != 0);

  mdtAbstractPort::error_t portError;
  mdtUsbPort *port;

  // We need a mdtUsbPort object
  port = dynamic_cast<mdtUsbPort*>(pvPort);
  Q_ASSERT(port != 0);

  // Setup frame
  ctlFrame->setDirectionDeviceToHost(true);
  ctlFrame->setRequestType(mdtFrameUsbControl::RT_CLASS);
  ctlFrame->setRequestRecipient(mdtFrameUsbControl::RR_ENDPOINT);
  ctlFrame->setbRequest(4);   // CHECK_ABORT_BULK_IN_STATUS
  ctlFrame->setwValue(0);
  ctlFrame->setwIndex(port->currentReadEndpointAddress());
  ctlFrame->setwLength(0x08);
  ctlFrame->encode();
  // Submit the control request and wait until transfer is done
  port->addControlRequest(ctlFrame);
  while(port->controlResponseFrames().size() < 1){
    // Check about stoping
    if(!pvRunning){
      return mdtAbstractPort::NoError;
    }
    portError = port->handleUsbEvents();
    if(portError != mdtAbstractPort::NoError){
      return portError;
    }
  }

  return mdtAbstractPort::NoError;
}

/**
mdtAbstractPort::error_t mdtUsbtmcPortThread::handleCommonErrors(mdtAbstractPort::error_t portError)
{
  if(portError == mdtAbstractPort::Disconnected){
    // Try to reconnect
    portError = reconnect(true);
    if(portError != mdtAbstractPort::NoError){
      return mdtAbstractPort::UnhandledError;
    }
    return mdtAbstractPort::NoError;
  }

  return portError;
}
*/

void mdtUsbtmcPortThread::run()
{
  Q_ASSERT(pvPort != 0);

  mdtFrame *writeFrame = 0;
  mdtFrame *readFrame = 0;
  mdtFrameUsbTmc *usbtmcFrame;
  mdtAbstractPort::error_t portError;
  int n;
  int i;
  mdtUsbPort *port;
  bool waitAnAnswer = false;
  ///qint64 written;
  QList<mdtAbstractPort::error_t> errors;
  ///int bTagToAbort;
  QList<quint8> expectedBulkInbTags;
  bool ok;

  pvPort->lockMutex();
#ifdef Q_OS_UNIX
  pvNativePthreadObject = pthread_self();
  Q_ASSERT(pvNativePthreadObject != 0);
#endif
  // We need a mdtUsbPort object here
  port = dynamic_cast<mdtUsbPort*> (pvPort);
  Q_ASSERT(port != 0);
  // Set the running flag
  pvRunning = true;
  // Get a frame for read
  readFrame = getNewFrameRead();
  if(readFrame == 0){
    return;
  }
  // Notify that we are ready
  notifyError(mdtAbstractPort::NoError);

  // Run...
  while(1){
    // Read thread state
    if(!pvRunning){
      break;
    }
    qDebug() << "USBPTHD: going idle ...";
    portError = port->handleUsbEvents();
    qDebug() << "USBPTHD: event !";
    // Check about event handling errors
    if(portError != mdtAbstractPort::NoError){
      // Check about stoping
      if(!pvRunning){
        break;
      }
      portError = handleCommonErrors(portError);
      if(portError != mdtAbstractPort::NoError){
        // Errors that must be signaled + stop the thread
        notifyError(portError);
        break;
      }
    }
    // Check about port errors
    errors = port->lastErrors();
    for(i=0; i<errors.size(); i++){
      // Check about stoping
      if(!pvRunning){
        break;
      }
      portError = handleCommonErrors(errors.at(i));
      if(portError != mdtAbstractPort::NoError){
        if(portError == mdtAbstractPort::WriteCanceled){
          // Restore current frame into pool and notify error
          if(writeFrame != 0){
            port->writeFramesPool().enqueue(writeFrame);
            writeFrame = 0;
            notifyError(mdtAbstractPort::WriteCanceled);
          }
        }else if((portError == mdtAbstractPort::ReadCanceled)||(portError == mdtAbstractPort::ReadTimeout)){
          // We submit the uncomplete frame
          pvPort->readenFrames().enqueue(readFrame);
          emit newFrameReaden();
          ///qDebug() << "mdtUsbtmcPortThread::run(): read cancel or timeout";
          readFrame = getNewFrameRead();
        }else if(portError == mdtAbstractPort::ControlCanceled){
          // mdtUsbPort does the job, we just have to notify the error
          notifyError(mdtAbstractPort::ControlCanceled);
        }else{
          // Unhandled error. Notify and stop
          notifyError(portError);
          pvRunning = false;
          break;
        }
      }
    }
    // Check about stoping
    if(!pvRunning){
      break;
    }
    // Check if a control response is available
    if(port->controlResponseFrames().size() > 0){
      emit(controlResponseReaden());
    }
    // Check if a message IN is available
    if(port->messageInFrames().size() > 0){
      emit(messageInReaden());
    }
    // Init a new message IN transfert (will only init if needed)
    portError = port->initMessageInTransfer();
    if(portError != mdtAbstractPort::NoError){
      // Check about stoping
      if(!pvRunning){
        break;
      }
      portError = handleCommonErrors(portError);
      if(portError != mdtAbstractPort::NoError){
        // Errors that must be signaled + stop the thread
        notifyError(portError);
        break;
      }
    }
    // Write ...
    portError = usbtmcWrite(&writeFrame, &waitAnAnswer, expectedBulkInbTags);
    if(portError != mdtAbstractPort::NoError){
      break;
    }
    /**
    // Check about device flush
    if(port->readUntilShortPacketReceivedRequestPending()){
      portError = readUntilShortPacketReceived(100);
      if(portError != mdtAbstractPort::NoError){
        // Check about stoping
        if(!pvRunning){
          break;
        }
        portError = handleCommonErrors(portError);
        if(portError != mdtAbstractPort::NoError){
          // Errors that must be signaled + stop the thread
          notifyError(portError);
          break;
        }
      }
      emit(readUntilShortPacketReceivedFinished());
    }
    */
    Q_ASSERT(readFrame != 0);
    // Check about stoping
    if(!pvRunning){
      break;
    }
    // read/store available data
    n = -1;
    if(!port->readTimeoutOccured()){
      // We not emit the newFrameReaden() signal, see below
      n = readFromPort(&readFrame, false);
      qDebug() << "USBPTHD: frames readen: " << n;
      if(n < 0){
        // Unhandled error: notify and stop
        notifyError(n);
        break;
      }
      // Reset waitAnAnswer flag if a frame was received
      if(n > 0){
        waitAnAnswer = false;
      }
    }else{
      // Abort, clear frame and notify
      portError = abortBulkIn((static_cast<mdtFrameUsbTmc*>(readFrame))->bTag());
      readFrame->clear();
      readFrame->clearSub();
      if(portError != mdtAbstractPort::NoError){
        // Check about stoping
        if(!pvRunning){
          break;
        }
        portError = handleCommonErrors(portError);
        if(portError != mdtAbstractPort::NoError){
          break;
        }
      }
      notifyError(mdtAbstractPort::ReadTimeout);
      waitAnAnswer = false;
    }
    ///qDebug() << "USBPTHD: waitAnAnswer: " << waitAnAnswer << " , n: " << n;
    // Two conditions to init a read transfer:
    // - A query/reply is pending (waitAnAnswer)
    // - Current read frame is not complete
    if((waitAnAnswer)&&(n == 0)){
      // Init a new read transfer (will only init if not pending)
      qDebug() << "USBPTHD: to read: " << readFrame->bytesToStore();
      portError = port->initReadTransfer(readFrame->bytesToStore());
      if(portError != mdtAbstractPort::NoError){
        // Check about stoping
        if(!pvRunning){
          break;
        }
        portError = handleCommonErrors(portError);
        if(portError != mdtAbstractPort::NoError){
          if(portError == mdtAbstractPort::ReadCanceled){
            // Abort, clear frame and notify
            portError = abortBulkIn((static_cast<mdtFrameUsbTmc*>(readFrame))->bTag());
            readFrame->clear();
            readFrame->clearSub();
            if(portError != mdtAbstractPort::NoError){
              // Check about stoping
              if(!pvRunning){
                break;
              }
              portError = handleCommonErrors(portError);
              if(portError != mdtAbstractPort::NoError){
                break;
              }
            }
            waitAnAnswer = false;
          }else{
            // Errors that must be signaled + stop the thread
            notifyError(portError);
            break;
          }
        }
      }
    }
    // At this state, if somes frame(s) are complete, we must check them and emit the signal if ok
    /// \bug Must deque frames and re-enqueue if ok (and restore if Nok
    
    QMutableListIterator<mdtFrame*> it(port->readenFrames());
    while(it.hasNext()){
      it.next();
      qDebug() << "readFrames() size A : " << port->readenFrames().size();
      // We need a USBTMC frame
      usbtmcFrame = static_cast<mdtFrameUsbTmc*>(it.value());
      qDebug() << "USBTMCTHD: current readFrame's bTag: " << usbtmcFrame->bTag() << " , expectedBulkInbTags: " << expectedBulkInbTags;
      ok = true;
      // Check if complete
      if(!usbtmcFrame->isComplete()){
        ok = false;
        mdtError e(MDT_USB_IO_ERROR, "Receive a uncomplete frame, will be aborted", mdtError::Warning);
        MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
        e.commit();
      }
      // Check bTag/bTagInverse coherence
      if(!usbtmcFrame->bTagOk()){
        ok = false;
        mdtError e(MDT_USB_IO_ERROR, "Receive a frame with incoherent bTag/bTagInverse, will be aborted", mdtError::Error);
        MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
        e.commit();
      }
      // Check if MsgID is supported
      if(!usbtmcFrame->MsgIDsupported()){
        ok = false;
        mdtError e(MDT_USB_IO_ERROR, "Receive a frame with unsupported MsgID, will be aborted", mdtError::Error);
        MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
        e.commit();
      }
      // Abort if error occured
      if(!ok){
        // Remove frame and put back to pool
        it.remove();
        port->readFramesPool().enqueue(usbtmcFrame);
        qDebug() << "USBTMCTHD: aborting ...";
        expectedBulkInbTags.clear();
        portError = abortBulkIn(usbtmcFrame->bTag());
        qDebug() << "USBTMCTHD: aborting DONE";
        if(portError != mdtAbstractPort::NoError){
          qDebug() << "USBTMCTHD: handling common errors ...";
          handleCommonErrors(portError);
          qDebug() << "USBTMCTHD: handling common errors DONE";
          break;
        }
      }else{
        // Check if bTag was expected
        if(!expectedBulkInbTags.contains(usbtmcFrame->bTag())){
          mdtError e(MDT_USB_IO_ERROR, "Receive a frame with unexpeced bTag, will be skipped", mdtError::Error);
          MDT_ERROR_SET_SRC(e, "mdtUsbtmcPortThread");
          e.commit();
          // Remove frame and put back to pool
          it.remove();
          port->readFramesPool().enqueue(usbtmcFrame);
          expectedBulkInbTags.removeOne(usbtmcFrame->bTag());
        }else{
          expectedBulkInbTags.removeOne(usbtmcFrame->bTag());
          // Simply emit signal, port manager will take the frame when port mutext is unlocked
          qDebug() << "USBTMCTHD: emit newFrameReaden() ...";
          emit(newFrameReaden());
        }
      }
      qDebug() << "readFrames() size B : " << port->readenFrames().size();
    }
  }

  ///qDebug() << "USBTHD: cleanup ...";
  port->cancelTransfers();

  // Put current frame into pool
  if(readFrame != 0){
    port->readFramesPool().enqueue(readFrame);
  }

  ///qDebug() << "USBTHD: END";

  pvRunning = false;
  pvPort->unlockMutex();
  notifyError(mdtAbstractPort::Disconnected);
}

