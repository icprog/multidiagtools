
#include "mdtSerialPortPosix.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h> 
#include <QDebug>

mdtSerialPortPosix::mdtSerialPortPosix(QObject *parent)
 : mdtAbstractSerialPort(parent)
{
  pvSerialPortFd = -1;
  pvPreviousCarState = 0;
  pvPreviousDsrState = 0;
  pvPreviousCtsState = 0;
  pvPreviousRngState = 0;

  // Control signal thread kill utils
  pvCtlThread = 0;
  // We must catch the SIGALRM signal, else the application process
  // will be killed by pthread_kill()
  sigemptyset(&(pvSigaction.sa_mask));
  pvSigaction.sa_flags = 0;
  pvSigaction.sa_handler = sigactionHandle;
  sigaction(SIGALRM, &pvSigaction, NULL);
}

mdtSerialPortPosix::~mdtSerialPortPosix()
{
  closePort();
}

/// NOTE: validité config
bool mdtSerialPortPosix::openPort(mdtSerialPortConfig &cfg)
{
  // Close previous opened port
  closePort();
  // Try to open port
  //  O_RDWR : Read/write access
  //  O_NOCTTY: not a terminal
  //  O_NDELAY: ignore DCD signal
  pvSerialPortFd = open(cfg.interface().toStdString().c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK );
  if(pvSerialPortFd < 0){
    qDebug() << "mdtSerialPortPosix::setConfig(): unable to open port: " << cfg.interface();
    return false;
  }
  // Get current config and save it to pvOriginalTermios
  if(tcgetattr(pvSerialPortFd, &pvOriginalTermios) < 0){
    qDebug() << "mdtSerialPortPosix::setConfig(): tcgetattr() failed, " << cfg.interface() << " is not a serial port, or is not available";
    close(pvSerialPortFd);
    return false;
  }
  // Get current system config on "work copy"
  tcgetattr(pvSerialPortFd, &pvTermios);
  // Set baud rate
  if(!setBaudRate(cfg.baudRate())){
    qDebug() << "mdtSerialPortPosix::setConfig(): unsupported baud rate '" << cfg.baudRate() << "' for port " << cfg.interface();
    return false;
  }
  // Set local mode and enable the receiver
  pvTermios.c_cflag |= (CLOCAL | CREAD);
  // Set data bits
  if(!setDataBits(cfg.dataBitsCount())){
    qDebug() << "mdtSerialPortPosix::setConfig(): unsupported data bits count '" << cfg.dataBitsCount() << "' for port " << cfg.interface();
    return false;
  }
  // Set stop bits
  if(!setStopBits(cfg.stopBitsCount())){
    qDebug() << "mdtSerialPortPosix::setConfig(): unsupported stop bits count '" << cfg.stopBitsCount() << "' for port " << cfg.interface();
    return false;
  }
  // Set parity
  setParity(cfg.parity());
  // Set flow control
  setFlowCtlRtsCts(cfg.flowCtlRtsCtsEnabled());
  setFlowCtlXonXoff(cfg.flowCtlXonXoffEnabled(), cfg.xonChar(), cfg.xoffChar());
  // Set raw data mode
  pvTermios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  pvTermios.c_oflag &= ~OPOST;
  // Apply the setup
  if(tcsetattr(pvSerialPortFd, TCSANOW, &pvTermios) < 0){
    qDebug() << "mdtSerialPortPosix::setConfig(): unble to apply configuration for port " << cfg.interface();
    return false;
  }

  return checkConfig(cfg);
}


int mdtSerialPortPosix::waitEventRx(int timeout)
{
  fd_set input;
  struct timeval sTimeout;
  int n;

  // Timeout
  sTimeout.tv_sec = timeout/1000;
  sTimeout.tv_usec = 1000*(timeout%1000);

  // Init the select call
  FD_ZERO(&input);
  FD_SET(pvSerialPortFd, &input);

  n = select(pvSerialPortFd+1, &input, 0, 0, &sTimeout);
  if(n < 0){
    switch(errno){
      case EBADF:
        perror("EBADF");
        break;
      case EINTR:
        perror("EINTR");
        break;
      case EINVAL:
        perror("EINVAL");
        break;
      case ENOMEM:
        perror("ENOMEM");
        break;
      default:
        qDebug() << "unknow error";
    }
  }

  return n;
}

