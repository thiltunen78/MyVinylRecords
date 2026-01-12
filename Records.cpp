#include <string>
#include "Records.h"
#include <SD.h>

bool sortFunctionCount (Records::Data &a, Records::Data &b) 
{
  return (a.uCount<b.uCount);
}

bool sortFunctionName (Records::Data &a, Records::Data &b) 
{
  if (a.strArtist.compare(b.strArtist) == 0)
    return (a.strAlbum<b.strAlbum);
  return (a.strArtist<b.strArtist);
}

bool sortFunctionDate (Records::Data &a, Records::Data &b)
{
  return (a.strLastPlayedDate<b.strLastPlayedDate);
}

constexpr uint16_t SDCSPin{4};

Records::Records(std::string strTitle, int32_t iColor1, int32_t iColor2, std::string strFile, uint16_t uReserve) :
  m_strTitle{strTitle},
  m_iColor1{iColor1},
  m_iColor2{iColor2},
  m_strFile{strFile},
  m_sortMode{SortMode::Count},
  m_uTotalCount{0}
{
  if (!SD.begin(SDCSPin)) {    
    return;
  }

  m_vectData.reserve(uReserve);
  readFile();
  std::sort (m_vectData.begin(), m_vectData.end(), sortFunctionCount);
}

void Records::readFile() 
{
  m_vectData.clear();

  //if file is missing then create example file and exit 
  if (!SD.exists(m_strFile.c_str())) {
    m_vectData.push_back({"Artist", "Album", 0, "n/a", "-"});
    writeFile();
    return;
  }

  m_uTotalCount = 0;
  std::string strTemp{""};
  std::string strArtist{""};
  std::string strAlbum{""};
  int32_t iCount{-1};
  std::string strDate{""};
  fs::File file;  

  file = SD.open(m_strFile.c_str());
  if (file) {
    // read from the file until there's nothing else in it:
    while (file.available()) { 
      char c = file.read();      
      if((c != '\n') && (c != '\r') && (c != ',')) {
        strTemp.push_back(c);      
      }
      else {
        if (strArtist.empty()) {
          strArtist = strTemp;
          strTemp = "";
        }
        else if (strAlbum.empty()) {
          strAlbum = strTemp;
          strTemp = "";
        }
        else if (iCount < 0) {
          iCount = std::stoi(strTemp);
          strTemp = "";
        }
        else if (strDate.empty()) {
          strDate = strTemp;
          strTemp = "";
        }
        else {
          m_vectData.push_back({strArtist, strAlbum, static_cast<uint16_t>(iCount), strDate, strTemp});
          m_uTotalCount += static_cast<uint16_t>(iCount);
          strArtist = "";
          strAlbum = "";
          iCount = -1;
          strDate = "";
          strTemp = "";
        }
      }
    }
    file.close();
  } 
}

void Records::writeFile() 
{
  std::string strRow("");
  fs::File file;

  if (SD.exists(m_strFile.c_str())) {
    SD.remove(m_strFile.c_str());
  }

  file = SD.open(m_strFile.c_str(), FILE_WRITE);
  if (file) {
    for (const auto &record : m_vectData) {
      strRow = record.strArtist;
      strRow.append(",");
      strRow.append(record.strAlbum);
      strRow.append(",");
      strRow.append(std::to_string(record.uCount));
      strRow.append(",");
      strRow.append(record.strLastPlayedDate);
      strRow.append(",");
      strRow.append(record.strNotes);
      file.println(strRow.c_str());
    }   
    
    file.close();
  }
  // This is to enable back faster screen updating after SD card operations
  SPI.beginTransaction({40000000, MSBFIRST, SPI_MODE0});
}

std::string Records::getArtist(int32_t index)
{
  return m_vectData[index].strArtist;
}

std::string Records::getAlbum(int32_t index)
{
  return m_vectData[index].strAlbum;
}

std::string Records::getCount(int32_t index)
{
  return std::to_string(m_vectData[index].uCount);
}

std::string Records::getLastPlayedDate(int32_t index)
{
  return m_vectData[index].strLastPlayedDate;
}

std::string Records::getNotes(int32_t index)
{
  return m_vectData[index].strNotes;
}

uint16_t Records::getTotalCount()
{
  return m_uTotalCount;
}

std::size_t Records::getSize()
{
  return m_vectData.size();
}

std::string Records::getTitle()
{
  return m_strTitle;
}

int32_t Records::getColor1()
{
  return m_iColor1;
}

int32_t Records::getColor2()
{
  return m_iColor2;
}

void Records::sort()
{
  if (m_sortMode == SortMode::Count) {
    m_sortMode = SortMode::Name;
    std::sort(m_vectData.begin(), m_vectData.end(), sortFunctionName);
  }
  else if (m_sortMode == SortMode::Name) {
    m_sortMode = SortMode::Date;
    std::sort(m_vectData.begin(), m_vectData.end(), sortFunctionDate);
  }
  else if (m_sortMode == SortMode::Date) {
    m_sortMode = SortMode::Count;
    std::sort(m_vectData.begin(), m_vectData.end(), sortFunctionCount);
  }
}

SortMode Records::getSortMode()
{
  return m_sortMode;
}

std::string Records::getSortModeString()
{
  std::string strSortMode;
  if (m_sortMode == SortMode::Name)
    strSortMode = "name";
  else if (m_sortMode == SortMode::Count)
    strSortMode = "count";
  else
    strSortMode = "date";
  
  return strSortMode;
}

void Records::increaseCount(int32_t index, int32_t amount, std::string &date)
{
  m_vectData[index].strLastPlayedDate = date;
  m_vectData[index].uCount += amount;
  m_uTotalCount += amount;

  //Save file as sorted by name
  std::sort(m_vectData.begin(), m_vectData.end(), sortFunctionName);
  writeFile();

  if (m_sortMode == SortMode::Count) {
    std::sort(m_vectData.begin(), m_vectData.end(), sortFunctionCount);
  }
  else if (m_sortMode == SortMode::Date) {
    std::sort(m_vectData.begin(), m_vectData.end(), sortFunctionDate);
  }
}

