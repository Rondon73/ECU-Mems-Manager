#include <QDir>
#include <QDateTime>
#include "logger.h"
#include "shared/mems_core/mems_core.h"

/**
 * Constructor. Sets the interface class pointer as
 * well as log directory and log file extension.
 */
Logger::Logger(MEMSInterface* memsiface):
m_logExtension(".txt"), m_logDir("logs")
{
  m_mems = memsiface;
}

/**
 * Gets EcuId
 */
/* void Logger::EcuIdTransmitted(uint8_t* id)
{
	char idString[20];
	sprintf(idString, " %02X %02X %02X %02X", id[0], id[1], id[2], id[3]);
	ecuid = idString;
} */
bool Logger::EcuIdTransmitted(QString id)
{
	ecuid = id;
	return true;
}

/**
 * Attempts to open a log file with the name specified.
 * @return True on success, false otherwise
 */
bool Logger::openLog(QString fileName)
{
  bool success = false;

  m_lastAttemptedLog = m_logDir + QDir::separator() + fileName + m_logExtension;

  // if the 'logs' directory exists, or if we're able to create it...
  if (!m_logFile.isOpen() && (QDir(m_logDir).exists() || QDir().mkdir(m_logDir)))
  {
    // set the name of the log file and open it for writing
    bool alreadyExists = QFileInfo(m_lastAttemptedLog).exists();

    m_logFile.setFileName(m_lastAttemptedLog);
    if (m_logFile.open(QFile::WriteOnly | QFile::Append))
    {
      m_logFileStream.setDevice(&m_logFile);

      if (!alreadyExists)
      {

       m_logFileStream << ecuid << endl;
       m_logFileStream << QString::fromStdString(mems::core::csvHeaderLine()) << endl;
    }
      success = true;
    }
  }

  return success;
}

/**
 * Close the log file.
 */
void Logger::closeLog()
{
  m_logFile.close();
}

/**
 * Converts degrees F to degrees C if necessary
 */
/* uint8_t Logger::convertTemp(uint8_t degrees)
{
  if (m_tempUnits == Celsius)
  {
    return ((degrees - 32) / 1.8);
  }
  else
  {
    return degrees;
  }
} */

/**
 * Converts lambda signal to mV
 */
/* uint8_t Logger::convertLambda(uint8_t mV)
{
  if (m_LScale == _4mV_steps)
  {
    return (mV * 4);
  }
  else
  {
    return (mV * 5);
  }
} */

/**
 * Commands the logger to query the interface for the currently
 * buffered data, and write it to the file.
 */
void Logger::logData()
{
  mems_data* data = m_mems->getData();

  if (m_logFile.isOpen() && (m_logFileStream.status() == QTextStream::Ok))
      {
        const mems::core::Snapshot snapshot = mems::core::fromRoscoData(*data);
        const std::string timestamp = QDateTime::currentDateTime().toString("hh:mm:ss").toStdString();
        m_logFileStream << QString::fromStdString(mems::core::csvDataLine(snapshot, timestamp)) << endl;
    }
}
/**
 * Returns the full path to the last log that we attempted to open.
 * @return Full path to last log file
 */
   QString Logger::getLogPath()
 {
   return m_lastAttemptedLog;
 }