int mdtSerialPortPosix::readData(char *data, int maxLen)
{
  int n;
  
  n = read(pvSerialPortFd, data, maxLen);
  if(n < 0){
    qDebug() << "mdtSerialPortPosix::readData(): read error";
    switch(errno){
      case EINTR:
        perror("EINTR");
        break;
      case EAGAIN:
        perror("EAGAIN");
        break;
      case EIO:
        perror("EIO");
        break;
      case EISDIR:
        perror("EISDIR");
        break;
      case EBADF:
        perror("EBADF");
        break;
      case EINVAL:
        perror("EINVAL");
        break;
      case EFAULT:
        perror("EFAULT");
        break;
      default:
        qDebug() << "unknow error";
    }
  }

  return n;
}

int mdtSerialPortPosix::waitEventTxReady(int timeout)
{
  fd_set output;
  struct timeval sTimeout;
  int n;

  // Timeout
  sTimeout.tv_sec = timeout/1000;
  sTimeout.tv_usec = 1000*(timeout%1000);

  // Init the select call
  FD_ZERO(&output);
  FD_SET(pvSerialPortFd, &output);
  
  n = select(pvSerialPortFd+1, 0, &output, 0, &sTimeout);
  if(n < 0){
    switch(errno){
      case EBADF:
        perror("EBADF");
        break;
      case EINTR:
        perror("EINTR");
        break;
      case EINVAL:
        perror("EINVAL");
        break;
      case ENOMEM:
        perror("ENOMEM");
        break;
      default:
        qDebug() << "unknow error";
    }
  }

  return n;
}

int mdtSerialPortPosix::writeData(const char *data, int len)
{
  int n;
  
  n = write(pvSerialPortFd, data, len);
  if(n < 0){
    qDebug() << "mdtSerialPortPosix::writeData(): write error";
    switch(errno){
      case EINTR:
        perror("EINTR");
        break;
      case EAGAIN:
        perror("EAGAIN");
        break;
      case EIO:
        perror("EIO");
        break;
      case EPIPE:
        perror("EPIPE");
        break;
      case EBADF:
        perror("EBADF");
        break;
      case EINVAL:
        perror("EINVAL");
        break;
      case EFAULT:
        perror("EFAULT");
        break;
      case ENOSPC:
        perror("ENOSPC");
        break;
      default:
        qDebug() << "unknow error";
    }
  }

  return n;
}

int mdtSerialPortPosix::waitEventCtl(int timeout)
{
  int event = 0;

  qDebug() << "Call mdtSerialPortPosix::waitEventCtl() ...";

  //alarm(1);
  // We wait until a line status change happen NOTE: blocking forever if no change happens.
  if(ioctl(pvSerialPortFd, TIOCMIWAIT, (TIOCM_CAR | TIOCM_DSR | TIOCM_CTS | TIOCM_RNG)) < 0){
    perror("mdtSerialPortPosix::waitEventCtl() , ioctl() with command TIOCMIWAIT");
    return -1;
  }
  //alarm(0);
  // We have a event here
  if(ioctl(pvSerialPortFd, TIOCMGET, &event) < 0){
    perror("mdtSerialPortPosix::waitEventCtl() , ioctl() with command TIOCMGET");
    return -1;
  }

  qDebug() << "EVT !!! : " << event;
  
  // See witch signals changed
  if((event & TIOCM_CAR) != pvPreviousCarState){
    qDebug() << "CAR (CD) changed: " << (event & TIOCM_CAR);
    // Store new state
    pvPreviousCarState = (event & TIOCM_CAR);
    // Determine current state and emit signal
    if(event & TIOCM_CAR){
      pvCarIsOn = true;
    }else{
      pvCarIsOn = false;
    }
    emit carChanged(pvCarIsOn);
  }
  if((event & TIOCM_DSR) != pvPreviousDsrState){
    qDebug() << "DSR changed: " << (event & TIOCM_DSR);
    // Store new state
    pvPreviousDsrState = (event & TIOCM_DSR);
    // Determine current state and emit signal
    if(event & TIOCM_DSR){
      pvDsrIsOn = true;
    }else{
      pvDsrIsOn = false;
    }
    emit dsrChanged(pvDsrIsOn);
  }
  if((event & TIOCM_CTS) != pvPreviousCtsState){
    qDebug() << "CTS changed: " << (event & TIOCM_CTS);
    // Store new state
    pvPreviousCtsState = (event & TIOCM_CTS);
    // Determine current state and emit signal
    if(event & TIOCM_CTS){
      pvCtsIsOn = true;
    }else{
      pvCtsIsOn = false;
    }
    emit ctsChanged(pvCtsIsOn);
  }
  if((event & TIOCM_RNG) != pvPreviousRngState){
    qDebug() << "RNG (RI) changed: " << (event & TIOCM_RNG);
    // Store new state
    pvPreviousRngState = (event & TIOCM_RNG);
    // Determine current state and emit signal
    if(event & TIOCM_RNG){
      pvRngIsOn = true;
    }else{
      pvRngIsOn = false;
    }
    emit rngChanged(pvRngIsOn);
  }
  
  return event;
}

void mdtSerialPortPosix::setRts(bool on)
{
  qDebug() << "setRTS called..";
}

void mdtSerialPortPosix::setDtr(bool on)
{
  qDebug() << "set DTR called..";
}

void mdtSerialPortPosix::defineCtlThread(pthread_t ctlThread)
{
  pvCtlThread = ctlThread;
}

void mdtSerialPortPosix::abortWaitEventCtl()
{
  Q_ASSERT(pvCtlThread != 0);
  
  pthread_kill(pvCtlThread, SIGALRM);
}

bool mdtSerialPortPosix::setBaudRate(int rate)
{
  speed_t brMask = 0;

  switch(rate){
    case 50:
      brMask = B50;
      break;
    case 75:
      brMask = B75;
      break;
    case 110:
      brMask = B110;
      break;
    case 134:   // Is 134.5 baud
      brMask = B134;
      break;
    case 150:
      brMask = B150;
      break;
    case 200:
      brMask = B200;
      break;
    case 300:
      brMask = B300;
      break;
    case 600:
      brMask = B600;
      break;
    case 1200:
      brMask = B1200;
      break;
    case 1800:
      brMask = B1800;
      break;
    case 2400:
      brMask = B2400;
      break;
    case 4800:
      brMask = B4800;
      break;
    case 9600:
      brMask = B9600;
      break;
#ifdef B19200
    case 19200:
      brMask = B19200;
      break;
#endif
    case 38400:
      brMask = B38400;
      break;
#ifdef  B57600
    case 57600:
      brMask = B57600;
      break;
#endif
#ifdef B76800
    case 76800:
      brMask = B76800;
      break;
#endif
#ifdef  B115200
    case 115200:
      brMask = B115200;
      break;
#endif
#ifdef  B230400
    case 230400:
      brMask = B230400;
      break;
#endif
#ifdef  B460800
    case 460800:
      brMask = B460800;
      break;
#endif
#ifdef  B500000
    case 500000:
      brMask = B500000;
      break;
#endif
#ifdef  B576000
    case 576000:
      brMask = B576000;
      break;
#endif
#ifdef  B921600
    case 921600:
      brMask = B921600;
      break;
#endif
#ifdef  B1000000
    case 1000000:
      brMask = B1000000;
      break;
#endif
#ifdef  B1152000
    case 1152000:
      brMask = B1152000;
      break;
#endif
#ifdef  B1500000
    case 1500000:
      brMask = B1500000;
      break;
#endif
#ifdef  B2000000
    case 2000000:
      brMask = B2000000;
      break;
#endif
#ifdef  B2500000
    case 2500000:
      brMask = B2500000;
      break;
#endif
#ifdef  B3000000
    case 3000000:
      brMask = B3000000;
      break;
#endif
#ifdef  B3500000
    case 3500000:
      brMask = B3500000;
      break;
#endif
#ifdef  B4000000
    case 4000000:
      brMask = B4000000;
      break;
#endif
    default:
      return false;
  }
  // Apply baud rate to the termios configuration structure
  if(cfsetispeed(&pvTermios, brMask) < 0){
    return false;
  }
  if(cfsetospeed(&pvTermios, brMask) < 0){
    return false;
  }

  return true;
}

int mdtSerialPortPosix::baudRate()
{
  speed_t brMask = 0;
  int rate;

  // Get configured baud rate
  // Note, we allways configure the same rate for input
  // or output, so we choose here to get input configured rate
  brMask = cfgetispeed(&pvTermios);
  // Get numeric value
  switch(brMask){
    case B50:
      rate = 50;
      break;
    case B75:
      rate = 75;
      break;
    case B110:
      rate = 110;
      break;
    case B134:   // Is 134.5 baud
      rate = 134;
      break;
    case B150:
      rate = 150;
      break;
    case B200:
      rate = 200;
      break;
    case B300:
      rate = 300;
      break;
    case B600:
      rate = 600;
      break;
    case B1200:
      rate = 1200;
      break;
    case B1800:
      rate = 1800;
      break;
    case B2400:
      rate = 2400;
      break;
    case B4800:
      rate = 4800;
      break;
    case B9600:
      rate = 9600;
      break;
#ifdef B19200
    case B19200:
      rate = 19200;
      break;
#endif
    case B38400:
      rate = 38400;
      break;
#ifdef  B57600
    case B57600:
      rate = 57600;
      break;
#endif
#ifdef B76800
    case B76800:
      rate = B76800;
      break;
#endif
#ifdef  B115200
    case B115200:
      rate = 115200;
      break;
#endif
#ifdef  B230400
    case B230400:
      rate = 230400;
      break;
#endif
#ifdef  B460800
    case B460800:
      rate = 460800;
      break;
#endif
#ifdef  B500000
    case B500000:
      rate = 500000;
      break;
#endif
#ifdef  B576000
    case B576000:
      rate = 576000;
      break;
#endif
#ifdef  B921600
    case B921600:
      rate = 921600;
      break;
#endif
#ifdef  B1000000
    case B1000000:
      rate = 1000000;
      break;
#endif
#ifdef  B1152000
    case B1152000:
      rate = 1152000;
      break;
#endif
#ifdef  B1500000
    case B1500000:
      rate = 1500000;
      break;
#endif
#ifdef  B2000000
    case B2000000:
      rate = 2000000;
      break;
#endif
#ifdef  B2500000
    case B2500000:
      rate = 2500000;
      break;
#endif
#ifdef  B3000000
    case B3000000:
      rate = 3000000;
      break;
#endif
#ifdef  B3500000
    case B3500000:
      rate = 3500000;
      break;
#endif
#ifdef  B4000000
    case B4000000:
      rate = 4000000;
      break;
#endif
    default:
      rate = 0;
  }

  return rate;
}

bool mdtSerialPortPosix::setDataBits(int n)
{
  tcflag_t nbMask = 0;

  switch(n){
    case 5:
      nbMask = CS5;
      break;
    case 6:
      nbMask = CS5;
      break;
    case 7:
      nbMask = CS7;
      break;
    case 8:
      nbMask = CS8;
      break;
    default:
      return false;
  }
  // Apply to configuration
  pvTermios.c_cflag &= ~CSIZE;
  pvTermios.c_cflag |= nbMask;
  
  return true;
}

int mdtSerialPortPosix::dataBits()
{
  int nb = 0;

  // Get configured bits count
  if((pvTermios.c_cflag & CSIZE) == CS8){
    nb = 8;
  }else if((pvTermios.c_cflag & CSIZE) == CS7){
    nb = 7;
  }else if((pvTermios.c_cflag & CSIZE) == CS6){
    nb = 6;
  }else if((pvTermios.c_cflag & CSIZE) == CS5){
    nb = 5;
  }else{
    nb = 0;
  }
  
  return nb;
}

bool mdtSerialPortPosix::setStopBits(int n)
{
  if(n == 1){
    pvTermios.c_cflag &= ~CSTOPB;
  }else if(n == 2){
    pvTermios.c_cflag |= CSTOPB;
  }else{
    return false;
  }

  return true;
}

int mdtSerialPortPosix::stopBits()
{
  if(pvTermios.c_cflag & CSTOPB){
    return 2;
  }
  return 1;
}

void mdtSerialPortPosix::setParity(mdtSerialPortConfig::parity_t p)
{
  if(p == mdtSerialPortConfig::NoParity){
    pvTermios.c_cflag &= ~PARENB;
    pvTermios.c_cflag &= ~CSTOPB;
    pvTermios.c_iflag &= ~(INPCK | ISTRIP);
  }else if(p == mdtSerialPortConfig::ParityOdd){
    pvTermios.c_cflag |= PARENB;
    pvTermios.c_cflag |= PARODD;
    // Check and strip parity
    pvTermios.c_iflag |= (INPCK | ISTRIP);
  }else{
    pvTermios.c_cflag |= PARENB;
    pvTermios.c_cflag &= ~PARODD;
    // Check and strip parity
    pvTermios.c_iflag |= (INPCK | ISTRIP);
  }
}

mdtSerialPortConfig::parity_t mdtSerialPortPosix::parity()
{
  if(!(pvTermios.c_cflag & PARENB)){
    return mdtSerialPortConfig::NoParity;
  }
  // Here, parity is enabled
  if(pvTermios.c_cflag & PARENB){
    return mdtSerialPortConfig::ParityOdd;
  }
  return mdtSerialPortConfig::ParityEven;
}

void mdtSerialPortPosix::setFlowCtlRtsCts(bool on)
{
  if(on){
    pvTermios.c_cflag |= CRTSCTS;
  }else{
    pvTermios.c_cflag &= ~CRTSCTS;
  }
}

bool mdtSerialPortPosix::flowCtlRtsCtsOn()
{
  return (pvTermios.c_cflag & CRTSCTS);
}

void mdtSerialPortPosix::setFlowCtlXonXoff(bool on, char xonChar, char xoffChar)
{
  if(on){
    pvTermios.c_iflag |= (IXON | IXOFF | IXANY);
    pvTermios.c_cc[VSTART] = xonChar;
    pvTermios.c_cc[VSTOP] = xoffChar;
  }else{
    pvTermios.c_iflag &= ~(IXON | IXOFF | IXANY);
  }
}

bool mdtSerialPortPosix::flowCtlXonXoffOn()
{
  return (pvTermios.c_iflag & (IXON | IXOFF | IXANY));
}

void mdtSerialPortPosix::closePort()
{
  if(pvSerialPortFd >= 0){
    tcsetattr(pvSerialPortFd, TCSANOW, &pvOriginalTermios); 
    close(pvSerialPortFd);
  }
}

bool mdtSerialPortPosix::checkConfig(mdtSerialPortConfig cfg)
{
  Q_ASSERT(pvSerialPortFd >= 0);

  // Get current system config
  tcgetattr(pvSerialPortFd, &pvTermios);

  // Check static coded flags
  if(!(pvTermios.c_cflag & (CLOCAL | CREAD))){
    qDebug() << "mdtSerialPortPosix::checkConfig(): CLOCAL and CREAD are not set";
    return false;
  }
  if(pvTermios.c_lflag & (ICANON | ECHO | ECHOE | ISIG)){
    qDebug() << "mdtSerialPortPosix::checkConfig(): raw data mode not set";
    return false;
  }
  if(pvTermios.c_oflag & OPOST){
    qDebug() << "mdtSerialPortPosix::checkConfig(): raw data mode not set (OPOST is on)";
    return false;
  }
  // Check parameters given by cfg
  if(baudRate() != cfg.baudRate()){
    qDebug() << "mdtSerialPortPosix::checkConfig(): baud rate is not set";
    return false;
  }
  if(dataBits() != cfg.dataBitsCount()){
    qDebug() << "mdtSerialPortPosix::checkConfig(): data bits count is not set";
    return false;
  }
  if(stopBits() != cfg.stopBitsCount()){
    qDebug() << "mdtSerialPortPosix::checkConfig(): stop bits count is not set";
    return false;
  }
  if(parity() != cfg.parity()){
    qDebug() << "mdtSerialPortPosix::checkConfig(): parity is not set";
    return false;
  }
  if(flowCtlRtsCtsOn() != cfg.flowCtlRtsCtsEnabled()){
    qDebug() << "mdtSerialPortPosix::checkConfig(): Flow ctl RTS/CTS is not set";
    return false;
  }
  if(flowCtlXonXoffOn() != cfg.flowCtlXonXoffEnabled()){
    qDebug() << "mdtSerialPortPosix::checkConfig(): Flow ctl Xon/Xoff is not set";
    return false;
  }
  if(cfg.flowCtlXonXoffEnabled()){
    if(pvTermios.c_cc[VSTART] != cfg.xonChar()){
      qDebug() << "mdtSerialPortPosix::checkConfig(): Xon char is not set";
      return false;
    }
    if(pvTermios.c_cc[VSTOP] != cfg.xoffChar()){
      qDebug() << "mdtSerialPortPosix::checkConfig(): Xoff char is not set";
      return false;
    }
  }

  return true;
}

void mdtSerialPortPosix::sigactionHandle(int/*signum*/)
{
}
